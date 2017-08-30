#pragma once    
#include "Common.h"

NS_K3D_METAL_BEGIN

class Shader;

class Library {
public:
    explicit Library(id<MTLDevice> device);
    
    void LoadFromFile(const char* name);
    Shader * GetShaderFunction(const char *functionName);
    
private:
    id<MTLDevice> m_Device;
    id<MTLLibrary> m_ShaderLib;
};

class Shader {
public:
    Shader();
    
private:
    friend class Library;
    
    id<MTLFunction> m_Function;
};

NS_K3D_METAL_END