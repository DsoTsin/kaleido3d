#include "PluginMain.h"
#include <Core/LogUtil.h>
#include <maya/MFnPlugin.h>


static Mk3dRenderer *gRenderer = 0;
//static Mk3dRendererHUD *gRendererHUD = 0;
//static Mk3dRendererFullUI *gRendererFullUI = 0;

MStatus initializePlugin( MObject obj )
{
	// Init Log
	MString k3dLogDir(getenv("KALEIDO3D_DIR"));
	k3d::Log::InitLogFile((k3dLogDir + "/Data/MayaToolkitLog.html").asChar());

	MStatus   status = MStatus::kFailure;
    MFnPlugin plug(obj, MAYA_PLUGIN_COMPANY, MAYA_PLUGIN_VERSION, MAYA_PLUGIN_REQUIREDAPI);

	gRenderer = new Mk3dRenderer();
	if (gRenderer)
	{
		status = gRenderer->registerRenderer();
		if (status != MStatus::kSuccess)
		{
			status.perror("Failed to register Kaleido3D renderer properly.");
		}
	}
	/*
	gRendererHUD = new Mk3dRendererHUD();
	if (gRendererHUD)
	{
		status = gRendererHUD->registerRenderer();
		if (status != MStatus::kSuccess)
		{
			status.perror("Failed to register Kaleido3D renderer properly.");
		}
	}
	gRendererFullUI = new Mk3dRendererFullUI();
	if (gRendererFullUI)
	{
		status = gRendererFullUI->registerRenderer();
		if (status != MStatus::kSuccess)
		{
			status.perror("Failed to register Kaleido3D renderer properly.");
		}
	}
	*/
	status = plug.registerFileTranslator(
		MAYA_TRANSLATOR,
        "none",
        DCTranslator::creator
    );
	if (status != MStatus::kSuccess)
	{
		status.perror("Failed to register Kaleido3D Translator properly.");
	}
	return status;
}
//////////////////////////////////////////////////////////////

MStatus uninitializePlugin( MObject obj )
{
    MFnPlugin plug( obj ); 
	MStatus   status = MStatus::kSuccess;

	// Deregister the renderer
	if (gRenderer)
	{
		status = gRenderer->deregisterRenderer();
		if (status != MStatus::kSuccess)
		{
			status.perror("Failed to deregister Kaleido3D renderer properly.");
		}
	}
	gRenderer = 0;
	/*
	if (gRendererHUD)
	{
		status = gRendererHUD->deregisterRenderer();
		if (status != MStatus::kSuccess)
		{
			status.perror("Failed to deregister Kaleido3D renderer properly.");
		}
	}
	gRendererHUD = 0;
	if (gRendererFullUI)
	{
		status = gRendererFullUI->deregisterRenderer();
		if (status != MStatus::kSuccess)
		{
			status.perror("Failed to deregister Kaleido3D renderer properly.");
		}
	}
	gRendererFullUI = 0; 
	*/
	status = plug.deregisterFileTranslator(MAYA_TRANSLATOR);
	if (status != MStatus::kSuccess)
	{
		status.perror("Failed to deregister Kaleido3D Translator properly.");
	}
	k3d::Log::CloseLog();
	return status;
}
