#ifndef __k3dk3dAssetManager_h__
#define __k3dk3dAssetManager_h__
#pragma once

#include <Config/Prerequisities.h>
#include <KTL/threadpool.h>
#include <KTL/semaphore.h>
#include <KTL/Singleton.h>
#include <Core/AsynMeshTask.h>

namespace k3d {
	class Image;
	class Mesh;
	class Shader;

	struct BytesPackage
	{
		std::vector<kByte> Bytes;
	};

	/// Resource Item is
	/// \brief The ResourceItem class
	///
	class kResourceItem
	{
	public:
		enum ResType {
			MESH_DATA,
			TEX_DATA,
			MATER_DATA,
			SHADER_DATA,
			OTHER
		};

		kResourceItem(const char*name, ResType t)
			: m_ResType(t)
		{
			m_ResName = k3dString(name);
		}

		ResType   Type() const { return m_ResType; }
		k3dString ResName() { return m_ResName; }

	protected:

		ResType   m_ResType;
		k3dString m_ResName;

	};

	//Curiously Recurring Template Pattern, CRTP
	template <class T, typename RefObj = kCoreObject>
	class tResItem
		: public kResourceItem
		, public RefObj
	{
	public:
		tResItem(ResType type) : kResourceItem(nullptr, type) {}
	};

	/// AssetManager
	/// **Support Asynchronic Reader
	/// Search Path Supported
	/// fix: Need Archieve Support
	/// With A Pool-Allocator
	class AssetManager : public Singleton<AssetManager>
	{
	public:
		void Init();

		void Shutdown();

		/// \brief LoadAssetDescFile
		/// \param fileName
		void LoadAssetDescFile(const char *fileName);

		/// Add Search Path Support
		/// \brief AddSearchPath
		/// \param path
		void AddSearchPath(const char *path);


		void CommitAsynResourceTask(const char *fileName,
			BytesPackage & bp,
			std::semaphore & sp,
			std::function<void()> callback);

		void CommitAsynResourceTask(
			const char *fileName,
			BytesPackage & bp,
			std::semaphore & sp
			);

		void CommitAsynResourceTask(
			const char *fileName,
			BytesPackage &bp,
			std::atomic<bool> &finished);

		void CommitSynResourceTask(
			const char *fileName,
			BytesPackage & bp
			);

		void CommitAsynMeshTask(
			const AsynMeshTask & task
			);
		

		//  template <class T>
		//  void AsynLoadMesh(const char *meshName, void (T::*ptr)(), T*);

		  /// Free The Allocated Memory
		  /// \brief Free
		  /// \param byte_ptr
		static void Free(char *byte_ptr);

		/// \brief FindMesh
		/// \param meshName
		/// \return std::shared_ptr<Mesh>
		std::shared_ptr<Mesh> FindMesh(const char *meshName);

		/// \brief FindImage
		/// \param imgName
		/// \return std::shared_ptr<Image>
		std::shared_ptr<Image> FindImage(const char *imgName);

		typedef std::unordered_map<k3dString, std::shared_ptr<Mesh> > MapMesh;
		typedef MapMesh::iterator MapMeshIter;
		typedef std::unordered_map<k3dString, std::shared_ptr<Image> > MapImage;
		typedef MapImage::iterator MapImageIter;

		//  typedef std::unordered_map<k3dString, std::shared_ptr<k3dShader> > MapShader;
		//  typedef MapShader::iterator MeshMapIter;

		AssetManager();

	protected:

		std::vector<k3dString>  m_SearchPaths;
		std::thread_pool*       m_pThreadPool;

		MapMesh                 m_MeshMap;
		MapImage                m_ImageMap;

		mutable bool            m_IsLoading;
		mutable bool            m_HasPendingObject;

		std::atomic<int>        m_NumPendingObject;
	};
}

#endif
