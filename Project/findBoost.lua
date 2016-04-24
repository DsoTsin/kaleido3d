function findBoost()
		local boostDir = os.getenv("Boost_DIR")
		if (boostDir) then
			local filepath = string.format("%s%s",boostDir,"boost/version.hpp")
			headerBoost = io.open(filepath, "r")
			if (headerBoost) then
				 printf("Found Boost: '%s'", filepath)
				return true
			end
		end
		return false
	end

function initBoost()
	configuration {}
	
	local boostDir = os.getenv("Boost_DIR")
			includedirs {"$(Boost_DIR)"}
	
		configuration "x32"
			libdirs {"$(Boost_DIR)/stage/x86"}
		configuration "x64"
			libdirs {"$(Boost_DIR)/stage/x64"}
		configuration {}
		links {
		""
		}
		return true
end