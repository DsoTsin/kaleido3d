#pragma once
#include <Core/Kaleido3D.h>
#include <Core/Interface/IRHI.h>

#define NS_GLRHI_BEGIN K3D_COMMON_NS { namespace opengl {
#define NS_GLRHI_END }}

#ifdef USE_OPENGL_ES

#elif USE_DESKTOP_OPENGL

#endif