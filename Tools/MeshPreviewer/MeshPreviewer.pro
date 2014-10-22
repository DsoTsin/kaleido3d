#-------------------------------------------------
#
# Project created by QtCreator 2014-03-30T20:27:50
#
#-------------------------------------------------

TARGET = MeshPreviewer
TEMPLATE = app
QT += core gui

include(d:/lib/glfw_x64.pri)
include(d:/lib/physx_x64.pri)

PRECOMPILED_HEADER = ../../Include/Config/Prerequisities.h

DEFINES += ENABLE_SSE
INCLUDEPATH += ../../Include ../../Source

CONFIG += c++11
SOURCES += kmeshpreviewer.cpp \
    ../../Source/Renderer/OpenGL/GLEW/glew.c \
    ../../Source/Renderer/OpenGL/GLFX/glfx.cpp \
    ../../Source/Renderer/OpenGL/GLFX/glfxLALRParser.cpp \
    ../../Source/Renderer/OpenGL/GLFX/glfxScanner.cpp \
    ../../Source/Renderer/OpenGL/kGLTechnique.cpp \
    ../../Source/Renderer/k3dTechnique.cpp \
    ../../Source/Core/k3dDbg.cpp \
    ../../Source/Core/k3dFile.cpp \
    ../../Source/Core/k3dLog.cpp \
    ../../Source/Core/k3dMesh.cpp \
    ../../Source/Core/JsonCpp/json_reader.cpp \
    ../../Source/Core/JsonCpp/json_value.cpp \
    ../../Source/Core/JsonCpp/json_writer.cpp \
    ../../Source/Core/DDSHelper/DDSHelper.cpp \
    ../../Source/Core/k3dImage.cpp \
    ../../Source/Renderer/OpenGL/kGLTexture.cpp \
    ../../Source/Core/k3dMaterial.cpp \
    ../../Source/Core/k3dJsonObject.cpp \
    ../../Source/Renderer/OpenGL/kGLShader.cpp \
    ../../Source/Physics/kPsEngine.cpp \
    ../../Source/Entity/k3dVehicle.cpp \
    ../../Source/Physics/k3dVehicleManager.cpp \
     ../../Source/Physics/kVehicleRaycast.cpp \
    ../../Source/Physics/kVehicleSceneQuery.cpp

HEADERS += \
    ../../Source/Physics/kPsEngine.h \
    ../../Source/Entity/k3dVehicle.h \
    ../../Source/Physics/k3dVehicleManager.h \
     ../../Source/Physics/kVehicleRaycast.h \
    ../../Source/Physics/kVehicleSceneQuery.h

