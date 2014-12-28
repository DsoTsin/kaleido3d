#pragma once 

#include <Interface/ITask.h>
#include <string>

namespace k3d {
	
	class Mesh;

	class AsynMeshTask : public ITask {
	public:

		AsynMeshTask(const char *meshPackName, TaskPriority priority = TaskPriority::Normal);

		~AsynMeshTask() override;

		void Execute() override;

		void OnFinish() override;

	private:

		TaskPriority	m_Priority;
		std::string		m_MeshPackName;

	};

}