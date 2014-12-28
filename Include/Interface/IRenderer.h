#pragma once

struct IRenderMesh;

struct IRenderer {

	virtual ~IRenderer() {}

	virtual void PrepareFrame() = 0;

	virtual void DrawOneFrame() = 0;

	virtual void EndOneFrame() = 0;

	virtual void DrawMesh(IRenderMesh *) = 0;

	virtual void OnResize(int width, int height) = 0;
};