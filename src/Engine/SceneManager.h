#pragma once
#include <KTL/Singleton.hpp>
#include <KTL/HashMap.hpp>
#include <KTL/SharedPtr.hpp>
#include <KTL/DynArray.hpp>
#include "SceneObject.h"

namespace k3d 
{
	class BaseCamera;
	//! \class	SceneManager
	//! \brief	Manager for 3D scene.
	class SceneManager : public Singleton<SceneManager>
	{
	public:

		/// \brief VMarkMap
		/// visible marks map
		typedef HashMap<uint32, SharedPtr<DynArray<char> > > VMarkMap;

		SceneManager();
		~SceneManager();

		void InitScene();
		void AddSceneObject(SObject::SObjPtr objptr, const kMath::Vec3f &position = kMath::Vec3f(0.0f, 0.0f, 0.0f));
		void FrustumCull(Camera *);
		void UpdateScene();

		bool LoadFromJSON(const char *scene_file);
		void SaveToJSON(const char* scene_file);

	protected:

		BaseCamera*        m_CameraPtr;
		SObject::SOVector  m_SceneVisibleObjs;
		SObject::SOVector  m_SceneObjs;
		VMarkMap           m_VisibleMarkMap;

	};

	typedef SharedPtr<SceneManager> SharedSceneManagerPtr;
}