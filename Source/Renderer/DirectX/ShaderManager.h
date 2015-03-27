#pragma once
#include "DXCommon.h"

#include <map>

#include <KTL/NonCopyable.hpp>
#include <KTL/Singleton.hpp>

/*
namespace k3d {

	class ShaderManager : protected NonCopyable, public Singleton<ShaderManager>
	{
	public:

		SpShader LoadShader(const char * shaderRelativePath, const char * entryName, const char * shaderModel);

	private:
		typedef std::map<std::string, SpShader> ShaderSpMap;
		ShaderSpMap		m_ShaderMap;

	};

}
*/