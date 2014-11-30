#pragma once 

namespace Concurrency {
	enum class ThreadPriority {
		Low,
		Normal,
		High,
		RealTime
	};


	enum class ThreadStatus {
		Ready,
		Running,
		Finish
	};
}