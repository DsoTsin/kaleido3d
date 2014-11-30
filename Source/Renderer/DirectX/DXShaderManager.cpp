#include "Kaleido3D.h"
#include "DXShaderManager.h"

#include <Core/Utils/StringUtils.h>
#include <Core/AssetManager.h>


namespace k3d {

	SpShader DXShaderManager::LoadShader(const char * shaderPath, const char * entryName, const char * shaderModel)
	{
		AssetManager::SpIODevice shaderFile = nullptr;
		SpShader	shader;
		// first step -- check shader cache
		std::string cCachePath = GenerateShaderCachePath(shaderPath, entryName, shaderModel);
		shaderFile = AssetManager::OpenAsset(cCachePath.c_str());
		if (shaderFile)
		{
				
			return shader;
		}

		// second step -- load shader source
		shaderFile = AssetManager::OpenAsset(shaderPath);
		if (shaderFile)
		{
			
		}


		return shader;
	}
}