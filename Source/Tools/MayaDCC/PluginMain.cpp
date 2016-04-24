#include "PluginMain.h"
#include <Core/LogUtil.h>
#include <maya/MFnPlugin.h>


static viewRenderOverrideMRT* viewRenderOverrideMRTInstance = NULL;

MStatus initializePlugin( MObject obj )
{
	// Init Log
	MString k3dLogDir(getenv("KALEIDO3D_DIR"));

	MStatus   status = MStatus::kFailure;
    MFnPlugin plug(obj, MAYA_PLUGIN_COMPANY, MAYA_PLUGIN_VERSION, MAYA_PLUGIN_REQUIREDAPI);

	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (renderer)
	{
		if (!viewRenderOverrideMRTInstance)
		{
			viewRenderOverrideMRTInstance = new viewRenderOverrideMRT("my_viewRenderOverrideMRT");
			renderer->registerOverride(viewRenderOverrideMRTInstance);
		}
	}

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

	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (renderer)
	{
		if (viewRenderOverrideMRTInstance)
		{
			renderer->deregisterOverride(viewRenderOverrideMRTInstance);
			delete viewRenderOverrideMRTInstance;
		}
		viewRenderOverrideMRTInstance = NULL;
	}

	status = plug.deregisterFileTranslator(MAYA_TRANSLATOR);
	if (status != MStatus::kSuccess)
	{
		status.perror("Failed to deregister Kaleido3D Translator properly.");
	}
	return status;
}
