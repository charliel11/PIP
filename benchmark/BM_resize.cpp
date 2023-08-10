#include <benchmark/benchmark.h>
#include <resize.h>
#include <rwimage.h>
#include <tbb/global_control.h>

#define BM(func) BENCHMARK(BM_##func)->Unit(benchmark::kMillisecond)

using namespace pip;

void BM_resize_512(benchmark::State &state) {
  Image a;
  ImageF32 b;
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  for (auto _ : state) {
    resize(a, b, 512, 512);
  }
  benchmark::DoNotOptimize(b);
}
BM(resize_512);

void BM_resize_2048(benchmark::State &state) {
  Image a;
  ImageF32 b;
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  for (auto _ : state) {
    resize(a, b, 2048, 2048);
  }
  benchmark::DoNotOptimize(b);
}
BM(resize_2048);

void BM_resize_6144(benchmark::State &state) {
  Image a;
  ImageF32 b;
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  for (auto _ : state) {
    resize(a, b, 6144, 6144);
  }
  benchmark::DoNotOptimize(b);
}
BM(resize_6144);

void BM_serial_add(benchmark::State &state) {
  Image a;
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  size_t n = a.shape()[0] * a.shape()[1] * a.shape()[2];
  Vector_Aligned64<uint8_t> in(n, 0);
  Vector_Aligned64<float_t> out(n, 0);
  for (auto _ : state) {
#pragma omp parallel for
    for (size_t i = 0; i < n; ++i) {
      out[i] = in[i];
    }
  }
}
BM(serial_add);

void BM_uAlign_add(benchmark::State &state) {
  Image a;
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  size_t n = a.shape()[0] * a.shape()[1] * a.shape()[2];
  std::vector<uint8_t> in(n, 0);
  std::vector<float_t> out(n, 0);
  for (auto _ : state) {
#pragma omp parallel for
    for (size_t i = 0; i < n; ++i) {
      out[i] = in[i];
    }
  }
}
BM(uAlign_add);

BENCHMARK_MAIN();