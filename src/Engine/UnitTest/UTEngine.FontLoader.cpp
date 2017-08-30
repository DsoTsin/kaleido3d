#include <Renderer/FontRenderer.h>

#if K3DPLATFORM_OS_WIN
#pragma comment(linker,"/subsystem:console")
#endif

using namespace k3d;
using namespace std;
using namespace render;

void TestFontManager()
{
	FontManager fm;
	fm.LoadLib("../../Data/Test/calibri.ttf");
	fm.ChangeFontSize(64);
	auto quads = fm.AcquireText("FuckYou");
	quads.Count();
}

int main(int argc, char**argv)
{
	TestFontManager();
	return 0;
}