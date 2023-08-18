#pragma once

#include <Image.h>

namespace pip {

Image read_image(const char *path);
void write_image(Image const &a, const char *path);

} // namespace pip
