#include "kGLRender.h"
#include "kGLRenderManager.h"
#include "kGLRenderState.h"

#include <GL/glew.h>
#include <Core/k3dLog.h>
#include <Core/k3dDbg.h>
#include <Engine/kDebugDrawUtils.h>


kGLRender::kGLRender()
: k3dRender()
{
}

kGLRender::~kGLRender()
{

}

void kGLRender::Init(SharedWindowPtr window)
{
	if (!m_RenderManager)  m_RenderManager = new kGLRenderManager;
	m_RenderManager->CreateContext(window);
	m_RenderManager->Init();
	
	if (!m_RenderState)   
		m_RenderState = new kGLRenderState;

//	if (!m_RenderTexture && m_RenderManager)
//		m_RenderTexture = m_RenderManager->MakeRenderTexture();

	const float color[] = { 1.0f, 1.0f, 0.7f, 1.0f };
	m_RenderState->ClearBuffer(RState::BUFFER_COLOR | RState::BUFFER_DEPTH, color, 20.f, 1);
	m_RenderState->EnableDepthTest(true);

	m_DeferredRenderer.CreateDeferredRenderer(this);
}

void kGLRender::Shutdown()
{
	m_RenderManager->Shutdown();
}
