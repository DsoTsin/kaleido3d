#ifndef __k3dCamera_h__
#define __k3dCamera_h__
#pragma once
#include <Math/kMath.hpp>
#include <Math/kGeometry.hpp>
#include "Controller.h"

struct MouseEvent;

namespace kMath 
{
	typedef Quaternion<float> Quaternionf;
}

namespace k3d 
{
	enum BoundType {
		BO_YES,
		BO_NO,
		BO_PARTIAL
	};

	//! \class	Camera
	//! \brief	Abstract 3D Camera Interface.
	class BaseCamera
	{
	public:
		BaseCamera();
		virtual       ~BaseCamera();

		virtual void Update();
		virtual void Rotate(const float yaw, const float pitch, const float roll);

		void  SetProjection(float fovy, float ratio, float near_plane = 0.1f, float far_plane = 1000.f);
		const kMath::Mat4f GetViewMatrix() const;
		const kMath::Mat4f GetProjectionMatrix() const;

		void  SetPosition(const kMath::Vec3f& v);
		const kMath::Vec3f GetPosition() const;

		void  SetFOV(const float fov);
		const float GetFOV() const;
		const float GetAspectRatio() const;

		void CalcFrustumPlanes();
		bool IsPointInFrustum(const kMath::Vec3f& point);
		bool IsSphereInFrustum(const kMath::BoundingSphere& sphere);

		BoundType IntersectBox(const kMath::AABB& aabb);
		void GetFrustumPlanes(kMath::Vec4f planes[6]);

		static BaseCamera* Load(const char* cameraJson);

	private:
		K3D_DISCOPY(BaseCamera)

	protected:
		float m_Yaw, m_Pitch, m_Roll;
		float m_Fov, m_AspectRatio, m_Znear, m_Zfar;

		kMath::Vec3f    m_FarPts[4];
		kMath::Vec3f    m_NearPts[4];
		kMath::Plane    m_Planes[6];

		kMath::Quaternionf    m_Rot;
		kMath::Vec3f          m_UpVector;
		kMath::Vec3f          m_LookVector;
		kMath::Vec3f          m_RightVector;
		kMath::Vec3f          m_CameraPosition;
		mutable kMath::Mat4f  m_ProjectionMatrix;
		mutable kMath::Mat4f  m_ViewMatrix;
	};
}
#endif
