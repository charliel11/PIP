#include <Image.h>
#include <algorithm>
#include <cassert>
#include <exception>
#include <iostream>
#include <iterator>
#include <median_blur.h>
#include <stdint.h>
#include <tbb/blocked_range2d.h>
#include <tbb/partitioner.h>
#include <tbb/tbb.h>
#include <type_traits>
#include <vector>
#include <x86intrin.h>

namespace pip {

#pragma region minmax

template <typename packed_type, typename simd_type,
          typename = std::enable_if<sizeof(simd_type) == sizeof(packed_type)>>
void min_max(simd_type &a, simd_type &b) {
  simd_type t = a;
  a = std::min(a, b);
  b = std::max(t, b);
}

#if defined(__AVX512__)

#define SIMD_INT __m512i

template <> inline void min_max<uint8_t, __m512i>(__m512i &a, __m512i &b) {
  __m512i t = a;
  a = _mm512_min_epu8(a, b);
  b = _mm512_max_epu8(b, t);
}
template <> inline void min_max<uint16_t, __m512i>(__m512i &a, __m512i &b) {
  __m512i t = a;
  a = _mm512_min_epu16(a, b);
  b = _mm512_max_epu16(b, t);
  //__m512i t = _mm512_subs_epu16(a, b);
  // a = _mm512_subs_epu16(a, t);
  // b = _mm512_adds_epu16(b, t);
}
template <> inline void min_max<float_t, __m512>(__m512 &a, __m512 &b) {
  __m512 t = a;
  a = _mm512_min_ps(a, b);
  b = _mm512_max_ps(b, t);
}

#elif defined(__AVX2__)

#define SIMD_INT __m256i

template <> void min_max<uint8_t, __m256i>(__m256i &a, __m256i &b) {
  __m256i t = a;
  a = _mm256_min_epu8(a, b);
  b = _mm256_max_epu8(b, t);
}

template <> void min_max<uint16_t, __m256i>(__m256i &a, __m256i &b) {
  __m256i t = _mm256_subs_epu16(a, b);
  a = _mm256_subs_epu16(a, t);
  b = _mm256_adds_epu16(b, t);
}

template <> void min_max<float_t, __m256>(__m256 &a, __m256 &b) {
  __m256 t = a;
  a = _mm256_min_ps(a, b);
  b = _mm256_max_ps(b, t);
}

#elif defined(__SSE__)

#define SIMD_INT __m128i

template <> void min_max<uint8_t, __m128i>(__m128i &a, __m128i &b) {
  __m128i t = a;
  a = _mm_min_epu8(a, b);
  b = _mm_max_epu8(b, t);
}

template <> void min_max<uint16_t, __m128i>(__m128i &a, __m128i &b) {
  __m128i t = _mm_subs_epu16(a, b);
  a = _mm_subs_epu16(a, t);
  b = _mm_adds_epu16(b, t);
}

template <> void min_max<float_t, __m128>(__m128 &a, __m128 &b) {
  __m128 t = a;
  a = _mm_min_ps(a, b);
  b = _mm_max_ps(b, t);
}

#endif

#pragma endregion

/*
reference: pairwise sorting network
*/
template <typename packed_type, typename Iterator,
          typename = std::is_same<
              typename std::iterator_traits<Iterator>::iterator_category,
              std::random_access_iterator_tag>>
void pop_min_max_to_end(Iterator _First, Iterator _End) {
  // _Adl_verify_range(_First, _End);
  auto n = std::distance(_First, _End);

  uint32_t left = 0;
  uint32_t right = n - 1;
  for (uint32_t i = left; i < right; i += 2)
    min_max<packed_type>(_First[i], _First[i + 1]);

  for (uint32_t step = 2; step < (right - left + 1); step *= 2) {
    for (uint32_t i = left; (i + step) <= right; i += 2 * step) {
      min_max<packed_type>(_First[i], _First[i + step]);
      min_max<packed_type>(_First[i + step - 1],
                           _First[std::min(right, i + 2 * step - 1)]);
    }
  }
}

template <typename T, typename image_type>
void median_selection(Vector_Aligned64<T> &arr) {
  auto _First = arr.begin();
  auto _End = arr.end();
  auto n = std::distance(_First, _End);
  auto first_n = (n + 1) / 2 + 1;
  int64_t cnt = n - first_n;
  auto _Mid = _First + first_n;
  while (cnt >= 0) {
    pop_min_max_to_end<image_type>(_First, _Mid);
    ++_First;
    --_End;
    --cnt;
    std::swap(*(_Mid - 1), *_End);
  }
}

template <typename T, typename F>
void blur_2D(const Image &img_i, Image &img_o, int row, int col, int ch,
             int mask_w, int mask_h, Vector_Aligned64<T> &arr, F func) {

  int i = 0;
  for (int r = row - (mask_h >> 1); r <= row + (mask_h >> 1); ++r) {
    _mm_prefetch((const char *)&img_i(col - (mask_w >> 1), r + 1, ch),
                 _MM_HINT_T0);
    for (int c = col - (mask_w >> 1); c <= col + (mask_w >> 1); ++c) {
      // auto ptr = (T *)&img_i(c, r, ch);
      memcpy((void *)&arr[i], (void *)&img_i(c, r, ch), sizeof(T));
      // _mm256_store_si256(&arr[i], _mm256_loadu_si256((T *)&img_i(c, r,
      // ch)));
      ++i;
    }
  }

  func(arr);

  return;
}

void median_blur(const Image &in_img, Image &out_img, int mask_width,
                 int mask_height) {
  auto [width, height, channel] = in_img.shape();
  out_img.reshape(width, height, channel);
  constexpr size_t blocksize = 64;
  int n = mask_width * mask_height;

  using buffer_type = SIMD_INT;

  constexpr int ratio = sizeof(buffer_type) / sizeof(Image::value_type);

  for (int ch = 0; ch < channel; ++ch) {
    tbb::parallel_for(
        tbb::blocked_range2d<size_t, size_t>{0, height, blocksize, 0, width,
                                             blocksize},
        [&](const tbb::blocked_range2d<size_t, size_t> &range) {
          int r_begin = range.rows().begin();
          int r_end = range.rows().end();
          int c_begin = range.cols().begin();
          int c_end = range.cols().end();

          thread_local Vector_Aligned64<buffer_type> arr(mask_height *
                                                         mask_width);

          for (int r = r_begin; r < r_end; ++r) {
            for (int c = c_begin; c < c_end; c += ratio) {

              // auto nth_element = [](Vector_Aligned64<buffer_type> &arr) {
              //   int med = arr.size() >> 1;
              //   std::nth_element(arr.begin(), arr.begin() + med,
              //   arr.end());
              // };

              // blur_2D(in_img, out_img, r, c, ch, mask_width, mask_height,
              // arr,
              //         nth_element);

              blur_2D(in_img, out_img, r, c, ch, mask_width, mask_height, arr,
                      median_selection<buffer_type, Image::value_type>);

              memcpy((void *)&out_img(c, r, ch), (void *)&arr[n >> 1],
                     sizeof(buffer_type));
              // _mm256_storeu_si256((buffer_type *)&out_img(c, r, ch),
              //                     arr[n >> 1]);
            }
          }
        },
        tbb::simple_partitioner{});
  }
}

} // namespace pip