#include "Kaleido3D.h"

#include <Core/LogUtil.h>
#include <Core/Window.h>
#include <Core/Message.h>
#include <Core/AsynMeshTask.h>
#include <Core/ConditionVariable.h>
#include <Core/AssetManager.h>

#include <Renderer/MOGL/OGLRenderer.h>
#include <Renderer/MOGL/OGLBuffer.h>
#include <Renderer/MOGL/OGLTexture.h>
#include <Renderer/MOGL/OGLCmdList.h>
#include <Renderer/MOGL/OGLShader.h>

#include <Renderer/MOGL/OGLCmdList.h>

#include <GL/gl.h>

using namespace k3d;
#define TAG "AZDORenderer"


/**
  * AZDORenderer implementation
  */
class AZDORenderer : public OGLRenderer, public AsynMeshTask 
{
public:
  AZDORenderer( OGLDevice * device, const char * meshPackName ) 
    : OGLRenderer( device ), AsynMeshTask( meshPackName )
    , mIsDataLoaded( false ), mIsSceneReady( false ) {
    Debug::Out( TAG, "AZDORenderer initialized." );
  }

  ~AZDORenderer() {
    Debug::Out( TAG, "AZDORenderer deinitialize." );
  }

  void DrawOneFrame() override {
    if ( mIsDataLoaded )
    {
      InitResources();
      mIsDataLoaded = false;
      mIsSceneReady = true;
    }

    if ( mIsSceneReady ) 
    {

    }
  }

  void OnFinish() override
  {
    Debug::Out( TAG, "Mesh loaded." );
    mIsDataLoaded = true;
  }

  void InitResources() {
    for ( SpMesh& m : m_MeshPtrList )
    {
      OGLBufferDescriptor descriptor;
      descriptor.Data = reinterpret_cast<void*>(m->GetIndexBuffer());
      descriptor.Size = m->GetIndexNum() * sizeof( uint32 );
      OGLBuffer *ibo = new OGLBuffer;
      ibo->Allocate( descriptor );
      ibo->MakeResident();
      /*
      NVTokenIbo ibo;
      ibo.setType( GL_UNSIGNED_INT );
      ibo.setBuffer( obj.ibo, obj.iboADDR );
      nvtokenEnqueue( stream, ibo );
      */
    }
  }

private:
  bool mIsDataLoaded;
  bool mIsSceneReady;
};


int main( int, const char** )
{
  Window window( "UTAZDOOpenGL", 1700, 680 );
  window.Show();
  GLDeviceFormat fmt;
  OGLDevice * glDevice = OGLDevice::CreateGLDevice( &window, fmt);
  OGLRenderer * glRender = new AZDORenderer( glDevice );
  Message msg;
  while ( window.IsOpen() )
  {
    bool isQuit = false;
    while ( window.PollMessage( msg ) )
    {
      if ( msg.type == Message::MouseButtonPressed )
      {
        Debug::Out( "Main", "leftbutton" );
      }
      else if ( msg.type == Message::Closed )
      {
        isQuit = true;
        break;
      }
    }
    glRender->PrepareFrame();
    glRender->DrawOneFrame();
    glRender->EndOneFrame();
    if ( isQuit )
      break;
  }
}