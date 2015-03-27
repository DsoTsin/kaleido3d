#include "Kaleido3D.h"
#include "Shader.h"

namespace k3d {
  namespace d3d12 {
	  HRESULT Shader::Compile(
		  const char* fileName,
		  const char* entryPoint,
		  const char* target)
	  {
		  wchar_t wFileName[1024] = { 0 };
		  StringUtil::CharToWchar(fileName, wFileName, 1024);
		  HRESULT hr = ::D3DCompileFromFile(
			  wFileName, nullptr, nullptr,
			  entryPoint, target, D3DCOMPILE_WARNINGS_ARE_ERRORS, 0,
			  mBlob.GetInitReference(), mErrBlob.GetInitReference());

		  if (mErrBlob)
		  {
			  Debug::Out("Shader", "Load Error : %s", reinterpret_cast<LPCSTR>(mErrBlob->GetBufferPointer()));
		  }

		  return hr;
	  }

	  HRESULT Shader::Load(const char* CacheFileName)
	  {
		  wchar_t wFileName[1024] = { 0 };
		  StringUtil::CharToWchar(CacheFileName, wFileName, 1024);
		  HRESULT hr = ::D3DReadFileToBlob(wFileName, mBlob.GetInitReference());
		  return hr;
	  }
  }
}