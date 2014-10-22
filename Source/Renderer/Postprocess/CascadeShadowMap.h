#pragma once
#include <Math/kMath.hpp>
#include <Config/Prerequisities.h>

class kCSMLight {
public:

  kCSMLight(int width, int height, int nCascades);
  ~kCSMLight();

  void		SetIntensity(const kMath::Vec3f& intensity);
  void		SetDirection(const kMath::Vec3f& direction);
private:
  K3DDISCOPY(kCSMLight);

public:
  kMath::Vec3f 					m_LightDir;	// Light direction points the direction of the light flux
  kMath::Mat4f					m_LightView;		// View matrix (center on cam pos)
  kMath::Mat4f					m_CamView;	// Camera view matrix used to generated the current CSM
  kMath::Mat4f*					m_Projs;		// Proj matrices for each split
  kMath::Mat4f*					m_ViewProjs;	// ViewProj matrices for each split
  float*						m_NearPlanes;
  float*						m_FarPlanes;
  kMath::Vec3f		 			m_Intensity;
  int							m_NCascades;
  class k3dTexture*             m_DepthTexs; // create 2d array
  class k3dTexture*				m_TmpTexs;	// Store linear moment for VSM and EVSM
  class k3dTexture*				m_MomentTexs;	// Store linear moment for VSM and EVSM
  class k3dTexture*				m_FilterTexs;	// Store filtered linear moment for VSM and EVSM
  uint32						tmpFBO;
  uint32						depthFBO;
  uint32						filterFBO;
};

class kCSMBuilder {
public:
  kCSMBuilder();
  void    Draw( kCSMLight&						_light,
                const class k3dCamera&          _camera,
                float 							_cascadeAlpha,
                float 							_blendFactor,
                const class k3dSceneManager&    _scene);
private:
  K3DDISCOPY(kCSMBuilder);

public:

//  struct RegularRenderer
//  {
//    RegularRenderer():program("CSMBuilder::RegularRenderer"){}
////    Program 				program;
//    uint32 					projVar;
//    uint32 					viewVar;
//    uint32 					modelVar;
//    uint32 					nCascadesVar;
//  };

//  struct TerrainRenderer
//  {
//    TerrainRenderer():program("CSMBuilder::TerrainRenderer"){}
//    Program 				program;
//    uint32 					projVar;
//    uint32 					viewVar;
//    uint32 					nCascadesVar;

//    uint32					heightTexUnit;
//    uint32 					tileSizeVar;
//    uint32 					tileCountVar;
//    uint32 					tileOffsetVar;
//    uint32 					projFactorVar;
//    uint32 					tessFactorVar;
//    uint32					heightFactorVar;
//  };

//  struct MomentFilter
//  {
//    MomentFilter():program("CSMBuilder::MomentFilter"){}
//    Program 				program;
//    uint32 					momentTexUnit;
//    uint32 					directionVar;
//  };

//  int							maxCascades;
//  RegularRenderer				regularRenderer;
//  TerrainRenderer				terrainRenderer;
//  MomentFilter				momentFilter;

//  VertexBuffer2F				vbo;
//  VertexArray					vao;
};

class kCSMRenderer
{
public:
  kCSMRenderer(	int _w,
                int _h);
//  void  	Draw( const kCSMLight&	_light,
//                  const GBuffer&	_gbuffer,
//                  const glm::vec3&_viewPos,
//                  float 			_blendFactor,
//                  float 			_bias,
//                  RenderTarget&	_target);
private:
  K3DDISCOPY(kCSMRenderer);

public:
  uint32 						positionTexUnit;
  uint32 						diffuseTexUnit;
  uint32 						normalTexUnit;
  uint32 						shadowTexUnit;

  uint32						blendFactorVar;
  uint32						viewPosVar;
  uint32						lightDirVar;
  uint32 						lightViewProjsVar;
  uint32						lightIntensityVar;
  uint32						biasVar;
  uint32						nCascadesVar;

//  Program 					program;
};
