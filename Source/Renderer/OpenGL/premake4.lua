project "RenderGL"

	language "C++"
				
	kind "StaticLib"
	
	defines {"GLEW_STATIC"}
	
	includedirs {
		"..",
		"../../Include",
		"D:/lib/bullet-2.82-r2704/src"
	}
	
	targetdir "../../Lib"
	
	files {
		"**.h",
		"**.cpp" 
	}