#include "Kaleido3D.h"
#include "CameraData.h"

namespace k3d
{
	CameraData::CameraData()
	{
	}

	CameraData::~CameraData()
	{
	}

	void CameraData::SetName(const char * name)
	{
		strncpy(m_Name, name, 64);
	}

	Archive& operator << (class Archive & arch, const CameraData & camera)
	{
		arch.ArrayIn(CameraData::ClassName(), 64);
		arch.ArrayIn(camera.m_Name, 64);

		arch << camera.m_FOV;
		arch << camera.m_NearPlane;
		arch << camera.m_FarPlane;

		arch << camera.m_FocalLength;
		arch << camera.m_Position;

		arch << camera.m_ViewVector[0] << camera.m_ViewVector[1] << camera.m_ViewVector[2];
		arch << camera.m_UpVector[0] << camera.m_UpVector[1] << camera.m_UpVector[2];
		arch << camera.m_RightVector[0] << camera.m_RightVector[1] << camera.m_RightVector[2];
		return arch;
	}


	Archive& operator >> (class Archive & arch, CameraData & camera)
	{
		arch.ArrayOut(camera.m_Name, 64);

		arch >> camera.m_FOV;
		arch >> camera.m_NearPlane;
		arch >> camera.m_FarPlane;

		arch >> camera.m_FocalLength;
		arch >> camera.m_Position;

		arch >> camera.m_ViewVector[0] >> camera.m_ViewVector[1] >> camera.m_ViewVector[2];
		arch >> camera.m_UpVector[0] >> camera.m_UpVector[1] >> camera.m_UpVector[2];
		arch >> camera.m_RightVector[0] >> camera.m_RightVector[1] >> camera.m_RightVector[2];
		return arch;
	}
}