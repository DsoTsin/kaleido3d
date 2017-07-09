#include "PluginMain.h"
#include <Core/LogUtil.h>
#include <maya/MFnPlugin.h>

using namespace k3d;

// specifies a script for the Export Options UI
//char* g_OptionScript = "K3DTranslator";

// the default option string
//char* g_DefaultExportOptions = "-option1 1 -option2 1";

MStatus initializePlugin( MObject obj )
{
	MStatus   status = MStatus::kFailure;
  MFnPlugin plug(obj, MAYA_PLUGIN_COMPANY, MAYA_PLUGIN_VERSION, MAYA_PLUGIN_REQUIREDAPI);
	status = plug.registerFileTranslator(MAYA_TRANSLATOR, "none", DCTranslator::creator/*,(char*)g_OptionScript,(char*)g_DefaultExportOptions*/);
	MS_CHECK(status, "Failed to register Kaleido3D Translator properly.");
  status = plug.registerNode(HAIR_NODE, HairNode::id, HairNode::creator, HairNode::initialize);
  MS_CHECK(status, "Failed to register HairNode properly.");

#if Qt5_FOUND
  plug.registerCommand(HelixQtCmd::commandName, HelixQtCmd::creator);
  MS_CHECK(status, "Failed to register QUI properly.");
#endif
	return status;
}
/////////////////////////////////////////////////////////////

MStatus uninitializePlugin( MObject obj )
{
  MFnPlugin plug( obj ); 
	MStatus   status = MStatus::kSuccess;
	status = plug.deregisterFileTranslator(MAYA_TRANSLATOR);
  MS_CHECK(status, "Failed to deregister Kaleido3D Translator properly.");
  status = plug.deregisterNode(HairNode::id);
  MS_CHECK(status, "Failed to deregister Kaleido3D HairNode properly.");

#if Qt5_FOUND
  HelixQtCmd::cleanup();
  status = plug.deregisterCommand(HelixQtCmd::commandName);
  MS_CHECK(status, "Failed to deregister Kaleido3D QUI properly.");
#endif
  return status;
}
