#pragma once

#include <Image.h>

namespace pip {

void resize(const Image &in, ImageF32 &out, std::size_t width,
            std::size_t height);

}