#include "Kaleido3D.h"
#include "TaskWrapper.h"

#if K3DPLATFORM_OS_WIN
#include "Windows/ThreadImpl.h"
#elif K3DPLATFORM_OS_IOS
#include "iOS/ThreadImpl.h"
#endif
#include <functional>

namespace Concurrency {


}