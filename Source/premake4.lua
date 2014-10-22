project "EngineCore"

	language "C++"
				
	kind "StaticLib"
		
	includedirs {
		"..",
		"../../Include",
		"D:/lib/bullet-2.82-r2704/src"
	}
	targetdir "../../Lib"

	files {
		"Engine/*.cpp",
		"Engine/*.h",
		"Core/*.cpp",
		"Core/*.h"
	}