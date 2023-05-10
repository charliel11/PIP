#pragma once

#include <Image.h>

namespace pip {

void read_image(Image &a, const char *path);
void write_image(Image const &a, const char *path);

} // namespace pip
