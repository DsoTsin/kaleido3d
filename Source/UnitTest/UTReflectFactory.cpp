#include "Kaleido3D.h"
#include <Core/ReflectFactory.h>
#include <Config/OSHeaders.h>

#include <Core/Mesh.h>
#include <Core/ReflectFactory.h>

using namespace k3d;

Mesh* newMesh() {
	return new Mesh;
}


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

	ReflectFactory::Get().Register<Mesh>("Mesh", &Mesh::Reflect, new Mesh);
	ReflectFactory::Get().Register<Mesh, &newMesh>("FnMesh");

	ReflectFactory::Get().GetClass("FnMesh");

}