#pragma once
#include "ngfx.h"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <unordered_map>

#ifdef BUILD_VULKAN_GLSLANG
#define V_API __declspec(dllexport)
#else
#define V_API __declspec(dllimport)
#endif

using ByteCode = std::vector<uint32_t>;

namespace spirv_cross
{
  class Compiler;
}

class V_API SPIRVCrossReflection : public ngfx::PipelineReflection
{
public:
  SPIRVCrossReflection(void* pData, uint32_t size);
  explicit SPIRVCrossReflection(const ByteCode&);
  ~SPIRVCrossReflection();

  uint32_t          VariableCount() const override;
  ngfx::Variable*   VariableAt(uint32_t id) const override;
  ngfx::ShaderType  GetStage() const override;

private:
  void DoReflect();
  spirv_cross::Compiler*      m_Reflector;
  std::vector<ngfx::Variable*> m_Vars;
};

struct FunctionData
{
  ByteCode          ByteCode;
  ngfx::ShaderType  Stage;
};

struct EntryInfo
{
  char      Name[128];
  char      Entry[128];
  uint32_t  ShaderType;
  uint32_t  Size;
  uint32_t  OffSet;
};

using FunctionMap = std::unordered_map<std::string, FunctionData>;

extern V_API 
ngfx::Result CompileFromSource(const ngfx::CompileOption& Opt, const char* pSource, FunctionMap& FuncMap, std::string& ErrorInfo);

extern V_API
ngfx::Result ReflectFromSPIRV(ByteCode const&bc, ngfx::PipelineReflection ** ppResult);

extern V_API
ngfx::Result SerializeLibrary(const FunctionMap& Data, const char* Path);