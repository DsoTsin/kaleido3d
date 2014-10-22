#include "k3dMaterial.h"
#include <Core/k3dFile.h>
#include <Core/k3dDbg.h>
#include <Core/DDSHelper/DDSHelper.h>
#include <Renderer/k3dShader.h>
#include <Renderer/OpenGL/kGLTexture.h>
#include <Renderer/OpenGL/kGLShader.h>
#include <fstream>
#include <sstream>

using namespace std;
using namespace kMath;

BaseMaterialShader::BaseMaterialShader()
  : m_MatAttrib(0)
{
  m_MatShaderInstance = std::shared_ptr<kGLShader>(new kGLShader);
}

bool BaseMaterialShader::GenerateShaderFromMaterial(const BaseMaterialParam &params)
{
  GetMaterialAttrib( params );
  // generate single texooord data shader
  ostringstream vertexShader, fragmentShader;
  vertexShader << "#version 430\n"
               << "uniform mat4 gWorld;\n"
               << "uniform mat4 gView;\n"
               << "uniform mat4 gProj;\n"
               << "layout(location = 0) in vec3 position; \n"
               << "layout(location = 1) in vec3 normal; \n";

  fragmentShader << "#version 430\n"
                 << "layout(location = 0) out vec4 FragColor;\n"
                 << "in vec3 vPosition;\n"
                 << "in vec3 vNormal;\n";
  bool needTexCoord = (m_MatAttrib&Has_DiffuseMap) || (m_MatAttrib&Has_NormalMap) || (m_MatAttrib&Has_SpecularMap) || (m_MatAttrib&Has_AmbientMap);

  if(needTexCoord) {
    vertexShader << "layout(location = 2) in vec2 texcoord;\n";
    vertexShader << "out vec2 vTexCoord;\n";
    fragmentShader << "in vec2 vTexCoord;\n";
  }

  m_MatAttrib&Has_DiffuseMap ? fragmentShader << "uniform sampler2D diffuseMap;\n":fragmentShader << "uniform vec3 diffuseColor;\n";
  m_MatAttrib&Has_SpecularMap? fragmentShader << "uniform sampler2D specualrMap;\n" : fragmentShader << "uniform vec3 specularColor;\n";

  vertexShader << "out vec3 vPosition;\n";
  vertexShader << "out vec3 vNormal;\n";
  vertexShader << "void main()\n"
               << "{\nmat4 vm = gView*gWorld;\n"
               << "vec4 sPosition = vm*vec4(position,1.0);\n"
               << "vPosition = -sPosition.xyz;\n"
               << "vNormal = mat3(vm)*normal;\n";
  if(needTexCoord)  vertexShader << "vTexCoord = texcoord;\n";
  vertexShader << "gl_Position = gProj*sPosition;\n}";

  fragmentShader << "uniform vec3 lightPosition = vec3(10.0,10.0,10.0);\n"
                 << "uniform float specularPower = 200.0f;\n";
  fragmentShader << "void main()\n"
                 << "{\nvec3 N = normalize(vNormal);\n"
                 << "vec3 L = normalize(lightPosition + vPosition);\n"
                 << "vec3 V = normalize(vPosition);\n"
                 << "vec3 H = normalize(L+V);\n"
                 << "vec3 diffuse = max(dot(N,L), 0.0)*"
                 << ((m_MatAttrib&Has_DiffuseMap)?"texture(diffuseMap, vTexCoord).xyz;\n":"diffuseColor;\n")
                 << "vec3 specular = pow(max(dot(N,H), 0.0), specularPower)*"
                 << ((m_MatAttrib&Has_SpecularMap)?"texture(specularMap, vTexCoord).xyz;\n":"specularColor;\n")
                 << "FragColor = vec4(diffuse+specular, 1.0);\n}";

  bool vsBuild = m_MatShaderInstance->CompileShaderFromString(vertexShader.str(), k3dShader::VERTEX);
  bool fsBuild = m_MatShaderInstance->CompileShaderFromString(fragmentShader.str(), k3dShader::FRAGMENT);
  return vsBuild && fsBuild && m_MatShaderInstance->Link();
}

uint32 BaseMaterialShader::GetMaterialAttrib() const
{
  return m_MatAttrib;
}

void BaseMaterialShader::GetMaterialAttrib(const BaseMaterialParam &params)
{
  if( params.DiffuseMap != "" )   m_MatAttrib |= Has_DiffuseMap;
  if( params.SpecularMap != "" )  m_MatAttrib |= Has_SpecularMap;
  if( params.AmbientMap != "" )   m_MatAttrib |= Has_AmbientMap;
  if( params.NormalMap != "" )    m_MatAttrib |= Has_NormalMap;
  if( params.EmissiveMap != "" )  m_MatAttrib |= Has_EmissiveMap;
  if( params.TransparencyMap != "") m_MatAttrib |= Has_TransMap;
  if( params.DisplaceMap != "" ) m_MatAttrib |= Has_DisplacementMap;
}

k3dMaterial::k3dMaterial()
{
  m_MaterialType            = Material_Opaque;
  m_IsShaderLocationBinded  = false;
}

void k3dMaterial::SetMaterialType(int materialType)
{
  m_MaterialType = materialType;
}

void k3dMaterial::InitShaderVariableInternalBinding()
{

}

bool k3dMaterial::IsReflective() const
{
  return false;
}

bool k3dMaterial::IsEmissive() const
{
  return false;
}


int k3dMaterialBatch::GetBatchCount() const
{
  return (int)m_MaterialPtrs.size();
}

k3dMaterialBatch::SharedMaterialPtr k3dMaterialBatch::GetMaterialPtr(int i) const
{
  assert(i<m_MaterialPtrs.size() && "i is greater than material vector size!");
  return m_MaterialPtrs[i];
}
