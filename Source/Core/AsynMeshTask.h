#pragma once 

#include "TaskManager.h"
#include "Mesh.h"
#include <string>


namespace k3d {
	
	class AsynMeshTask : public IBaseThread {
	public:

		explicit AsynMeshTask(const char *meshPackName, TaskPriority priority = TaskPriority::Normal);
		explicit AsynMeshTask(std::string meshPackName, TaskPriority priority = TaskPriority::Normal);

		~AsynMeshTask() override;

		void OnRun() override;

		void OnFinish() override;

	protected:

		typedef std::vector<SpMesh>	SpVecMesh;
		SpVecMesh		m_MeshPtrList;

	private:

		TaskPriority	m_Priority;
		std::string		m_MeshPackName;

	};

}