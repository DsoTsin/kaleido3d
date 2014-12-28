#pragma once

struct IRenderMesh {
	virtual ~IRenderMesh() {}

	virtual void Render() = 0;
};