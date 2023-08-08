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
BM(resize_1);

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
// BM(serial_add);

void BM_simd_add(benchmark::State &state) {
#define NUM_ELEMENTS (10000000)
  for (auto _ : state) {
    int *array = (int *)malloc(sizeof(int) * NUM_ELEMENTS);
    // Perform a memory-bound operation - access array elements in a loop
    int i;
    for (i = 0; i < NUM_ELEMENTS; i++) {
      array[i] = i;       // Write to the array (memory write)
      int val = array[i]; // Read from the array (memory read)
    }

    // Free the allocated memory
    free(array);
    benchmark::DoNotOptimize(array);
  }
}
// BM(simd_add);

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