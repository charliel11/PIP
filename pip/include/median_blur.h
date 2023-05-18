#pragma once

#include <Image.h>

namespace pip {

void median_blur(const Image &in, Image &out, int mask_width, int mask_height);

}