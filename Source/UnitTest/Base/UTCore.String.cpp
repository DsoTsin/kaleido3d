#include "Common.h"

using namespace k3d;
using namespace std;

String toBeRemoved("Love you later.");

threadptr TestString()
{
    auto ret = std::make_shared<std::thread>([]() {
        String testString("HeyYou!");
        cout << testString.CStr() << endl;
        String testStringBaby("BabyGirl!!");
        testStringBaby.Swap(testString);
        cout << testString.CStr() << endl;
        
        testString += toBeRemoved;
        testString += "..";
        cout << testString.CStr() << endl;
        
        testString.AppendSprintf("%d %s", 5, "fhdsfjdhjkfdhksfhdkjshfjkdshfk");
        cout << testString.CStr() << endl;
        
        //Archive ar;
        //ar << testString;
        
        String testMoveString(Move(toBeRemoved));
        cout << "testMove:" << testMoveString.CStr()
        << " original:" << (toBeRemoved.CStr()?toBeRemoved.CStr():"null") << endl;
    });
    return ret;
}
