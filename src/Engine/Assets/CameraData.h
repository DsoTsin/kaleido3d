#pragma once

#include "Bundle.h"
#include <Math/kMath.hpp>
#include <Math/kGeometry.hpp>
#include <KTL/Archive.hpp>
#include <KTL/SharedPtr.hpp>

namespace k3d
{
	enum class ECamVersion : uint64
	{
		VERSION_1_0 = 201612u,
	};

	/**
	*  Camera Header <<POD>>
	*/
	struct CameraHeader {
		ECamVersion     Version;
	};

	class K3D_CORE_API CameraData
	{
	public:
		CameraData();
		~CameraData();

		const char*		Name() const { return m_Name; }
		void SetName(const char* name);

		KOBJECT_CLASSNAME(CameraData)

		friend K3D_CORE_API class Archive& operator << (class Archive & arch, const CameraData & mesh);
		friend K3D_CORE_API class Archive& operator >> (class Archive & arch, CameraData & mesh);

		KOBJECT_PROPERTY_IMPL(Position, kMath::Vec3f);
		KOBJECT_PROPERTY_IMPL(UpVector, kMath::Vec3f);
		KOBJECT_PROPERTY_IMPL(RightVector, kMath::Vec3f);
		KOBJECT_PROPERTY_IMPL(ViewVector, kMath::Vec3f);
		KOBJECT_PROPERTY_IMPL(FOV, float);
		KOBJECT_PROPERTY_IMPL(FocalLength, float);
		KOBJECT_PROPERTY_IMPL(NearPlane, float);
		KOBJECT_PROPERTY_IMPL(FarPlane, float);

	private:
		char			m_Name[64];
		kMath::Vec3f	m_Position;
		kMath::Vec3f	m_UpVector;
		kMath::Vec3f	m_RightVector;
		kMath::Vec3f	m_ViewVector;
		float			m_FOV;
		float			m_FocalLength;
		float			m_NearPlane;
		float			m_FarPlane;
	};

	typedef SharedPtr<CameraData> SpCameraData;
}