#include <iostream>
#include <vector>

#include <Image.h>
#include <median_blur.h>
#include <resize.h>
#include <rwimage.h>

using namespace std;
using namespace pip;

int main() {

  Image a, b, c;
  ImageF32 d;
  a = read_image((std::string(PROJECTDIR) + "/original.jpg").c_str());

  // int s = a(0, 0, 0);
  // int s2 = a(0, 1, 0);
  // int ss = a(0, 0, 1);
  // b = a;
  // c = a;
  // // d = resize(a, 512, 512);

  // // median_blur(a, b, 15, 15);
  write_image(a, (std::string(PROJECTDIR) + "/test2.jpg").c_str());
  // // median_blur(a, c, 13, 13);
  // write_image(c, (std::string(PROJECTDIR) + "/test1.jpg").c_str());
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