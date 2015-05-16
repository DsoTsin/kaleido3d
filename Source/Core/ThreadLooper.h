#pragma once 
#include "Looper.h"
#include "Thread.h"

namespace Concurrency {
	class ThreadLooper : public k3d::Looper, public Thread {

	};
}