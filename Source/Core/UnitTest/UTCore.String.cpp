#include "Common.h"
#include <Core/Utils/MD5.h>

#if K3DPLATFORM_OS_WIN
#pragma comment(linker,"/subsystem:console")
#endif

using namespace k3d;
using namespace std;

String toBeRemoved("Love you later.");

void TestString()
{
    String testString("HeyYou!");
    cout << testString.CStr() << endl;
    String testStringBaby("BabyGirl!!");
    testStringBaby.Swap(testString);
    cout << testString.CStr() << endl;
        
    testString += toBeRemoved;
    testString += "..";
    cout << testString.CStr() << endl;
        
    testString.AppendSprintf("%d %s", 5, "fhdsfjdhjkfdhksfhdkjshfjkdshfk");
	testString += 'B';
    cout << testString.CStr() << endl;
        
	MD5 md5;
	md5.Update(testString);
	auto testMd5 = md5.Str();
	cout << "md5:" << testMd5.CStr() << endl;

	cout << "md5:" << MD5Encode(testString).CStr() << endl;

    //Archive ar;
    //ar << testString;
        
    String testMoveString(Move(toBeRemoved));
    cout << "testMove:" << testMoveString.CStr()
    << " original:" << (toBeRemoved.CStr()?toBeRemoved.CStr():"null") << endl;

	auto b64 = Base64Encode(testMoveString);
	cout << "b64:" << b64.CStr() << endl;

	auto d64 = Base64Decode(b64);
	cout << "d64:" << d64.CStr() << endl;

}

int main(int argc, char**argv)
{
	TestString();
	return 0;
}