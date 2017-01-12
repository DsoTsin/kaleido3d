#include "Common.h"

using namespace k3d;

threadptr TestOs()
{
	auto ret = std::make_shared<std::thread>([]() {
		K3D_ASSERT(Os::MakeDir(KT("./TestOs")));
		K3D_ASSERT(Os::Remove(KT("./TestOs")));
	});
	return ret;
}
