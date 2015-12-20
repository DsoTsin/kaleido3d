project "Core"
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
   "App.h",
   "MacOS/App.mm",
   --'MacOS/AAPLAppDelegate.h",
   --"MacOS/AAPLAppDelegate.mm",
   --"MacOS/main.mm",
   "MacOS/ModuleCore.mm",
   --"Apple/AAPLView.h",
   --"Apple/AAPLView.mm",
   --"Apple/AAPLViewController.h",
   --"Apple/AAPLViewController.mm",
   "LogUtil.h", 
   --"LogUtil.cpp",
   "MacOS/LogUtil.mm",
   "Utils/MD5.h",
   "Utils/MD5.cpp",
   "File.h",
   "File.cpp",
   "Mesh.h",
   "Mesh.cpp",
   "AssetManager.h",
   "AssetManager.cpp",
   "Image.h",
   "Image.cpp",
   "Window.h",
   "MacOS/Window.mm",
   "ModuleCore.h",
   "ModuleCore.cpp"
   }
