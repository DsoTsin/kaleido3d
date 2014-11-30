#include "Kaleido3D.h"
#include "AsynMeshTask.h"

#include "Archive.h"
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
		AssetManager::SpIODevice meshPackage = AssetManager::OpenAsset(m_MeshPackName.c_str());
		if (meshPackage) {
			Archive arch;
			arch.SetIODevice(meshPackage.get());
			MeshHeader header;
			arch >> header;
			Debug::Out("AsynMeshTask","dcc package version %d", header.Version);
			char ClassName[64] = { 0 };
			arch.ArrayOut( ClassName, 64 );
			Debug::Out("AsynMeshTask","dcc class name = %s", ClassName);
			std::string className(ClassName);
			while (className != "End")
			{
				if (className == "Mesh") 
				{
					SpMesh mesh(new Mesh);
					arch >> *mesh;
					AssetManager::Get().AppendMesh(mesh);
					m_MeshPtrList.push_back(mesh);
				}
				//! read Next Class Name
				arch.ArrayOut(ClassName, 64);
				className = { ClassName };
			}
		}
		else {
			Debug::Out("AsynMeshTask","Cann't find file (%s).", m_MeshPackName.c_str());
		}

		this->OnFinish();
	}
	
	void AsynMeshTask::OnFinish()
	{
	}
}