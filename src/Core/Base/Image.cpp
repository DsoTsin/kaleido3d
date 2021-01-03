#include "CoreMinimal.h"
#include "Image.h"

#include "Image/spng/spng.h"

namespace k3d {
ImageFactory &ImageFactory::g() {
  static ImageFactory _if;
  return _if;
}

ImageFactory::ImageFactory() {}

ImageFactory::~ImageFactory() {}
} // namespace k3d