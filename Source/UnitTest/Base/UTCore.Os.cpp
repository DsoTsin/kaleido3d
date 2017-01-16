#include "Common.h"
#include <iostream>
#include <sstream>

using namespace std;
using namespace k3d;

threadptr TestOs()
{
	auto ret = std::make_shared<std::thread>([]() {
		K3D_ASSERT(Os::MakeDir(KT("./TestOs")));
		K3D_ASSERT(Os::Remove(KT("./TestOs")));
		while (1)
		{
			Os::Sleep(1000);
			int count = Os::GetCpuCoreNum();
			float * usage = Os::GetCpuUsage();
			stringstream str;
			str << "usage: ";
			for (int i = 0; i < count; i++)
			{
				str << usage[i] << ",";
			}
			cout << str.str() << endl;
		}
	});
	return ret;
}
