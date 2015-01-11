#pragma once 

#include "TaskManager.h"
#include <string>

namespace k3d {
	
	class Mesh;

	class AsynMeshTask : public IBaseThread {
	public:

		explicit AsynMeshTask(const char *meshPackName, TaskPriority priority = TaskPriority::Normal);
		explicit AsynMeshTask(std::string meshPackName, TaskPriority priority = TaskPriority::Normal);

		~AsynMeshTask() override;

		void OnRun() override;

		void OnFinish() override;

	private:

		TaskPriority	m_Priority;
		std::string		m_MeshPackName;

	};

}