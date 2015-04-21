#pragma once

#include "DXCommon.h"
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
      HRESULT Compile (const char* fileName,
                       const char* entryPoint,
                       const char* target)
      {
        wchar_t wFileName[1024] = { 0 };
        StringUtil::CharToWchar (fileName, wFileName, 1024);
        HRESULT hr = ::D3DCompileFromFile (
          wFileName, nullptr, nullptr,
          entryPoint, target, D3DCOMPILE_WARNINGS_ARE_ERRORS, 0,
          mBlob.GetInitReference (), mErrBlob.GetInitReference ());

        if (mErrBlob)
        {
          Debug::Out ("Shader", "Load Error : %s", reinterpret_cast<LPCSTR>(mErrBlob->GetBufferPointer ()));
        }

        return hr;
      }

      /**
      * Load ShaderByteCode from File.
      */
      HRESULT Load (const char* CacheFileName)
      {
        wchar_t wFileName[1024] = { 0 };
        StringUtil::CharToWchar (CacheFileName, wFileName, 1024);
        HRESULT hr = ::D3DReadFileToBlob (wFileName, mBlob.GetInitReference ());
        return hr;
      }

      /**
       * Return ShaderByteCode
	   */
      PtrBlob GetBlob () const
      {
        return mBlob;
      }

    private:
      PtrBlob mBlob;
      PtrBlob mErrBlob;
    };
  }
}