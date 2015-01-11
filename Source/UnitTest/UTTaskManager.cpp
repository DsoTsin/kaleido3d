#include <Core/TaskManager.h>
#include <Config/OSHeaders.h>
#include <Core/LogUtil.h>
#include <Core/Event.h>
//#include <KTL/LockFreeQueue.hpp>

/*
A------B--
		|
Game--- |
		|
Main-----
*/


using namespace k3d;

Event::SpEvent gEvent;
Event::SpEvent gAEvent;
Event::SpEvent gBEvent;

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
			kDebug(output.c_str());
			if (i == 10) {
				kDebug("NamedTask::Game Thread Finished..\n");
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
		gAEvent = Event::SpEvent(new Event);
		while (true) {
			i++;
			::Sleep(1000);
			if (i == 5) {
				kDebug("ATask::What does the fox say ?\n");
				gEvent->Signal();
			}
			if (i == 11) {
				gAEvent->Signal();
				kDebug("ATask:: MainThread Wake up!!\n");
			}
		}
	}

};

class BTask : public IBaseThread {
public:
	BTask() {
	}

	void OnRun() {
		Event::WaitFor(gEvent, 80000);
		kDebug("BTask:: Fuck you!!!\n");
		while (true) {
			::Sleep(1000);
			kDebug("BTask:: Fox say Nothing\n");
		}
	}

};

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	TaskManager::Get();
	TaskManager::Get().Init();

	gEvent = Event::SpEvent(new Event);

	NamedTask *gameTask = new NamedTask("GameTask::In Game Thread...\n");
	TaskManager::Get().Post(gameTask);
		
	ATask *aTask = new ATask;
	BTask *bTask = new BTask;

	TaskManager::Get().Post(aTask);
	TaskManager::Get().Post(bTask);

	WThread::Wait(gameTask);

	kDebug("MainThread::BThread Wake Up!!\n");
	gEvent->Signal();

	Event::WaitFor(gAEvent);
	kDebug("MainThread::Release gEvent\n");
	gEvent->Release();
	gameTask->Join();

	TaskManager::Get().ShutDown();
	return 0;
}