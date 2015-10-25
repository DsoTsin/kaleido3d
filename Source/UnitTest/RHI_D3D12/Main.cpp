#include "UnitTestRHIDevice.h"
#include "UnitTestRHICommandContext.h"

#if		defined(__TESTRHIDEVICE__)
	K3D_APP_MAIN(UnitTestRHIDevice);
#elif	defined(__TESTRHICOMMANDCONTEXT__)
	K3D_APP_MAIN(UnitTestRHICommandContext);
#endif
