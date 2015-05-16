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
			  m_Blob.GetInitReference(), m_ErrBlob.GetInitReference());

		  if (m_ErrBlob)
		  {
			  Debug::Out("Shader", "Load Error : %s", reinterpret_cast<LPCSTR>(m_ErrBlob->GetBufferPointer()));
		  }

		  return hr;
	  }

	  HRESULT Shader::Load(const char* CacheFileName)
	  {
		  wchar_t wFileName[1024] = { 0 };
		  StringUtil::CharToWchar(CacheFileName, wFileName, 1024);
		  HRESULT hr = ::D3DReadFileToBlob(wFileName, m_Blob.GetInitReference());
		  return hr;
	  }
  }
}