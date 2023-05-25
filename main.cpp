#include <Image.h>
#include <iostream>
#include <median_blur.h>
#include <rwimage.h>
#include <vector>

using namespace std;
using namespace pip;

int main() {
  Image a, b, c;
  read_image(a, (std::string(PROJECTDIR) + "/original.jpg").c_str());

  median_blur(a, b, 15, 15);
  write_image(b, (std::string(PROJECTDIR) + "/test2.jpg").c_str());
  median_blur(a, c, 13, 13);
  write_image(c, (std::string(PROJECTDIR) + "/test1.jpg").c_str());
  // median_blur(a, b, 7, 7);
  // write_image(b, (std::string(PROJECTDIR) + "/test7.jpg").c_str());
  // median_blur(a, b, 9, 9);
  // write_image(b, (std::string(PROJECTDIR) + "/test9.jpg").c_str());
  // auto mp = tbb::global_control::max_allowed_parallelism;
  // tbb::global_control gc(mp, 1);
  // int ss = 1;
  // cout << "Hello PIP!" << endl;
  // tbb::parallel_invoke([]() { cout << " Hello " << endl; }, []() { cout << "
  // TBB! " << endl; });
  return 0;
}