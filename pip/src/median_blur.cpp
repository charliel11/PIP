#include "Image.h"
#include <algorithm>
#include <cassert>
#include <corecrt_memcpy_s.h>
#include <emmintrin.h>
#include <exception>
#include <iostream>
#include <iterator>
#include <median_blur.h>
#include <smmintrin.h>
#include <stdint.h>
#include <tbb/blocked_range2d.h>
#include <tbb/partitioner.h>
#include <tbb/tbb.h>
#include <type_traits>
#include <vector>
#include <winnt.h>

namespace pip {

#pragma region minmax

template <typename packed_type, typename simd_type,
          typename = std::enable_if<sizeof(simd_type) == sizeof(packed_type)>>
void min_max(simd_type &a, simd_type &b) {
  simd_type t = a;
  a = std::min(a, b);
  b = std::max(t, b);
}

// template <typename packed_type> void min_max(packed_type &a, packed_type &b)
// {
//   packed_type t = a;
//   a = std::min(a, b);
//   b = std::max(t, b);
// }

// template <> inline void min_max<64, __m512i>(__m512i &a, __m512i &b) {
//   __m512i t = a;
//   a = _mm512_min_epu8(a, b);
//   b = _mm512_max_epu8(b, t);
// }
// template <> inline void min_max<32, __m512i>(__m512i &a, __m512i &b) {
//   __m512i t = a;
//   a = _mm512_min_epu16(a, b);
//   b = _mm512_max_epu16(b, t);
//   //__m512i t = _mm512_subs_epu16(a, b);
//   // a = _mm512_subs_epu16(a, t);
//   // b = _mm512_adds_epu16(b, t);
// }
// template <> inline void min_max<16, __m512>(__m512 &a, __m512 &b) {
//   __m512 t = a;
//   a = _mm512_min_ps(a, b);
//   b = _mm512_max_ps(b, t);
// }

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

#pragma endregion

/*
reference: pairwise sorting network
*/
template <typename packed_type, typename Iterator,
          typename = std::is_same<
              typename std::iterator_traits<Iterator>::iterator_category,
              std::random_access_iterator_tag>>
void pop_min_max_to_end(Iterator _First, Iterator _End) {
  _Adl_verify_range(_First, _End);
  auto n = std::distance(_First, _End);

  using iter_value_type = typename std::iterator_traits<Iterator>::value_type;

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

void blur_2D_simd(const Image &img_i, Image &img_o, int row, int col, int ch,
                  int mask_w, int mask_h) {
  std::vector<Image::value_type> arr;
  int n = mask_h * mask_w;
  arr.reserve(n);

  for (int r = row - (mask_h >> 1); r <= row + (mask_h >> 1); ++r) {
    for (int c = col - (mask_w >> 1); c <= col + (mask_w >> 1); ++c) {
      arr.push_back(img_i(c, r, ch));
    }
  }

  std::nth_element(arr.begin(), arr.begin() + n / 2, arr.end());
  img_o(col, row, ch) = arr[n >> 1];
  return;
}

template <typename T, typename F>
void blur_2D(const Image &img_i, Image &img_o, int row, int col, int ch,
             int mask_w, int mask_h, Vector_Aligned64<T> &arr, F func) {

  // auto [width, height, channel] = img_i.shape();
  // Vector_Aligned64<T> local(mask_w * mask_h);
  int i = 0;
  for (int r = row - (mask_h >> 1); r <= row + (mask_h >> 1); ++r) {
    for (int c = col - (mask_w >> 1); c <= col + (mask_w >> 1); ++c) {
      auto ptr = (T *)&img_i(c, r, ch);
      //_mm_stream_si32((int *)&arr[i], *(int *)ptr);
      memcpy((void *)&arr[i], (void *)&img_i(c, r, ch), sizeof(T));
      ++i;
      // _mm256_lddqu_si256((T *)ptr);
      // arr.insert(arr.end(), ptr, ptr + 1);
      // copy(ptr, ptr + 1, std::back_inserter(arr));
      // arr[i] = _mm256_lddqu_si256(ptr);
      // arr.push_back(_mm256_lddqu_si256(ptr));
      // auto ss = sizeof(T);
      // arr.push_back(_mm256_loadu_si256((T *)ptr));
      // arr.push_back(*(T *)ptr);
      // arr.push_back(img_i(c, r, ch));
    }
  }
  // _mm256_stream_si256(arr.data(), *local.data());

  func(arr);

  return;
}

void median_blur(const Image &in_img, Image &out_img, int mask_width,
                 int mask_height) {
  auto [width, height, channel] = in_img.shape();
  out_img.reshape(width, height, channel);
  constexpr size_t blocksize = 64;
  int n = mask_width * mask_height;

  // using buffer_type = Image::value_type;
  using buffer_type = __m256i;

  constexpr int ratio = sizeof(buffer_type) / sizeof(Image::value_type);

  for (int ch = 0; ch < channel; ++ch) {
    tbb::parallel_for(
        tbb::blocked_range2d<size_t, size_t>{0, height, blocksize, 0, width,
                                             blocksize},
        [&](const tbb::blocked_range2d<size_t, size_t> &range) {
          // std::cout << "row:" << ((r.rows().begin() + r.rows().end()) >> 1)
          //           << "col:" << ((r.cols().begin() + r.cols().end()) >> 1)
          //           << std::endl;

          // thread local

          thread_local Vector_Aligned64<buffer_type> arr(mask_height *
                                                         mask_width);
          // arr.resize(mask_height * mask_width);
          // arr.reserve(mask_height * mask_width);

          int r_begin = range.rows().begin();
          int r_end = range.rows().end();
          int c_begin = range.cols().begin();
          int c_end = range.cols().end();

          // auto nth_element = [](Vector_Aligned64<buffer_type> &arr) {
          //   int med = arr.size() >> 1;
          //   std::nth_element(arr.begin(), arr.begin() + med, arr.end());
          // };

          for (int r = r_begin; r < r_end; ++r) {
            _mm_prefetch((const char *)&in_img(c_begin + 64, r, ch),
                         _MM_HINT_T0);
            for (int c = c_begin; c < c_end; c += ratio) {
              _mm_prefetch((const char *)&in_img(c, r + 8, ch),
                           _MM_HINT_T0);
              // blur_2D(in_img, out_img, r, c, ch, mask_width, mask_height,
              // arr,
              //         nth_element);

              blur_2D(in_img, out_img, r, c, ch, mask_width, mask_height, arr,
                      median_selection<buffer_type, Image::value_type>);
              memcpy((void *)&out_img(c, r, ch), (void *)&arr[n >> 1],
                     sizeof(buffer_type));
              // _mm256_storeu_si256((buffer_type *)&out_img(c, r, ch),
              //                     arr[n >> 1]);
              //*(buffer_type *)&out_img(c, r, ch) = arr[n >> 1];
            }
          }
        },
        tbb::simple_partitioner{});
  }
}

} // namespace pip