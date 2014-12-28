TEMPLATE=lib
CONFIG += qt warn_off plugin
TARGET_EXT = .mll

include(d:/lib/physx_x64.pri)

MAYA_DIR = C:/Program Files/Autodesk/Maya2014

INCLUDEPATH+="$$MAYA_DIR/include"\
  ../../Source\
	../../Include

PRECOMPILED_HEADER = ../../Include/Config/Prerequisities.h

MAYALIB = $$MAYA_DIR/lib

DEFINES+=NDEBUG _WINDOWS NT_PLUGIN REQUIRE_IOSTREAM

LIBS+= -l"$$MAYALIB/OpenMaya" -l"$$MAYALIB/Foundation" -l"$$MAYALIB/OpenMayaUI" -l"$$MAYALIB/OpenMayaRender"

QMAKE_LFLAGS	= /export:initializePlugin /export:uninitializePlugin /SUBSYSTEM:WINDOWS

HEADERS += MayaToolKit.h StaticMeshTranslator.h \
    ../../Source/Core/k3dMesh.h \
    ../../Source/Core/k3dArchive.h \
    ../../Source/Physics/kPsEngine.h

SOURCES += MayaToolKit.cpp StaticMeshTranslator.cpp \
    ../../Source/Core/k3dFile.cpp \
    ../../Source/Core/JsonCpp/json_reader.cpp \
    ../../Source/Core/JsonCpp/json_value.cpp \
    ../../Source/Core/JsonCpp/json_writer.cpp \
    ../../Source/Core/k3dJsonObject.cpp \
    ../../Source/Core/k3dMesh.cpp \
    ../../Source/Core/k3dArchive.cpp \
    ../../Source/Core/k3dDbg.cpp \
    ../../Source/Physics/kPsEngine.cpp \
    ../../Source/Core/k3dLog.cpp


install.path = $$[MAYA_DIR]/bin/plug-ins
install.files = $$OUT_PWD/$$TARGET$$TARGET_EXT
INSTALLS += install
