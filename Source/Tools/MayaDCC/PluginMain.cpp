#include "PluginMain.h"
#include <Core/LogUtil.h>
#include <maya/MFnPlugin.h>

// specifies a script for the Export Options UI
//char* g_OptionScript = "K3DTranslator";

// the default option string
//char* g_DefaultExportOptions = "-option1 1 -option2 1";

MStatus initializePlugin( MObject obj )
{
	MStatus   status = MStatus::kFailure;
    MFnPlugin plug(obj, MAYA_PLUGIN_COMPANY, MAYA_PLUGIN_VERSION, MAYA_PLUGIN_REQUIREDAPI);
	status = plug.registerFileTranslator(
		MAYA_TRANSLATOR,
        "none",
        DCTranslator::creator
//		,(char*)g_OptionScript,
//		(char*)g_DefaultExportOptions
    );
	if (status != MStatus::kSuccess)
	{
		status.perror("Failed to register Kaleido3D Translator properly.");
	}
	return status;
}
/////////////////////////////////////////////////////////////

MStatus uninitializePlugin( MObject obj )
{
    MFnPlugin plug( obj ); 
	MStatus   status = MStatus::kSuccess;
	status = plug.deregisterFileTranslator(MAYA_TRANSLATOR);
	if (status != MStatus::kSuccess)
	{
		status.perror("Failed to deregister Kaleido3D Translator properly.");
	}
	return status;
}
