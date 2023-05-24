// #define BM(func)                                                               \
//   static void BM_##func(benchmark::State &state) {                             \
//     for (auto _ : state) {                                                     \
//       std::vector<std::string> vec;                                            \
//       func(vec, global_str, '_');                                              \
//     }                                                                          \
//   }                                                                            \

#include <benchmark/benchmark.h>
#include <median_blur.h>
#include <rwimage.h>
#include <tbb/global_control.h>

#define BM(func) BENCHMARK(BM_##func)->Unit(benchmark::kMillisecond)
#define N 17

using namespace pip;

static void BM_median_blur_1(benchmark::State &state) {
  Image a, b;
  auto mp = tbb::global_control::max_allowed_parallelism;
  tbb::global_control gc(mp, 1);
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  for (auto _ : state) {
    median_blur(a, b, N, N);
  }
}
BM(median_blur_1);

static void BM_median_blur_2(benchmark::State &state) {
  Image a, b;
  auto mp = tbb::global_control::max_allowed_parallelism;
  tbb::global_control gc(mp, 2);
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  for (auto _ : state) {
    median_blur(a, b, N, N);
  }
}
BM(median_blur_2);

static void BM_median_blur_4(benchmark::State &state) {
  Image a, b;
  auto mp = tbb::global_control::max_allowed_parallelism;
  tbb::global_control gc(mp, 4);
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  for (auto _ : state) {
    median_blur(a, b, N, N);
  }
}

BM(median_blur_4);

static void BM_median_blur_8(benchmark::State &state) {
  Image a, b;
  auto mp = tbb::global_control::max_allowed_parallelism;
  tbb::global_control gc(mp, 8);
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  for (auto _ : state) {
    median_blur(a, b, N, N);
  }
}

BM(median_blur_8);

static void BM_median_blur_16(benchmark::State &state) {
  Image a, b;
  auto mp = tbb::global_control::max_allowed_parallelism;
  tbb::global_control gc(mp, 16);
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  for (auto _ : state) {
    median_blur(a, b, N, N);
  }
}

BM(median_blur_16);

BENCHMARK_MAIN();