#ifndef __Common_h__
#define __Common_h__

#define NS_K3D_METAL_BEGIN namespace k3d { namespace metal {
#define NS_K3D_METAL_END }}

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <Core/LogUtil.h>

#define MTLLOG(Level, ...) \
::k3d::Log(::k3d::ELogLevel::Level, "kaleido3d::MetalRHI", __VA_ARGS__);

#define MTLLOGI(...) MTLLOG(Info, __VA_ARGS__);
#define MTLLOGE(...) MTLLOG(Error, __VA_ARGS__);

#endif
