#include <MedianBlur.h>
#include <algorithm>
#include <iostream>
#include <oneapi/tbb/partitioner.h>
#include <stdint.h>
#include <tbb/blocked_range2d.h>
#include <tbb/partitioner.h>
#include <tbb/tbb.h>

namespace pip {

// TODO: design base filter

void Blur_2D(const Image &inImg, Image &outImg, int row, int col, int ch, int mask_w, int mask_h) {
  std::vector<Image::value_type> arr;
  int n = mask_h * mask_w;
  arr.reserve(n);

  for (int r = row - (mask_h >> 1); r <= row + (mask_h >> 1); ++r) {
    for (int c = col - (mask_w >> 1); c <= col + (mask_w >> 1); ++c) {
      arr.push_back(inImg(c, r, ch));
    }
  }

  std::nth_element(arr.begin(), arr.begin() + n / 2, arr.end());
  outImg(col, row, ch) = arr[n >> 1];
  return;
}

void MedianBlur(const Image &in_img, Image &out_img, int mask_width, int mask_height) {
  auto [width, height, channel] = in_img.shape();
  out_img.reshape(width, height, channel);
  constexpr size_t blocksize = 16;

  for (int ch = 0; ch < channel; ++ch) {
    tbb::parallel_for(
        tbb::blocked_range2d<size_t, size_t>{0, height, blocksize, 0, width, blocksize},
        [&](const tbb::blocked_range2d<size_t, size_t> &range) {
          // std::cout << "row:" << ((r.rows().begin() + r.rows().end()) >> 1)
          //           << "col:" << ((r.cols().begin() + r.cols().end()) >> 1) << std::endl;

          // thread local
          int r_begin = range.rows().begin();
          int r_end = range.rows().end();
          int c_begin = range.cols().begin();
          int c_end = range.cols().end();

          for (int r = r_begin; r < r_end; ++r) {
            for (int c = c_begin; c < c_end; ++c) {
              Blur_2D(in_img, out_img, r, c, ch, mask_width, mask_height);
            }
          }
        },
        tbb::simple_partitioner{});
  }
}

} // namespace pip