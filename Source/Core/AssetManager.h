#ifndef __AssetManager_h__
#define __AssetManager_h__
#pragma once

#include <KTL/Singleton.hpp>
#include <Interface/IIODevice.h>

#include "MeshData.h"

#include <atomic>
#include <memory>

namespace k3d
{
	class	ImageData;
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
		virtual uint64 Read(void *data, uint64 size) = 0;
		virtual bool Seek(uint64 offset) = 0;
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
		std::shared_ptr<MeshData> FindMesh(const char *meshName);

		/// \brief FindImage
		/// \param imgName
		/// \return std::shared_ptr<Image>
		std::shared_ptr<ImageData> FindImage(const char *imgName);

		using string = std::string;

		typedef std::unordered_map<string, SpMesh> MapMesh;
		typedef MapMesh::iterator MapMeshIter;

		typedef std::unordered_map<string, std::shared_ptr<ImageData> > MapImage;
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
