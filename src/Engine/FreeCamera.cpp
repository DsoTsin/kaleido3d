#include "Kaleido3D.h"
#include "FreeCamera.h"

namespace k3d
{

	FreeCamera::FreeCamera()
	{
		m_Translation = kMath::Vec3f(0.f);
		m_CameraSpeed = 0.5f;
	}

	FreeCamera::~FreeCamera()
	{

	}

	//const int VK_W = 0x57;
	//const int VK_S = 0x53;
	//const int VK_A = 0x41;
	//const int VK_D = 0x44;
	//const int VK_Q = 0x51;
	//const int VK_Z = 0x5a;
	//const float EPSILON = 0.001f;
	//const float EPSILON2 = EPSILON*EPSILON;
	//float dt = 0.2f;
	void FreeCamera::Update()
	{
		kMath::Mat4f R = kMath::Quaternion<float>::YawPitchRoll(m_Yaw, m_Pitch, m_Roll);
		m_CameraPosition += m_Translation;

		m_LookVector = (R * kMath::Vec4f(0, 0, 1, 0)).ToVec3();
		m_UpVector = (R * kMath::Vec4f(0, 1, 0, 0)).ToVec3();
		m_RightVector = kMath::CrossProduct(m_LookVector, m_UpVector);

		kMath::Vec3f target = m_CameraPosition + m_LookVector;
		m_ViewMatrix = kMath::LookAt(m_CameraPosition, target, m_UpVector);

		//  if( GetAsyncKeyState(VK_W) & 0x8000) {
		//    Walk(dt);
		//  }

		//  if( GetAsyncKeyState(VK_S) & 0x8000) {
		//    Walk(-dt);
		//  }

		//  if( GetAsyncKeyState(VK_A) & 0x8000) {
		//    Strafe(-dt);
		//  }

		//  if( GetAsyncKeyState(VK_D) & 0x8000) {
		//    Strafe(dt);
		//  }

		//  if( GetAsyncKeyState(VK_Q) & 0x8000) {
		//    Lift(dt);
		//  }

		//  if( GetAsyncKeyState(VK_Z) & 0x8000) {
		//    Lift(-dt);
		//  }

		//  kMath::Vec3f t = GetTranslation();
		//  if(DotProduct(t,t)>EPSILON2) {
		//    SetTranslation(t*0.95f);
		//  }
	}

	void FreeCamera::Walk(const float dt)
	{
		m_Translation += (m_LookVector * m_CameraSpeed * dt);
		Update();
	}

	void FreeCamera::Strafe(const float dt)
	{
		m_Translation += (m_RightVector*m_CameraSpeed*dt);
		Update();
	}

	void FreeCamera::Lift(const float dt)
	{
		m_Translation += (m_UpVector*m_CameraSpeed*dt);
		Update();
	}

	void FreeCamera::SetTranslation(const kMath::Vec3f &t)
	{
		m_Translation = t;
		Update();
	}

	kMath::Vec3f FreeCamera::GetTranslation() const
	{
		return m_Translation;
	}

	void FreeCamera::SetSpeed(const float speed)
	{
		m_CameraSpeed = speed;
	}

	const float FreeCamera::GetSpeed() const
	{
		return m_CameraSpeed;
	}
}