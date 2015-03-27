#include "Kaleido3D.h"
#include "ShaderCompileWorker.h"

#include <Core/File.h>
#include <Core/LogUtil.h>
#include <Core/AssetManager.h>

namespace k3d
{
  namespace d3d12
  {
    void ShaderCompileWorker::OnRun ()
    {
      CompileFromFile(nullptr, nullptr);
    }

    void ShaderCompileWorker::CompileFromFile (D3D_SHADER_MACRO * macro, ID3DInclude * include)
    {
      if (mShaderByteCode!=nullptr && mShaderByteCode->GetBufferSize () > 0)
      {
        Debug::Out ("ShaderCompileWorker", "CompileShaderFromFile (%s) already loaded..", mShaderFilePath.c_str ());
        return;
      }

      if (mShaderFilePath.empty () || mShaderEntryPoint.empty () || mShaderModel.empty ())
      {
        Debug::Out ("ShaderCompileWorker",
                    "CompileShaderFromFile---DXShader is invalid: \"m_ShaderPath.empty() || m_ShaderEntryPoint.empty() || m_ShaderModel.empty()\"");
        return;
      }

      DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if (_WIN32_WINNT <= _WIN32_WINNT_WIN8)
#ifdef _DEBUG
      dwShaderFlags |= D3DCOMPILE_DEBUG;
      dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
#endif
      std::string realPath = AssetManager::AssetPath (mShaderFilePath.c_str ());
      /**
      // Generate Shader Cache Path
      std::string cCachePath = GenerateShaderCachePath (realPath.c_str (), mShaderEntryPoint.c_str (), mShaderModel.c_str ());
      // First: Check shader cache
      {
        File cache;
        if (cache.Open (cCachePath.c_str (), IORead))
        {
          int64 length = cache.GetSize ();
          char * bytes = new char[ length ];
          cache.Read (bytes, length);
          m_Blob.DeepCopy (bytes, length, true);
          cache.Close ();
          Debug::Out ("ShaderCompileWorker", "shader cache loaded! size=%ld", length);
          return;
        }
      }
      */
      Ref<ID3DBlob> pErrorBlob;
      wchar_t szFileName[ 256 ] = { 0 };
      StringUtil::CharToWchar (realPath.c_str (), szFileName, sizeof (szFileName));

      HRESULT hr = D3DCompileFromFile (szFileName, macro, include, 
                                       mShaderEntryPoint.c_str (), mShaderModel.c_str (), dwShaderFlags, 
                                       0, mShaderByteCode.GetInitReference (), pErrorBlob.GetInitReference ());

      if (FAILED (hr))
      {
        if (pErrorBlob)
        {
          OutputDebugStringA (reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer ()));
          Log::Error ("Shader failed to compile. %s", reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer ()));
        }
        Debug::Out ("DXShader", "CompileShaderFromFile-- Failed to Compile (%s).", mShaderFilePath.c_str ());
        return;
      }

      Log::Message ("Shader Compile Completed!");
      /**
      m_Blob.DeepCopy (pBlobOut->GetBufferPointer (), pBlobOut->GetBufferSize ());

      // Now : save shader cache
      {
        File cache;
        if (cache.Open (cCachePath.c_str (), IOWrite))
        {
          cache.Write (m_Blob.Data (), m_Blob.Size ());
          cache.Close ();
          Log::Message ("shader cache saved!");
        }
      }
      **/
    }
  }
}