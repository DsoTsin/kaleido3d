#include "CommonGL.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif
#include <stdio.h>
#include <assert.h>
#include <Config/Config.h>
#include <GL/glew.h>
// Nvidia spec CommandList support
#include "Nvidia/NVCommandList.h"

#include <Core/LogUtil.h>

//Global Variables
static bool nv_command_list_feature = false;


static NVPROC getGLFunction(const char *name) {
#if K3DPLATFORM_OS_WIN
	return (NVPROC)wglGetProcAddress(name);
#else
	return nullptr;
#endif
}

void GLInitializer::InitAndCheck() {
	int code = glewInit();
	assert(code==GLEW_OK);

    const char *glVersion = (const char*) glGetString(GL_VERSION);
    const char *glRenderer = (const char*) glGetString(GL_RENDERER);

	k3d::kDebug("GL Info:\n\t%s\n\t%s\nogl_LoadFunctions=%d\n", glVersion, glRenderer, code);

	int nv_support_code = init_NV_command_list(getGLFunction);
	nv_command_list_feature = (nv_support_code != 0);
	k3d::kDebug("Nvidia Command List Feature: %d\n", nv_support_code);
}

bool GLInitializer::NVCommandListSupported() {
	return nv_command_list_feature;
}