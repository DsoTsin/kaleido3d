#include "AsynMeshTask.h"

#include "Archive.h"
#include "Mesh.h"
#include "File.h"

#include <Engine/AssetManager.h>

namespace k3d {

	AsynMeshTask::AsynMeshTask(const char * meshPackName, TaskPriority priority)
	{
		m_MeshPackName	= meshPackName ;
		m_Priority		= priority;
	}

	AsynMeshTask::~AsynMeshTask()
	{
	}
	
	void AsynMeshTask::OnRun()
	{
		MemMapFile mem;
		if (mem.Open(m_MeshPackName.c_str(), IORead)) {
			Archive arch;
			arch.SetIODevice(&mem);
			
		}
		mem.Close();

		this->OnFinish();
	}
	
	void AsynMeshTask::OnFinish()
	{
	}
}