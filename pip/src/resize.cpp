#include <cassert>
#include <immintrin.h>
#include <math.h>
#include <numeric>
#include <x86intrin.h>

#include <resize.h>

namespace pip {

#if defined(__AVX512BW__)
#define simd_float __m512
#elif defined(__AVX2__)

#define simd_float __m256

#elif defined(__SSE2__)

#define simd_float __m128
#else

#define __no_intrinic__
#define simd_float float

#endif

void resize(const Image &in, Image &out, std::size_t width,
            std::size_t height) {
  auto [_width, _height, channel] = in.shape();
  out.reshape(width, height, channel);

  float w_ratio =
      static_cast<float>(_width - 1) / static_cast<float>(width - 1);
  float h_ratio =
      static_cast<float>(_height - 1) / static_cast<float>(height - 1);

  for (int ch = 0; ch < channel; ++ch) {
    for (int r = 0; r < height; ++r) {
      for (int c = 0; c < width; ++c) {

        constexpr int simd_n = sizeof(simd_float) / 32;

        Vector_Aligned64<float> col(simd_n);
        std::iota(col.begin(), col.end(), static_cast<float>(c));

        simd_float simd_col = _mm512_load_ps(col.data());

        // float int_part_c;

        // float fract_part_c = modf(c * w_ratio, &int_part_c); // c-c0
        // float inv_fract_part_c = 1.0 - fract_part_c;         // 1-(c-c0)

        // float int_part_r;
        // float fract_part_r = modf(r * h_ratio, &int_part_r); // r-r0
        // float inv_fract_part_r = 1.0 - fract_part_r;         // 1-(r-r0)

        // int in_c = (int)int_part_c;
        // int in_r = (int)int_part_r;
        // __m256d coef = _mm256_set_pd(inv_fract_part_c * inv_fract_part_r,
        //                              fract_part_c * inv_fract_part_r,
        //                              inv_fract_part_c * fract_part_r,
        //                              fract_part_c * fract_part_r);
        // __m256d value =
        //     _mm256_set_pd(in(in_r, in_c, ch), in(in_r, in_c + 1, ch),
        //                   in(in_r + 1, in_c, ch), in(in_r + 1, in_c + 1,
        //                   ch));

        // value = _mm256_mul_pd(coef, value);

        // float sum1 = value[0] + value[1] + value[2] + value[3];

        // float sum = in(in_r, in_c, ch) * inv_fract_part_c *
        // inv_fract_part_r
        // +
        //             in(in_r, in_c + 1, ch) * fract_part_c *
        //             inv_fract_part_r
        //             + in(in_r + 1, in_c, ch) * inv_fract_part_c *
        //             fract_part_r + in(in_r + 1, in_c + 1, ch) *
        //             fract_part_c
        //             * fract_part_r;

        // assert(sum1 == sum);

        // out(r, c, 0) = sum;
        int s = 1;
      }
    }
  }
  return;
}

} // namespace pip