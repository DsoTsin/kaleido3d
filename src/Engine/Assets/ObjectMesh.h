#pragma once
#include <KTL/SharedPtr.hpp>
#include <KTL/DynArray.hpp>
namespace k3d 
{
	struct ObjectLoadListener {
		virtual void OnLoad() = 0;
	};

	class ObjectMesh : public ObjectLoadListener {
	public:
		ObjectMesh();

		typedef SharedPtr<class Mesh>			PtrMesh;
		typedef DynArray<PtrMesh>                PtrMeshArray;
		typedef SharedPtr<ObjectMesh>				PtrModel;
		
		static PtrModel Load(const char * data, size_t len);
		static PtrModel Load(const char * name);

		void OnLoad() override;
	private:
		K3D_DISCOPY(ObjectMesh)
	private:

		PtrMeshArray  m_MeshInstances;
	};

}