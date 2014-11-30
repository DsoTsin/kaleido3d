project "Renderer_Metal"
   kind "StaticLib"
   language "C++"
   --targetdir "bin/%{cfg.buildcfg}"

   files { 
   "Device.h", 
   "Device.mm",
   "Util/ImageUtil.h",
   "Util/ImageUtil.mm",
   "Renderer.h",
   "Renderer.mm",
   "Shader.h",
   "Shader.mm",
   "CommandContext.h",
   "CommandContext.mm"
   }

   includedirs {
      "../..",
      "../../../Include"
   }

   defines {
      "MAC_OSX=1"
   }

   buildoptions "-std=gnu++14 -stdlib=libc++"