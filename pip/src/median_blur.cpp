#include <algorithm>
#include <emmintrin.h>
#include <exception>
#include <iostream>
#include <median_blur.h>
#include <stdint.h>
#include <tbb/blocked_range2d.h>
#include <tbb/partitioner.h>
#include <tbb/tbb.h>
#include <type_traits>
#include <vector>


namespace pip {

// TODO: design base filter
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

template <typename arr_container>
void blur_2D(const Image &img_i, Image &img_o, int row, int col, int ch,
             int mask_w, int mask_h, arr_container &arr) {
  // std::vector<Image::value_type> arr;
  int n = mask_h * mask_w;
  // arr.reserve(n);

  for (int r = row - (mask_h >> 1); r <= row + (mask_h >> 1); ++r) {
    for (int c = col - (mask_w >> 1); c <= col + (mask_w >> 1); ++c) {
      arr.push_back(img_i(c, r, ch));
    }
  }

  std::nth_element(arr.begin(), arr.begin() + n / 2, arr.end());
  img_o(col, row, ch) = arr[n >> 1];
  return;
}

void median_blur(const Image &in_img, Image &out_img, int mask_width,
                 int mask_height) {
  auto [width, height, channel] = in_img.shape();
  out_img.reshape(width, height, channel);
  constexpr size_t blocksize = 64;

  for (int ch = 0; ch < channel; ++ch) {
    tbb::parallel_for(
        tbb::blocked_range2d<size_t, size_t>{0, height, blocksize, 0, width,
                                             blocksize},
        [&](const tbb::blocked_range2d<size_t, size_t> &range) {
          // std::cout << "row:" << ((r.rows().begin() + r.rows().end()) >> 1)
          //           << "col:" << ((r.cols().begin() + r.cols().end()) >> 1)
          //           << std::endl;

          // thread local
          thread_local std::vector<Image::value_type> arr;
          arr.reserve(mask_height * mask_width);

          int r_begin = range.rows().begin();
          int r_end = range.rows().end();
          int c_begin = range.cols().begin();
          int c_end = range.cols().end();

          for (int r = r_begin; r < r_end; ++r) {
            for (int c = c_begin; c < c_end; ++c) {
              blur_2D(in_img, out_img, r, c, ch, mask_width, mask_height, arr);
              arr.clear();
            }
          }
        },
        tbb::simple_partitioner{});
  }
}

#pragma region minmax

template <uint16_t packed_signed_N_bit, typename T,
          typename = std::enable_if_t<sizeof(T) % packed_signed_N_bit == 0>>
void min_max(T &a, T &b) = delete;

template <> inline void min_max<1, uint8_t>(uint8_t &a, uint8_t &b) {
  uint8_t t = a;
  a = std::min(a, b);
  b = std::max(t, b);
}
template <> inline void min_max<1, uint16_t>(uint16_t &a, uint16_t &b) {
  uint16_t t = a;
  a = std::min(a, b);
  b = std::max(t, b);
}
template <> inline void min_max<1, float>(float &a, float &b) {
  float t = a;
  a = std::min(a, b);
  b = std::max(t, b);
}

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

template <> inline void min_max<32, __m256i>(__m256i &a, __m256i &b) {
  __m256i t = a;
  a = _mm256_min_epu8(a, b);
  b = _mm256_max_epu8(b, t);
}

template <> inline void min_max<16, __m256i>(__m256i &a, __m256i &b) {
  __m256i t = _mm256_subs_epu16(a, b);
  a = _mm256_subs_epu16(a, t);
  b = _mm256_adds_epu16(b, t);
}

template <> inline void min_max<8, __m256>(__m256 &a, __m256 &b) {
  __m256 t = a;
  a = _mm256_min_ps(a, b);
  b = _mm256_max_ps(b, t);
}

template <> inline void min_max<16, __m128i>(__m128i &a, __m128i &b) {
  __m128i t = a;
  a = _mm_min_epu8(a, b);
  b = _mm_max_epu8(b, t);
}

template <> inline void min_max<8, __m128i>(__m128i &a, __m128i &b) {
  __m128i t = _mm_subs_epu16(a, b);
  a = _mm_subs_epu16(a, t);
  b = _mm_adds_epu16(b, t);
}

template <> inline void min_max<4, __m128>(__m128 &a, __m128 &b) {
  __m128 t = a;
  a = _mm_min_ps(a, b);
  b = _mm_max_ps(b, t);
}

#pragma endregion

template <uint16_t simdWidth, typename T>
void pop_min_max_to_end(T *arr, uint32_t left, uint32_t right) {
  for (uint32_t i = left; i < right; i += 2)
    min_max<simdWidth>(arr[i], arr[i + 1]);

  for (uint32_t step = 2; step < (right - left + 1); step *= 2) {
    for (uint32_t i = left; (i + step) <= right; i += 2 * step) {
      min_max<simdWidth>(arr[i], arr[i + step]);
      min_max<simdWidth>(arr[i + step - 1],
                         arr[std::min(right, i + 2 * step - 1)]);
    }
  }
}

} // namespace pip