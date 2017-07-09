#pragma once

namespace k3d 
{
	struct ObjectLoadListener {
		virtual void OnLoad() = 0;
	};

	class ObjectMesh : public ObjectLoadListener {
	public:
		ObjectMesh();

		typedef std::shared_ptr<class Mesh>			PtrMesh;
		typedef std::vector<PtrMesh>                PtrMeshArray;
		typedef std::shared_ptr<ObjectMesh>				PtrModel;
		
		static PtrModel Load(const char * data, size_t len);
		static PtrModel Load(const char * name);

		void OnLoad() override;
	private:
		K3D_DISCOPY(ObjectMesh)
	private:

		PtrMeshArray  m_MeshInstances;
	};

}