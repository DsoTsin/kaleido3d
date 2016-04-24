project "UnitTestRHICommandContext"

	language "C++"
				
	kind "WindowedApp"
		
	includedirs {
		".",
		"../..",
		"../../RHI",
		"../../../Include"
	}

	libdirs {
		"../../../Bin"
	}

	links {
		"Core", "RHI_Metal"
	}

	targetdir "../../../Bin"
	
	files {
		"Info.plist",
		"Base.lproj/MainMenu.xib",
		"**.metal",
		"**.mm",
		"**.h"
	}
	linkoptions { "-framework MetalKit", "-framework Cocoa", "-framework Metal", "-framework ModelIO" }