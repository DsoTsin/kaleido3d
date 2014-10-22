  solution "AllKaleido3D"

	-- Multithreaded compiling
	if _ACTION == "vs2010" or _ACTION=="vs2013" then
		buildoptions { "/MP"  }
	end 
	
	act = ""
    
    if _ACTION then
        act = _ACTION
    end

	configurations {"Release", "Debug"}
	configuration "Release"
		flags { "Optimize", "EnableSSE2","StaticRuntime", "NoMinimalRebuild", "FloatFast"}
	configuration "Debug"
		defines {"_DEBUG=1"}
		flags { "Symbols", "StaticRuntime" , "NoMinimalRebuild", "NoEditAndContinue" ,"FloatFast"}
		
	if os.is("Linux") then
		if os.is64bit() then
			platforms {"x64"}
		else
			platforms {"x32"}
		end
	else
		platforms {"x64"}
	end

	configuration "x64"		
		targetsuffix ("_" .. act .. "_64" )
	configuration {"x64", "debug"}
		targetsuffix ("_" .. act .. "_x64d")
	configuration {"x64", "release"}
		targetsuffix ("_" .. act .. "_x64" )
	
	configuration{}

	postfix=""

	if _ACTION == "xcode4" then
			xcodebuildsettings
			{
        		'ARCHS = "$(ARCHS_STANDARD_32_BIT) $(ARCHS_STANDARD_64_BIT)"',
        		'VALID_ARCHS = "x86_64 i386"',
			}
	end

	
	--flags { "NoRTTI", "NoExceptions"}
	--defines { "_HAS_EXCEPTIONS=0" }
	targetdir "../Bin"
	location("./" .. act .. postfix)

	
	projectRootDir = os.getcwd() .. "/../"
	print("Project root directroy: " .. projectRootDir);
	
	language "C++"
	--dofile("findBoost.lua")
	--include "../3rdParty/JsonCpp"
	--include "../Source/RenderGL"
	--include "../Source/EngineCore"
	include "../Source"
