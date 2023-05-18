// #define BM(func)                                                               \
//   static void BM_##func(benchmark::State &state) {                             \
//     for (auto _ : state) {                                                     \
//       std::vector<std::string> vec;                                            \
//       func(vec, global_str, '_');                                              \
//     }                                                                          \
//   }                                                                            \

#include <Image.h>
#include <benchmark/benchmark.h>
#include <median_blur.h>
#include <rwimage.h>
#include <tbb/global_control.h>
#include <tbb/tbb.h>

#define BM(func) BENCHMARK(func)->Unit(benchmark::kMillisecond)
#define N 5

using namespace pip;

Image a, b;
static void BM_MedianBlur33_8(benchmark::State &state) {
  auto mp = tbb::global_control::max_allowed_parallelism;
  tbb::global_control gc(mp, 16);
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  for (auto _ : state) {
    median_blur(a, b, N, N);
  }
}
BM(BM_MedianBlur33_8);

static void BM_MedianBlur33_4(benchmark::State &state) {
  auto mp = tbb::global_control::max_allowed_parallelism;
  tbb::global_control gc(mp, 8);
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  for (auto _ : state) {
    median_blur(a, b, N, N);
  }
}
BM(BM_MedianBlur33_4);

static void BM_MedianBlur33_1(benchmark::State &state) {
  auto mp = tbb::global_control::max_allowed_parallelism;
  tbb::global_control gc(mp, 1);
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  for (auto _ : state) {
    median_blur(a, b, N, N);
  }
}
BM(BM_MedianBlur33_1);

BENCHMARK_MAIN();