#ifndef __k3dk3dAssetManager_h__
#define __k3dk3dAssetManager_h__
#pragma once

#include <KTL/ThreadPool.hpp>
#include <KTL/Semaphore.hpp>
#include <KTL/Singleton.hpp>
#include <Interface/IIODevice.h>
#include <Core/Mesh.h>
#include <Core/AsynMeshTask.h>
#include <memory>


namespace k3d {
	class Image;
	class Shader;

	struct BytesPackage
	{
		std::vector<kByte> Bytes;
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
			AsynMeshTask * task
			);
		
		void AppendMesh(SpMesh meshPtr);

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

		using string = std::string;

		typedef std::unordered_map<string, SpMesh> MapMesh;
		typedef MapMesh::iterator MapMeshIter;

		typedef std::unordered_map<string, std::shared_ptr<Image> > MapImage;
		typedef MapImage::iterator MapImageIter;

		//  typedef std::unordered_map<k3dString, std::shared_ptr<k3dShader> > MapShader;
		//  typedef MapShader::iterator MeshMapIter;

		AssetManager();

	public:
		typedef std::shared_ptr<IIODevice> SpIODevice;

		static SpIODevice		OpenAsset(const char * assetPath, IOFlag openFlag = IORead, bool fast = false);

		static std::string		AssetPath(const char * assetRelativePath);


	protected:
		static	std::string		s_envAssetPath;

		std::vector<string>		m_SearchPaths;
		std::thread_pool*       m_pThreadPool;

		MapMesh                 m_MeshMap;
		MapImage                m_ImageMap;

		mutable bool            m_IsLoading;
		mutable bool            m_HasPendingObject;

		std::atomic<int>        m_NumPendingObject;
	};
}

#endif
