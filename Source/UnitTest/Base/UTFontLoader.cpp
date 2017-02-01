#include "Common.h"

using namespace k3d;
using namespace std;
using namespace render;

threadptr TestFontManager()
{
	return std::make_shared<std::thread>([]() {
		FontManager fm;
		fm.LoadLib("../../Data/Test/calibri.ttf");
		fm.ChangeFontSize(64);
		auto & quads = fm.AcquireText("FuckYou");
		quads.Count();
	});
}