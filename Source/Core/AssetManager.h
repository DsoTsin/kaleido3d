#ifndef __AssetManager_h__
#define __AssetManager_h__
#pragma once

#include <KTL/Singleton.hpp>
#include <Interface/IIODevice.h>
#include <Core/Mesh.h>
#include <Core/AsynMeshTask.h>
#include <memory>


namespace k3d
{
	class	Image;
	class	Shader;
	struct	ObjectLoadListener;

	typedef std::shared_ptr< std::vector<kByte> > ByteArray;

	struct BytesPackage
	{
		std::vector<kByte> Bytes;
	};

	struct IAsset
	{
		virtual ~IAsset() {}
		virtual uint64	GetLength() = 0;
		virtual const void*	GetBuffer() = 0;
	};

	/// AssetManager
	/// **Support Asynchronic Reader
	/// Search Path Supported
	/// fix: Need Archieve Support
	/// With A Pool-Allocator
	class K3D_API AssetManager : public Singleton<AssetManager>
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
		void AddSearchPath(const kchar *path);

		//auto AsyncLoadObject(const kchar * objPath, ObjectLoadListener* listener);

		/*void CommitAsynResourceTask(const kchar *fileName,
			BytesPackage & bp,
			std::semaphore & sp,
			std::function<void()> callback);

		void CommitAsynResourceTask(
			const kchar *fileName,
			BytesPackage & bp,
			std::semaphore & sp
			);*/

		void CommitAsynResourceTask(
			const kchar *fileName,
			BytesPackage &bp,
			std::atomic<bool> &finished);

		void CommitSynResourceTask(
			const kchar *fileName,
			BytesPackage & bp
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

		AssetManager();

	public:
		typedef std::shared_ptr<IIODevice> SpIODevice;

		static SpIODevice		OpenAsset(const kchar * assetPath, IOFlag openFlag = IORead, bool fast = false);

		static kString			AssetPath(const kchar * assetRelativePath);

        static IAsset * 		Open(const char* path);
	protected:
		static	kString	 s_envAssetPath;

		std::vector<kString>    m_SearchPaths;
		//std::thread_pool*        m_pThreadPool;

		MapMesh                 m_MeshMap;
		MapImage                m_ImageMap;

		mutable bool            m_IsLoading;
		mutable bool            m_HasPendingObject;

		std::atomic<int>        m_NumPendingObject;
	};
}

#endif
