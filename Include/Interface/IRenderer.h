#pragma once

struct IRenderer {
	virtual ~IRenderer() {}
	virtual void SwapBuffers() = 0;
	
};