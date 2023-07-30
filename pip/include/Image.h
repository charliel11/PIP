#pragma once

#include <alignalloc.h>
#include <ndarray.h>
#include <stdint.h>
#include <variant>

namespace pip {

using Image = ndarray<3, uint8_t, 8>;
using ImageU16 = ndarray<3, uint16_t, 8>;
using ImageF32 = ndarray<3, float, 0>;

// using Image = std::variant<ImageU8, ImageU16, ImageF32>;

// struct ImageVisiter {
//   ImageU8 operator()(const ImageU8 &i) { return i; }
//   ImageU16 operator()(const ImageU16 &i) { return i; }
//   ImageF32 operator()(const ImageF32 &i) { return i; }
// };

template <typename T>
using Vector_Aligned64 = std::vector<T, AlignedAllocator<T>>;

template <typename T>
using Vector_Aligned32 = std::vector<T, AlignedAllocator<T, 32>>;

} // namespace pip
