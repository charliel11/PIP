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
  benchmark::DoNotOptimize(a);
}
BM(resize_1);

BENCHMARK_MAIN();