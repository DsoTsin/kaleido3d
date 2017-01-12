#include "Kaleido3D.h"
#include "CameraData.h"
#include "Archive.h"

namespace k3d
{
	CameraData::CameraData()
	{
	}

	CameraData::~CameraData()
	{
	}

	Archive& operator << (class Archive & arch, const CameraData & mesh)
	{
		
		return arch;
	}


	Archive& operator >> (class Archive & arch, CameraData & mesh)
	{
		return arch;
	}
}