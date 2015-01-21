#include "Kaleido3D.h"
#include <Core/TaskManager.h>
#include <Config/OSHeaders.h>
#include <Core/LogUtil.h>
#include <Core/ConditionVariable.h>
//#include <KTL/LockFreeQueue.hpp>

/*
A------B--
		|
Game--- |
		|
Main-----
*/


using namespace k3d;

SpConditionVariable gEvent;
SpConditionVariable gAEvent;
SpConditionVariable gBEvent;

class NamedTask : public IBaseThread {
public:
	NamedTask(const char * outputStr) {
		output = {outputStr};
	}

	void OnRun() {
		int i = 0;
		while (true) {
			i++;
			::Sleep(1000);
			Debug()<<output.c_str();
			if (i == 10) {
				Debug::Out("NamedTask","Game Thread Finished..");
				break;
			}
		}
	}

private:
	std::string output;
};


class ATask : public IBaseThread {
public:
	ATask() {
	}

	void OnRun() {
		int i = 0;
		gAEvent = SpConditionVariable(new ConditionVariable);
		while (true) {
			i++;
			::Sleep(1000);
			if (i == 5) {
				Debug::Out("ATask","What does the fox say ?\n");
				gEvent->Signal();
			}
			if (i == 11) {
				gAEvent->Signal();
				Debug::Out("ATask","MainThread Wake up!!\n");
			}
		}
	}

};

class BTask : public IBaseThread {
public:
	BTask() {
	}

	void OnRun() {
		ConditionVariable::WaitFor(gEvent, 80000);
		Debug::Out("BTask","\tFuck you!!!");
		while (true) {
			::Sleep(1000);
			Debug::Out("BTask","\tFox say Nothing");
		}
	}

};

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	TaskManager::Get();
	TaskManager::Get().Init();

	gEvent = SpConditionVariable(new ConditionVariable);

	NamedTask *gameTask = new NamedTask("GameTask::In Game Thread...\n");
	TaskManager::Get().Post(gameTask);
		
	ATask *aTask = new ATask;
	BTask *bTask = new BTask;

	TaskManager::Get().Post(aTask);
	TaskManager::Get().Post(bTask);

	WThread::Wait(gameTask);

	Debug::Out("MainThread", "BThread Wake Up!!");
	gEvent->Signal();

	ConditionVariable::WaitFor(gAEvent);
	Debug::Out("MainThread","Release gEvent");
	gEvent->Release();
	gameTask->Join();

	TaskManager::Get().ShutDown();
	return 0;
}