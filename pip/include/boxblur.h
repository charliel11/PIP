#pragma once

#include <Image.h>

namespace pip {

void xblur(Image &b, Image const &a, int nblur);
void yblur(Image &b, Image const &a, int nblur);
void boxblur(Image &a, int nxblur, int nyblur);

} // namespace pip
