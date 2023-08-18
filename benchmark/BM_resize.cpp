#include <benchmark/benchmark.h>
#include <resize.h>
#include <rwimage.h>
#include <tbb/global_control.h>

#define BM(func) BENCHMARK(BM_##func)->Unit(benchmark::kMillisecond)

using namespace pip;

Image a = read_image((std::string(PROJECTDIR) + "/original.jpg").c_str());
void BM_resize_512(benchmark::State &state) {
  for (auto _ : state) {
    Image b = resize(a, 512, 512);
  }
}
BM(resize_512);

void BM_resize_2048(benchmark::State &state) {
  for (auto _ : state) {
    Image b = resize(a, 2048, 2048);
  }
}
BM(resize_2048);

void BM_resize_6144(benchmark::State &state) {
  for (auto _ : state) {
    Image b = resize(a, 6144, 6144);
  }
}
BM(resize_6144);

constexpr size_t n = 1ull << 32;
void BM_serial_add(benchmark::State &state) {
  for (auto _ : state) {
    std::vector<pod<float>> in(n);
    for (size_t i = 0; i < n; ++i) {
      in[i] = 1;
    }
    benchmark::DoNotOptimize(in);
  }
}
// BM(serial_add);

void BM_serial_add1(benchmark::State &state) {
  for (auto _ : state) {
    std::vector<float> in(2);
    in.resize(10);
    for (size_t i = 0; i < n; ++i) {
      in[i] = 1;
    }
    benchmark::DoNotOptimize(in);
  }
}
// BM(serial_add1);

// void BM_serial_add(benchmark::State &state) {
//   Image a;
//   read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
//   size_t n = a.shape()[0] * a.shape()[1] * a.shape()[2];
//   Vector_Aligned64<uint8_t> in(n, 0);
//   Vector_Aligned64<float_t> out(n, 0);
//   for (auto _ : state) {
// #pragma omp parallel for
//     for (size_t i = 0; i < n; ++i) {
//       out[i] = in[i];
//     }
//   }
// }
// BM(serial_add);

// void BM_uAlign_add(benchmark::State &state) {
//   Image a;
//   read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
//   size_t n = a.shape()[0] * a.shape()[1] * a.shape()[2];
//   std::vector<uint8_t> in(n, 0);
//   std::vector<float_t> out(n, 0);
//   for (auto _ : state) {
// #pragma omp parallel for
//     for (size_t i = 0; i < n; ++i) {
//       out[i] = in[i];
//     }
//   }
// }
// BM(uAlign_add);

BENCHMARK_MAIN();