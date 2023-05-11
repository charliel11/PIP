#include <Image.h>
#include <MedianFilter.h>
#include <benchmark/benchmark.h>
#include <iostream>
#include <oneapi/tbb/global_control.h>
#include <rwimage.h>
#include <tbb/tbb.h>
#include <vcruntime.h>
#include <vector>

using namespace std;
using namespace pip;

int main() {
  Image a, b;
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  write_image(a, (std::string(PROJECTDIR) + "/test.jpg").c_str());
  auto mp = tbb::global_control::max_allowed_parallelism;
  tbb::global_control gc(mp, 1);
  MedianFilter(a, a, 3, 3);
  int ss = 1;
  cout << "Hello PIP!" << endl;
  tbb::parallel_invoke([]() { cout << " Hello " << endl; }, []() { cout << " TBB! " << endl; });
  return 0;
}