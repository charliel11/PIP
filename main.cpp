#include <Image.h>
#include <benchmark/benchmark.h>
#include <iostream>
#include <median_blur.h>
#include <rwimage.h>
#include <tbb/global_control.h>
#include <tbb/tbb.h>
#include <vector>


using namespace std;
using namespace pip;

int main() {
  Image a, b;
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());
  median_blur(a, b, 3, 3);
  write_image(b, (std::string(PROJECTDIR) + "/test.jpg").c_str());
  // auto mp = tbb::global_control::max_allowed_parallelism;
  // tbb::global_control gc(mp, 1);
  // int ss = 1;
  // cout << "Hello PIP!" << endl;
  // tbb::parallel_invoke([]() { cout << " Hello " << endl; }, []() { cout << "
  // TBB! " << endl; });
  return 0;
}