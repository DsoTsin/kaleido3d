#include "Shader.h"

NS_K3D_METAL_BEGIN

Library::Library(id<MTLDevice> device)
: m_Device(device)
{
}

void Library::LoadFromFile(const char *name) {
    NSString *fileName = [[NSString alloc] initWithUTF8String:name];
    m_ShaderLib = [m_Device newLibraryWithFile:fileName error:nullptr];
}

Shader* Library::GetShaderFunction(const char * functionName) {
    Shader * shader = new Shader;
    shader->m_Function = [m_ShaderLib newFunctionWithName:
                          [[NSString alloc] initWithUTF8String:functionName]];
    return shader;
}


Shader::Shader() {
    
}

NS_K3D_METAL_END