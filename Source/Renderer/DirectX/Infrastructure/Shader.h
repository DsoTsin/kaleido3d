#pragma once

#include "../DXCommon.h"
#include "Core/Utils/StringUtils.h"
#include "Core/LogUtil.h"

namespace k3d
{
  namespace d3d12
  {
    class Shader
    {
    public:

      /**
      * Compile ShaderSource
      */
		HRESULT Compile(const char* fileName,
			const char* entryPoint,
			const char* target);

      /**
      * Load ShaderByteCode from File.
      */
		HRESULT Load(const char* CacheFileName);

      /**
       * Return ShaderByteCode
	   */
      PtrBlob GetBlob () const
      {
        return m_Blob;
      }

    private:
      PtrBlob m_Blob;
      PtrBlob m_ErrBlob;
    };
  }
}