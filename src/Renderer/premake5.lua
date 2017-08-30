project "Render"
   kind "StaticLib"
   language "C++"

   includedirs {
      "..",
      "../../Include",
   }

   pchheader "../../Include/Kaleido3D.h"

   defines {
      "MAC_OSX=1"
   }

   buildoptions "-std=c++14 -stdlib=libc++"

   files { 
   "RenderContext.h",
   "RenderContext.cpp"
   }
