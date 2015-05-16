#include "Kaleido3D.h"
#include "ThreadLauncher.h"
#include "Thread.h"
#include "Windows/ThreadImpl.h"

namespace Concurrency {
	ThreadLauncherBase::ThreadLauncherBase()
	{
	}
	ThreadLauncherBase::~ThreadLauncherBase()
	{
	}
	void ThreadLauncherBase::Launch(Thread * thr)
	{
	}
	
	void ThreadLauncherBase::Release()
	{
	}
}