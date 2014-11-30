project "Metal-Cube"

	language "C++"
				
	kind "WindowedApp"
		
	includedirs {
		"."
	}
	targetdir "../../../Bin"
	
	files {
		"Info.plist",
		"Base.lproj/MainMenu.xib",
		"**.metal",
		"**.m",
		"**.h"
	}
	linkoptions { "-framework MetalKit", "-framework Cocoa", "-framework Metal", "-framework ModelIO" }