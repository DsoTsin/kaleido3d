#pragma once

namespace k3d {

	class Model {
	public:
		Model();

		typedef std::shared_ptr<class k3dMesh>      MeshPtr;
		typedef std::vector<MeshPtr>                MeshPtrArray;
		typedef std::shared_ptr<class k3dMaterial>  MatPtr;
		typedef std::vector<MatPtr>                 MatPtrArray;

	private:
		K3DDISCOPY(Model)
	private:

		MeshPtrArray  m_MeshInstances;
		MatPtrArray   m_MeshMaterials;
	};

}