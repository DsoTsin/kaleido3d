#include "Kaleido3D.h"
#include "ThreadImpl.h"

#include <strsafe.h>

namespace Concurrency {
	namespace ThreadImpl
	{
		void sleep(uint32 milliSeconds) {
			::Sleep(milliSeconds);
		}
		
		void getSysInfo()
		{
			SYSTEM_INFO info;
			::GetSystemInfo(&info);

			TCHAR msgBuf[1024] = { 0 };
			::StringCchPrintf(msgBuf, 1024, TEXT("Avaliable CPU Core Num= %d\n"), info.dwNumberOfProcessors);
			::OutputDebugString(msgBuf);
		}
	}

}