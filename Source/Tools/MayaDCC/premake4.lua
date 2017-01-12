
project "Kaleido3D_DCC_ToolKit"
	language "C++"
	kind "SharedLib"
	
	defines {"NDEBUG", "_WINDOWS", "NT_PLUGIN", "REQUIRE_IOSTREAM"}
	
	includedirs {
		"D:/Autodesk/Maya2014/include",
		"D:/bin/Maya2014/include",
		"../../Include",
		"../../Source",
		"../../Include/Config"
	}
	
	pchheader "Prerequisities.h"
	
	libdirs {
		"D:/Autodesk/Maya2014/lib",
		"D:/bin/Maya2014/lib"
	}
	
	links { "OpenMaya", "OpenMayaUI", "Foundation", "OpenMayaRender"}
	linkoptions { "/export:initializePlugin /export:uninitializePlugin /SUBSYSTEM:WINDOWS" }
	
	files {
		"StaticMeshTranslator.cpp",
		"StaticMeshTranslator.h",
		"MayaToolKit.cpp",
		"MayaToolKit.h",
			"../../Source/Core/k3dFile.cpp",
			"../../Source/Core/JsonCpp/json_reader.cpp",
			"../../Source/Core/JsonCpp/json_value.cpp",
			"../../Source/Core/JsonCpp/json_writer.cpp",
			"../../Source/Core/k3dJsonObject.cpp",
			"../../Source/Core/k3dMesh.cpp",
			"../../Source/Core/k3dArchive.cpp",
			"../../Source/Core/k3dDbg.cpp",
			"../../Source/Core/k3dLog.cpp"
	}
