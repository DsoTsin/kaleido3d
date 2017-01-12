#include "Common.h"
#include <Core/CameraData.h>
#include <Core/String.h>
#include <iostream>

using namespace std;
using namespace k3d;

class SharedTest
{
public:
	SharedTest()
	{

	}
	~SharedTest()
	{
		cout << "SharedTest Destroy. " << endl;
	}
};

threadptr TestSharedPtr()
{
	auto ret = std::make_shared<std::thread>([]() {
		SharedPtr<Os::File> spFile(new Os::File);
		cout << "file:" << spFile.UseCount() << endl;

		SharedPtr<CameraData> spCam(new CameraData);
		spCam.UseCount();
		spCam->SetName("TestSharedPtr");

		SharedPtr<SharedTest> spTest(new SharedTest);
		cout << "SharedTest:" << spTest.UseCount() << endl;

		SharedPtr<IIODevice> ioFile = spFile;
		K3D_ASSERT(ioFile.UseCount()==2);
		
		auto makeShared = MakeShared<Os::File>();
		makeShared->Open(KT("TestSharedPtr"), IOWrite);

		auto refMakeShared = makeShared;
		cout << "refMakeShared:" << refMakeShared.UseCount() << endl;

		refMakeShared->Close();
	});
	return ret;
}
