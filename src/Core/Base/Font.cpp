#include "CoreMinimal.h"
#include "Font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace k3d {
FontManager &FontManager::g() {
  static FontManager _f;
  return _f;
}

FontManager::FontManager() : m_libHandle(nullptr) {
  FT_Library ft = nullptr;
  FT_Init_FreeType(&ft);
  /* use custom memory management
  FT_New_Library( FT_Memory    memory,
                  FT_Library  *alibrary );
  use FT_Done_Library
  */
  m_libHandle = ft;
}

FontManager::~FontManager() {
  if (m_libHandle) {
    FT_Done_FreeType((FT_Library)m_libHandle);
    m_libHandle = nullptr;
  }
}

} // namespace k3d