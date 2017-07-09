#include "Common.h"
#include <Core/CameraData.h>
#include <KTL/String.hpp>
#include <iostream>

#if K3DPLATFORM_OS_WIN
#pragma comment(linker,"/subsystem:console")
#endif

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

class ESFT;

class ESFTChild1
{
public:
	ESFTChild1(SharedPtr<ESFT> pERoot)
		: Root(pERoot)
	{
		cout << "ESFTChild1 init .." << endl;
#if K3DPLATFORM_OS_WIN
		OutputDebugStringA("ESFTChild1 init ..\n");
#endif
	}
	~ESFTChild1()
	{
        cout << "ESFTChild1 release .." << endl;
#if K3DPLATFORM_OS_WIN
		OutputDebugStringA("ESFTChild1 release ..\n");
#endif
	}
	SharedPtr<ESFT> Root;
};

class Root
{
public:
	static DynArray< SharedPtr< ESFT > > root;
	static DynArray< SharedPtr<ESFTChild1> > childrenTracker;
};

DynArray< SharedPtr< ESFT > > Root::root;
DynArray< SharedPtr<ESFTChild1> > Root::childrenTracker;

class ESFT : public EnableSharedFromThis<ESFT>
{
public:
	ESFT()
    {
#if K3DPLATFORM_OS_WIN
		OutputDebugStringA ("ESFT init ..\n");
#endif
	}
	~ESFT()
    {
#if K3DPLATFORM_OS_WIN
		OutputDebugStringA("ESFT release ..\n");
#endif
		cout << "ESFT release .." << endl;
	}
	SharedPtr<ESFTChild1> NewChild1()
	{
		auto child = MakeShared<ESFTChild1>(SharedFromThis());
		Root::childrenTracker.Append(child);
		return child;
	}
};

void TestSharedPtr()
{
	SharedPtr<Os::File> spFile(new Os::File);
	cout << "file:" << spFile.UseCount() << endl;

	SharedPtr<CameraData> spCam(new CameraData);
	spCam.UseCount();
	spCam->SetName("TestSharedPtr");

	SharedPtr<SharedTest> spTest(new SharedTest);
	cout << "SharedTest:" << spTest.UseCount() << endl;

	SharedPtr<IIODevice> ioFile = spFile;
	K3D_ASSERT(ioFile.UseCount()==2);
		
	{
		WeakPtr<IIODevice> weakRef(ioFile);
		cout << "weakREf:" << ioFile.UseCount() << endl;
	}

	auto makeShared = MakeShared<Os::File>();
	makeShared->Open(KT("TestSharedPtr"), IOWrite);

	auto esft = MakeShared<ESFT>();
	Root::root. Append (esft);
	{
		auto child = esft->NewChild1();
	}
	auto child2 = esft->NewChild1();
	auto refMakeShared = makeShared;
	cout << "refMakeShared:" << refMakeShared.UseCount() << endl;

	refMakeShared->Close();
}

int atexit(void)
{
#if K3DPLATFORM_OS_WIN
	OutputDebugStringA("At Exit ..\n");
#endif
	return 0;
}

int main(int argc, char**argv)
{
	TestSharedPtr();
	return 0;
}
