#include <algorithm>
#include <cassert>
#include <cstring>
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

void resize(const Image &in, ImageF32 &out, std::size_t width,
            std::size_t height) {

  auto [_width, _height, channel] = in.shape();
  out.reshape(width, height, channel);

  float w_ratio =
      static_cast<float>(_width - 1) / static_cast<float>(width - 1);
  float h_ratio =
      static_cast<float>(_height - 1) / static_cast<float>(height - 1);

  constexpr int simd_n = sizeof(simd_float) / sizeof(float);
  simd_float int_part_c;
  simd_float fract_part_c;
  simd_float inv_fract_part_c;
  simd_float v1;
  simd_float v2;
  simd_float v3;
  simd_float v4;

  // float fract_part_r[simd_n];
  // float inv_fract_part_r[simd_n];
  // float int_part_r[simd_n];

  for (int ch = 0; ch < channel; ++ch) {
    for (int r = 0; r < height; ++r) {

      float int_part_r;
      float fract_part_r = modf(r * h_ratio, &int_part_r); // r-r0
      float inv_fract_part_r = 1.0 - fract_part_r;         // 1-(r-r0)

      // for (int c = 0; c < width; ++c) {
      for (int c = 0; c <= width - simd_n; c += simd_n) {

        Vector_Aligned64<float> cc(simd_n);
        iota(cc.begin(), cc.end(), c);
        fract_part_c = _mm256_load_ps(cc.data());

        for (int i = 0; i < simd_n; ++i) {

          fract_part_c[i] = modf((c + i) * w_ratio, (float *)(&int_part_c + i));

          _mm256_round_ps(fract_part_c, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
          int in_c = (int)int_part_c[i];
          int in_r = (int)int_part_r;
          v1[i] = in(in_c, in_r, ch);
          v2[i] = in(in_c + 1, in_r, ch);
          v3[i] = in(in_c, in_r + 1, ch);
          v4[i] = in(in_c + 1, in_r + 1, ch);
        }
        inv_fract_part_c = _mm256_sub_ps(_mm256_set1_ps(1.0), fract_part_c);

        simd_float c1 =
            _mm256_mul_ps(inv_fract_part_c, _mm256_set1_ps(inv_fract_part_r));
        simd_float c2 =
            _mm256_mul_ps(fract_part_c, _mm256_set1_ps(inv_fract_part_r));
        simd_float c3 =
            _mm256_mul_ps(inv_fract_part_c, _mm256_set1_ps(fract_part_r));
        simd_float c4 =
            _mm256_mul_ps(fract_part_c, _mm256_set1_ps(fract_part_r));

        simd_float res = _mm256_mul_ps(c1, v1) + _mm256_mul_ps(c2, v2) +
                         _mm256_mul_ps(c3, v3) + _mm256_mul_ps(c4, v4);

        memcpy(&out(c, r, ch), &res, sizeof(simd_float));

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
        //     _mm256_set_pd(in(in_c, in_r, ch), in(in_c + 1, in_r, ch),
        //                   in(in_c, in_r + 1, ch), in(in_c + 1, in_r + 1,
        //                   ch));

        // value = _mm256_mul_pd(coef, value);

        // float sum1 = value[0] + value[1] + value[2] + value[3];
        // out(c, r, ch) = sum1;
        // float sum = in(in_c, in_r, ch) * inv_fract_part_c *
        // inv_fract_part_r
        // +
        //             in(in_c + 1, in_r, ch) * fract_part_c *
        //             inv_fract_part_r
        //             + in(in_c, in_r + 1, ch) * inv_fract_part_c *
        //             fract_part_r + in(in_c + 1, in_r + 1, ch) *
        //             fract_part_c
        //             * fract_part_r;

        // assert(sum1 == sum);

        // out(r, c, 0) = sum;
        // int s = 1;
      }
    }
  }
  return;
}

} // namespace pip