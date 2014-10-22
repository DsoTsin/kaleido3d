#include "PluginMain.h
#include <Core/k3dLog.h>
#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
{
    MFnPlugin plug(obj, "Tsin Studio", "1.0", "Any");
    MStatus st = plug.registerFileTranslator(
                "kaleido3d digital content package",
                "none",
                DCTranslator::creator
                );
    MString k3dLogDir(getenv("KALEIDO3D_DIR"));

    k3dLog::InitLogFile((k3dLogDir+"/Data/MayaToolkitLog.html").asChar());

    return st;
}
//////////////////////////////////////////////////////////////

MStatus uninitializePlugin( MObject obj )
{
    MFnPlugin plug( obj );
    k3dLog::CloseLog();
    return plug.deregisterFileTranslator( "kaleido3d digital content package" );
}
