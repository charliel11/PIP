#include <MedianFilter.h>
#include <algorithm>
#include <iostream>
#include <stdint.h>
#include <tbb/blocked_range2d.h>
#include <tbb/partitioner.h>
#include <tbb/tbb.h>
#include <vcruntime.h>

namespace pip {

// TODO: design base filter

void BaseFilter(const Image &inImg, Image &outImg, int r, int c, int k, int mask_h, int mask_w) {
  std::vector<Image::value_type> row;
  int n = mask_h * mask_w;
  row.reserve(n);

  for (int i = r - mask_h; i < r + mask_h; ++i) {
    for (int j = c - mask_w; j < c + mask_w; ++j) {
      row.push_back(inImg(i, j, k));
    }
  }

  std::nth_element(row.begin(), row.begin() + n / 2, row.end());
  outImg(r, c, k) = row[n >> 1];
  return;
}

void MedianFilter(const Image &inImg, Image &outImg, int mask_width, int mask_height) {
  auto [w, h, c] = inImg.shape();
  outImg.reshape(w, h, c);
  constexpr size_t blocksize = 16;

  for (int k = 0; k < c; ++k) {
    tbb::parallel_for(
        tbb::blocked_range2d<size_t, size_t>{0, w, blocksize, 0, h, blocksize},
        [&](const tbb::blocked_range2d<size_t, size_t> &r) {
          // std::cout << "row:" << ((r.rows().begin() + r.rows().end()) >> 1)
          //           << "col:" << ((r.cols().begin() + r.cols().end()) >> 1) << std::endl;
          int ie = r.cols().end();
          int je = r.rows().end();

          for (int i = r.rows().begin(); i < ie; ++i) {
            for (int j = r.cols().begin(); j < je; ++j) {
              BaseFilter(inImg, outImg, i, j, k, mask_width, mask_height);
            }
          }
        },
        tbb::simple_partitioner{});
  }
}

} // namespace pip