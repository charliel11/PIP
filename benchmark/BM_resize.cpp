#include "Image.h"
#include <benchmark/benchmark.h>
#include <resize.h>
#include <rwimage.h>
#include <tbb/global_control.h>

#define BM(func) BENCHMARK(BM_##func)->Unit(benchmark::kMillisecond)

using namespace pip;

static void BM_resize_1(benchmark::State &state) {
  Image a;
  ImageF32 b;
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  for (auto _ : state) {
    resize(a, b, 512, 512);
  }
  benchmark::DoNotOptimize(b);
}
// BM(resize_1);

constexpr size_t n = 1 << 28;
Vector_Aligned64<float> a(n, 0);

void BM_serial_add(benchmark::State &state) {
  for (auto _ : state) {
    for (size_t i = 0; i < n; ++i) {
      a[i] = a[i] + 1;
    }
    benchmark::DoNotOptimize(a);
  }
}
BM(serial_add);

void BM_simd_add(benchmark::State &state) {
  for (auto _ : state) {
    for (size_t i = 0; i < n; i += 8) {
      __m256 simd = _mm256_load_ps(&a[i]);
      _mm256_stream_ps(&a[i], simd + 1);
    }
    benchmark::DoNotOptimize(a);
  }
}
BM(simd_add);

// void BM_parallel_add(benchmark::State &state) {
//   for (auto _ : state) {
// #pragma omp parallel for
//     for (size_t i = 0; i < n; ++i) {
//       a[i] = a[i] + 1;
//     }
//     benchmark::DoNotOptimize(a);
//   }
// }
// BM(parallel_add);

BENCHMARK_MAIN();