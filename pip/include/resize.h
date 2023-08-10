#pragma once

#include <Image.h>

namespace pip {

void resize(const Image &in, ImageF32 &out, size_t width, size_t height);

}