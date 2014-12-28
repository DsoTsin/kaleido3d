#pragma once

#include <maya/MViewportRenderer.h>
#include <Renderer/MOGL/OGLRenderer.h>

// use OpenGL API 
class MDagPath;
class MBoundingBox;

class Mk3dRenderer : public MViewportRenderer, public k3d::OGLRenderer
{
public:
	explicit Mk3dRenderer(const MString & name = "Kaleido3D Renderer");
	virtual ~Mk3dRenderer();
	virtual	MStatus	initialize() override;
	virtual	MStatus	uninitialize() override;
	virtual MStatus	render(const MRenderingInfo &renderInfo) override;
	virtual bool	nativelySupports(MViewportRenderer::RenderingAPI api, float version) override;
	virtual bool	override(MViewportRenderer::RenderingOverride _override) override;

protected:
	bool			drawSurface(const MDagPath &dagPath, bool active, bool templated);
	bool			drawBounds(const MDagPath &dagPath, const MBoundingBox &box);
	bool			setupLighting();
	bool			renderToTarget(const MRenderingInfo &renderInfo);

	RenderingAPI	m_API;		// Rendering API
	float			m_Version;	// OpenGL version number as float.
};

//
//class Mk3dRendererHUD : public Mk3dRenderer
//{
//public:
//	Mk3dRendererHUD();
//	virtual unsigned int	overrideThenStandardExclusion() const override;
//};
//
//class Mk3dRendererFullUI : public Mk3dRenderer
//{
//public:
//	Mk3dRendererFullUI();
//};
