  solution "Kaleido3D"

	-- Multithreaded compiling
	if _ACTION == "vs2010" or _ACTION=="vs2013" then
		buildoptions { "/MP"  }
	end 
	
	act = ""
    
    if _ACTION then
        act = _ACTION
    end

   	configurations { "Debug", "Release" }

	postfix=""
	
	--flags { "NoRTTI", "NoExceptions"}
	--defines { "_HAS_EXCEPTIONS=0" }
	targetdir "../Bin"
	location("./" .. act .. postfix)

	
	projectRootDir = os.getcwd() .. "/../"
	print("Project root directroy: " .. projectRootDir);
	
	language "C++"
	--dofile("findBoost.lua")
	include "../Source/Core"
	include "../Source/RHI/Metal"
	include "../Source/Renderer"
	include "../Source/Example/01-Metal-Cube"
	include "../Source/UnitTest/RHI_Metal"
