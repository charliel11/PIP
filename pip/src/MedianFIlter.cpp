#include <MedianFilter.h>
#include <stdint.h>
#include <tbb/blocked_range2d.h>
#include <tbb/partitioner.h>
#include <tbb/tbb.h>
#include <vcruntime.h>

namespace pip {

// TODO: design base filter

void BaseFilter(const Image &inImg, Image &outImg, int r, int c, int k, int mask_w, int mask_h) {
  std::vector<float *> row;
  row.resize(mask_h);
  return;
}

void MedianFilter(const Image &inImg, Image &outImg, int mask_width, int mask_height) {
  auto [w, h, c] = inImg.shape();
  constexpr size_t blocksize = 16;
  for (int k = 0; k < c; ++k) {
    tbb::parallel_for(
        tbb::blocked_range2d<size_t, size_t>{0, w, blocksize, 0, h, blocksize},
        [&](const tbb::blocked_range2d<size_t, size_t> &r) {
          int ie = r.cols().end();
          int je = r.rows().end();
          for (int i = r.rows().begin(); i < ie; ++i) {
            for (int j = r.cols().begin(); j < je; ++j) {
              BaseFilter(inImg, outImg, i, j, k, mask_width, mask_height);
            }
          }
        },
        tbb::simple_partitioner());
  }
}

} // namespace pip