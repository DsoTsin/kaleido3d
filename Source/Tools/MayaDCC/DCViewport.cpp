#include "DCViewport.h"
#include <Renderer/MOGL/CommonGL.hpp>

Mk3dRenderer::Mk3dRenderer(const MString & name)
	: OGLRenderer( nullptr )
{
	// Set the ui name
	fUIName.set("Kaleido3D Renderer");

	// This renderer overrides all drawing
	fRenderingOverride = MViewportRenderer::kOverrideThenStandard;

	// Set API and version number
	m_API = MViewportRenderer::kOpenGL;
	m_Version = 4.5f;
}

Mk3dRenderer::~Mk3dRenderer()
{
}

MStatus Mk3dRenderer::initialize()
{
	// Now initialize OpenGL Functions
	GLInitializer::InitAndCheck();
	return MStatus::kSuccess;
}

MStatus Mk3dRenderer::uninitialize()
{
	return MStatus::kSuccess;
}

MStatus Mk3dRenderer::render(const MRenderingInfo & renderInfo)
{
	return MStatus::kSuccess;
}

bool Mk3dRenderer::nativelySupports(MViewportRenderer::RenderingAPI api, float version)
{
	return false;
}

bool Mk3dRenderer::override(MViewportRenderer::RenderingOverride _override)
{
	return (_override == fRenderingOverride);
}

bool Mk3dRenderer::drawSurface(const MDagPath & dagPath, bool active, bool templated)
{
	return false;
}

bool Mk3dRenderer::drawBounds(const MDagPath & dagPath, const MBoundingBox & box)
{
	return false;
}

bool Mk3dRenderer::setupLighting()
{
	return false;
}

bool Mk3dRenderer::renderToTarget(const MRenderingInfo & renderInfo)
{
	return false;
}
//
//Mk3dRendererHUD::Mk3dRendererHUD()
//	: Mk3dRenderer("OpenGLViewportRendererHUD")
//{
//	fUIName.set("Plugin OpenGL Renderer HUD");
//	fRenderingOverride = MViewportRenderer::kOverrideThenStandard;
//}
//
//unsigned int Mk3dRendererHUD::overrideThenStandardExclusion() const
//{
//	// exclude everything apart from manipulators
//	return ~(unsigned int)kExcludeManipulators;
//}
//
//Mk3dRendererFullUI::Mk3dRendererFullUI()
//	: Mk3dRenderer("OpenGLViewportRendererFullUI")
//{
//	fUIName.set("Plugin OpenGL Renderer Full UI");
//	fRenderingOverride = MViewportRenderer::kOverrideThenUI;
//}
