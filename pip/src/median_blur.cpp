
#include <algorithm>
#include <cassert>
// #include <intrin.h>
#include <iterator>
#include <stdint.h>
#include <type_traits>
#include <vector>
#include <x86intrin.h>

#include <median_blur.h>

#include <oneapi/tbb/partitioner.h>
#include <tbb/blocked_range2d.h>
#include <tbb/partitioner.h>
#include <tbb/tbb.h>

namespace pip {

#pragma region minmax

template <typename packed_type, typename simd_type,
          typename = std::enable_if<sizeof(simd_type) == sizeof(packed_type)>>
void min_max(simd_type &a, simd_type &b) {
  simd_type t = a;
  a = std::min(a, b);
  b = std::max(t, b);
}

#if defined(__AVX512BW__)

#define simd_int __m512i

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

#define simd_int __m256i

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

#elif defined(__SSE2__)

#define simd_int __m128i

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

#else

#define __no_intrinic__
#define simd_int uint8_t

#endif

#pragma endregion

/*
reference: pairwise sorting network
*/
template <typename packed_type, typename Iterator,
          std::enable_if_t<std::is_same_v<typename std::iterator_traits<
                                              Iterator>::iterator_category,
                                          std::random_access_iterator_tag>,
                           int> = 0>
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

template <typename packed_type, typename Iterator,
          std::enable_if_t<std::is_same_v<typename std::iterator_traits<
                                              Iterator>::iterator_category,
                                          std::random_access_iterator_tag>,
                           int> = 0>
void median_selection(Iterator _First, Iterator _End) {
  auto n = std::distance(_First, _End);
  auto first_n = (n + 1) / 2 + 1;
  int64_t cnt = n - first_n;
  auto _Mid = _First + first_n;
  while (cnt >= 0) {
    pop_min_max_to_end<packed_type>(_First, _Mid);
    ++_First;
    --_End;
    --cnt;
    std::swap(*(_Mid - 1), *_End);
  }
}

// template <typename T, typename image_type>
// void median_selection(Vector_Aligned64<T> &arr) {
//   auto _First = arr.begin();
//   auto _End = arr.end();
//   auto n = std::distance(_First, _End);
//   auto first_n = (n + 1) / 2 + 1;
//   int64_t cnt = n - first_n;
//   auto _Mid = _First + first_n;
//   while (cnt >= 0) {
//     pop_min_max_to_end<image_type>(_First, _Mid);
//     ++_First;
//     --_End;
//     --cnt;
//     std::swap(*(_Mid - 1), *_End);
//   }
// }

template <typename T, typename image_type>
void median_selection_2row(Vector_Aligned64<T> &arr, int mask_width) {
  auto _First = arr.begin();
  auto _End = arr.end();
  auto n = std::distance(_First, _End);

  // common part
  auto _C_n = n - 2 * mask_width;
  int _C_cnt = (_C_n - (mask_width + 1)) >> 1;
  auto _C_First = _First + mask_width;
  auto _C_Mid = _C_First + (_C_n + 1) / 2 + 1;
  auto _C_End = _End - mask_width;
  while (_C_cnt > 0) {
    pop_min_max_to_end<image_type>(_C_First, _C_End);
    ++_C_First;
    --_C_End;
    std::swap(*(_C_Mid - 1), *_C_End);
    --_C_cnt;
  }

  assert(std::distance(_C_First, _C_End) == mask_width + 1);
  // first row
  std::copy(_C_First, _C_End, arr.begin() + mask_width);
  median_selection<image_type>(arr.begin(), arr.begin() + 2 * mask_width + 1);
  // second row
  std::copy(_C_First, _C_End, arr.end() - 2 * mask_width - 1);
  median_selection<image_type>(arr.end() - 2 * mask_width - 1, arr.end());

  return;
}

template <typename T, typename F>
void blur_2D(const ImageU8 &img_i, ImageU8 &img_o, int r_begin, int r_end,
             int c_begin, int c_end, int ch, Vector_Aligned64<T> &arr, F func) {
  int i = 0;
  for (int r = r_begin; r <= r_end; ++r) {
    _mm_prefetch((const char *)&img_i(c_begin, r + 1, ch), _MM_HINT_T0);
    for (int c = c_begin; c <= c_end; ++c) {
      memcpy(&arr[i], &img_i(c, r, ch), sizeof(T));
      ++i;
    }
  }

  // func(arr);
  // func(arr.begin(), arr.end());
  func(arr, c_end - c_begin + 1);

  return;
}

void median_blur(const Image &_in_img, Image &_out_img, int mask_width,
                 int mask_height) {
  using buffer_type = simd_int;
  using vector_aligned64 = Vector_Aligned64<buffer_type>;

  const ImageU8 &in_img = std::get<ImageU8>(_in_img);
  ImageU8 &out_img = std::get<ImageU8>(_out_img);

  constexpr size_t blocksize = 64;
  constexpr int ratio = sizeof(buffer_type) / sizeof(ImageU8::value_type);

  auto [width, height, channel] = in_img.shape();
  out_img.reshape(width, height, channel);

  int extend_row = mask_width >> 1;
  int extend_col = mask_height >> 1;
  int a = 1;
  int n = (mask_height + a) * mask_width;

  for (int ch = 0; ch < channel; ++ch) {
    tbb::parallel_for(
        tbb::blocked_range2d<size_t, size_t>{0, height >> a, blocksize >> 1, 0,
                                             width, blocksize},
        [&](const tbb::blocked_range2d<size_t, size_t> &range) {
          int r_begin = range.rows().begin();
          int r_end = range.rows().end();
          int c_begin = range.cols().begin();
          int c_end = range.cols().end();

          vector_aligned64 arr((mask_height + a) * mask_width);

          for (int r = r_begin; r < r_end; ++r) {
            for (int c = c_begin; c < c_end; c += ratio) {

#if defined(__no_intrinic__)
              auto nth_element = [](vector_aligned64 &arr) {
                int med = arr.size() >> 1;
                std::nth_element(arr.begin(), arr.begin() + med, arr.end());
              };

              blur_2D(in_img, out_img, r - extend_row, r + extend_row,
                      c - extend_col, c + extend_col, ch, arr, nth_element);
              out_img(c, r, ch) = arr[n >> 1];
#else
              int rr = (a + 1) * r;
              // blur_2D(in_img, out_img, rr - extend_row, rr + extend_row + a,
              //         c - extend_col, c + extend_col, ch, arr,
              //         median_selection<buffer_type, Image::value_type>);
              // memcpy(&out_img(c, rr, ch), &arr[n >> 1], sizeof(buffer_type));
              blur_2D(in_img, out_img, rr - extend_row, rr + extend_row + a,
                      c - extend_col, c + extend_col, ch, arr,
                      median_selection_2row<buffer_type, ImageU8::value_type>);
              memcpy(&out_img(c, rr, ch), &arr[mask_width + 1],
                     sizeof(buffer_type));
              memcpy(&out_img(c, rr + 1, ch), &arr[n - mask_width - 1],
                     sizeof(buffer_type));
#endif

              // blur_2D(in_img, out_img, rr, c, ch, mask_width, mask_height,
              // arr,
              //         median_selection_2row<buffer_type, Image::value_type>);

              // memcpy(&out_img(c, rr, ch), &arr[n >> 1], sizeof(buffer_type));
              // memcpy(&out_img(c, rr + 1, ch), &arr[(n >> 1) + mask_width],
              //        sizeof(buffer_type));
              // _mm256_storeu_si256((buffer_type *)&out_img(c, r, ch),
              //                     arr[n >> 1]);
            }
          }
        },
        tbb::simple_partitioner{});
  }
}

} // namespace pip