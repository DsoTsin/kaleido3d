#include "AsynMeshTask.h"
#include "AsynMeshTask.h"

#include "Archive.h"
#include "Mesh.h"
#include "File.h"
#include "LogUtil.h"
#include "AssetManager.h"

namespace k3d {

	AsynMeshTask::AsynMeshTask(const char * meshPackName, TaskPriority priority)
	{
		m_MeshPackName	= meshPackName ;
		m_Priority		= priority;
	}

	AsynMeshTask::AsynMeshTask(std::string meshPackName, TaskPriority priority)
	{
		m_MeshPackName = meshPackName;
		m_Priority = priority;
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
			MeshHeader header;
			arch >> header;
			kDebug("AsynMeshTask::dcc package version %d\n", header.Version);
			char ClassName[64] = { 0 };
			arch.ArrayOut( ClassName, 64 );
			kDebug("AsynMeshTask::dcc class name = %s\n", ClassName);
			std::string className(ClassName);
			while (className != "End")
			{
				if (className == "Mesh") 
				{
					SpMesh mesh(new Mesh);
					arch >> *mesh;
					AssetManager::Get().AppendMesh(mesh);
				}
				//! read Next Class Name
				arch.ArrayOut(ClassName, 64);
				className = { ClassName };
			}
		}
		else {
			kDebug("AsynMeshTask::Cann't find file (%s).\n", m_MeshPackName.c_str());
		}
		mem.Close();

		this->OnFinish();
	}
	
	void AsynMeshTask::OnFinish()
	{
	}
}