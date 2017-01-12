#include "Common.h"

#if K3DPLATFORM_OS_WIN
#pragma comment(linker,"/subsystem:console")
#endif

int main(int argc, char**argv)
{
	JOIN_TESTS(TestBundle(), TestOs(), TestShaderCompiler(), TestWebSocket(), TestSharedPtr(), TestString(), TestDynArrray());
	return 0;
}
