#include <algorithm>
#include <cassert>
#include <cstring>
#include <math.h>
#include <numeric>
#include <x86intrin.h>

#include <omp.h>

#include <resize.h>

namespace pip {

#if defined(__AVX512BW__)

#define simd_float __m256
#define simd_round _mm256_round_ps
#define simd_storeu _mm256_storeu_ps
// #define simd_float __m512
// #define simd_round _mm512_roundscale_ps
// #define simd_storeu _mm512_storeu_ps

#elif defined(__AVX2__)

#define simd_float __m256
#define simd_round _mm256_round_ps
#define simd_storeu _mm256_storeu_ps

#elif defined(__SSE2__)

#define simd_float __m128
#define simd_round _mm_round_ps
#define simd_storeu _mm_storeu_ps

#else

#define __no_intrinic__
#error "not support"

#endif

void resize(const Image &in, ImageF32 &out, size_t width, size_t height) {

  size_t _width = in.shape()[0];
  size_t _height = in.shape()[1];
  size_t channel = in.shape()[2];

  out.reshape(width, height, channel);

  float w_ratio =
      static_cast<float>(_width - 1) / static_cast<float>(width - 1);
  float h_ratio =
      static_cast<float>(_height - 1) / static_cast<float>(height - 1);

  constexpr int simd_n = sizeof(simd_float) / sizeof(float);

  for (int ch = 0; ch < channel; ++ch) {
#pragma omp parallel for
    for (int r = 0; r < height; ++r) {
      simd_float int_part_c;
      simd_float fract_part_c;
      simd_float inv_fract_part_c;
      simd_float v1;
      simd_float v2;
      simd_float v3;
      simd_float v4;

      float int_part_r;
      float fract_part_r = modff(r * h_ratio, &int_part_r); // r-r0
      float inv_fract_part_r = 1.0 - fract_part_r;          // 1-(r-r0)
      int in_r = static_cast<int>(int_part_r);
      int in_next_r = std::min(in_r + 1, static_cast<int>(_height - 1));

      int c = 0;
      for (; c <= width - simd_n; c += simd_n) {
        float *t = reinterpret_cast<float *>(&fract_part_c);
        float startNum = static_cast<float>(c);
        std::generate(t, t + simd_n, [&] {
          float res = startNum * w_ratio;
          startNum += 1.0f;
          return res;
        });

        int_part_c = simd_round(fract_part_c, _MM_FROUND_TO_ZERO);
        fract_part_c -= int_part_c;
        inv_fract_part_c = 1.0 - fract_part_c;

        for (int i = 0; i < simd_n; ++i) {
          int in_c = static_cast<int>(int_part_c[i]);
          int in_next_c = std::min(in_c + 1, static_cast<int>(_width - 1));

          v1[i] = in(in_c, in_r, ch);           // A
          v2[i] = in(in_next_c, in_r, ch);      // B
          v3[i] = in(in_c, in_next_r, ch);      // C
          v4[i] = in(in_next_c, in_next_r, ch); // D
        }

        simd_float c1 = inv_fract_part_c * inv_fract_part_r;
        simd_float c2 = fract_part_c * inv_fract_part_r;
        simd_float c3 = inv_fract_part_c * fract_part_r;
        simd_float c4 = fract_part_c * fract_part_r;

        simd_float res = c1 * v1 + c2 * v2 + c3 * v3 + c4 * v4;

        simd_storeu(&out(c, r, ch), res);
      }

      for (; c < width; ++c) {
        float int_part_c;
        float fract_part_c = modff(c * w_ratio, &int_part_c);
        float inv_fract_part_c = 1.0 - fract_part_c;

        float c1 = inv_fract_part_c * inv_fract_part_r;
        float c2 = fract_part_c * inv_fract_part_r;
        float c3 = inv_fract_part_c * fract_part_r;
        float c4 = fract_part_c * fract_part_r;

        int in_c = static_cast<int>(int_part_c);
        int in_next_c = std::min(in_c + 1, static_cast<int>(_width - 1));

        float v1 = in(in_c, in_r, ch);           // A
        float v2 = in(in_next_c, in_r, ch);      // B
        float v3 = in(in_c, in_next_r, ch);      // C
        float v4 = in(in_next_c, in_next_r, ch); // D

        float res = c1 * v1 + c2 * v2 + c3 * v3 + c4 * v4;

        out(c, r, ch) = res;
      }
    }
  }
  return;
}

} // namespace pip