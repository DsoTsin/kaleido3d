#pragma once

#include <Config/OSHeaders.h>
#include "../Thread.h"

namespace Concurrency 
{

	namespace ThreadImpl
	{
		void sleep(uint32 milliSeconds);
		void getSysInfo();
	}

}