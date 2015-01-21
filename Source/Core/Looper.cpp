#include "Kaleido3D.h"

#include "Looper.h"
#include <Config/OSHeaders.h>

namespace k3d {

	Looper::Looper() {
	}

	Looper::~Looper() {
	}

	void Looper::StartLooper() {
#if K3DPLATFORM_OS_WIN
		// Main message loop
		MSG msg = { 0 };
		while (WM_QUIT != msg.message) {
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				;
			}
		}
#endif
	}
}