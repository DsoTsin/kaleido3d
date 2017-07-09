#pragma once

struct IMesh;

struct IRenderMesh 
{
	virtual ~IRenderMesh() {}

	virtual void Render(IMesh *) = 0;
};