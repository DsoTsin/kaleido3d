project "RHI_Metal"
   kind "StaticLib"
   language "C++"
   --targetdir "bin/%{cfg.buildcfg}"

   files { 
   "Util/ImageUtil.h",
   "Util/ImageUtil.mm",
   "Renderer.h",
   "Renderer.mm",
   "Shader.h",
   "Shader.mm",
   "Public/MetalRHI.h",
   "Public/MetalRHIResource.h",
   "Private/MetalEnums.h",
   "Private/MetalDevice.mm",
   "Private/MetalCommandContext.mm",
   "Private/MetalPipelineState.mm",
   "Private/MetalResource.mm",
   "Private/MetalBuffer.mm",
   "Private/MetalTexture.mm"
   }

   includedirs {
      ".",
      "..",
      "../..",
      "../../../Include"
   }

   defines {
      "MAC_OSX=1"
   }

   buildoptions "-std=gnu++14 -stdlib=libc++"
