#pragma once

#include <alignalloc.h>
#include <ndarray.h>
#include <stdint.h>

namespace pip {

using Image = ndarray<3, uint8_t, 8>;

template <typename T>
using Vector_Aligned64 = std::vector<T, AlignedAllocator<T>>;

template <typename T>
using Vector_Aligned32 = std::vector<T, AlignedAllocator<T, 32>>;

} // namespace pip
