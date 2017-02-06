#include "Kaleido3D.h"
#include "AssetManager.h"
#include "Os.h"
#include "Core/LogUtil.h"
#include "ImageData.h"
#include "App.h"
#include "ObjectMesh.h"

#ifdef USE_TBB_MALLOC
#include <tbb/scalable_allocator.h>
#endif

#if K3DPLATFORM_OS_ANDROID
#include <android/asset_manager.h>
#endif

#include <fstream>
#include <algorithm>

#if K3DPLATFORM_OS_WIN
#include <strsafe.h>
#endif

namespace k3d
{

	kString AssetManager::s_envAssetPath;

	AssetManager::AssetManager()/* : m_pThreadPool(nullptr) */
	{
		m_IsLoading = false;
		m_HasPendingObject = false;
		m_NumPendingObject.store(0, std::memory_order_release);
	}

	void AssetManager::Init()
	{
		/*if (m_pThreadPool == nullptr)
			m_pThreadPool = new std::thread_pool(2);*/

		KLOG(Info, "AssetManager"," Initialized With two workers.");

#if K3DPLATFORM_OS_WIN
		kchar _path[2048] = { 0 };
		DWORD _len = GetEnvironmentVariableW(L"Kaleido3D_Dir", _path, 2048);
		if (_len > 0) {
			KLOG(Info, "AssetManager", "Kaleido3D_Dir Found. ");
			s_envAssetPath = _path;
			m_SearchPaths.push_back(_path);
		}
		else {
			KLOG(Error, "AssetManager", "Kaleido3D_Dir Not Found.");
		}
#endif
		//m_SearchPaths
	}

	void AssetManager::Shutdown()
	{
		KLOG(Info, "AssetManager", "Shutdown.");
	}

	void AssetManager::LoadAssetDescFile(const char *fileName)
	{
		std::ifstream ifs(fileName);
		if (!ifs.good())
		{
			KLOG(Fatal, "AssetManager", "LoadAssetDescFile failed. Cannot find file %s.", fileName);
			return;
		}
		ifs.close();
	}

	void AssetManager::AddSearchPath(const kchar *path)
	{
		typedef std::vector<kString>::const_iterator VSCIter;

		VSCIter pos = std::find(m_SearchPaths.begin(), m_SearchPaths.end(), kString(path));
		if (pos == m_SearchPaths.end()) {
			m_SearchPaths.push_back(kString(path));
		}
	}

	//void AssetManager::CommitAsynResourceTask(
	//	const kchar *fileName,
	//	BytesPackage &bp,
	//	std::semaphore &sp,
	//	std::function<void()> callback)
	//{
	//	auto result = m_pThreadPool->enqueue(
	//		[&bp, &sp](const kchar *name, std::function<void()> _callback)
	//	{
	//		//    ++m_NumPendingObject;
	//		MemMapFile file;
	//		bool opened = file.Open(name, IORead);
	//		assert(opened == true);
	//		if (opened)
	//		{
	//			int64 bytes_length = file.GetSize();
	//			bp.Bytes.assign(bytes_length + 1, '\0');
	//			//bp.Bytes.assign()
	//			file.Read((char*)&bp.Bytes[0], bytes_length);
	//			file.Close();

	//			_callback();
	//		}
	//		else
	//		{
	//			Log::Out(LogLevel::Error, "AssetManager", "CommitAsynResourceTask failed. Cannot find file %s.", name);
	//		}
	//		sp.notify();

	//		//    --m_NumPendingObject;
	//	},
	//		fileName,
	//		callback
	//		);
	//}

	//auto AssetManager::AsyncLoadObject(const kchar * objPath, ObjectLoadListener* listener)
	//{
	//	auto result = m_pThreadPool->enqueue([&listener](const kchar *path) {
	//		AssetManager::SpIODevice asset = OpenAsset(path, IORead, true);
	//		std::shared_ptr<MemMapFile> file = std::static_pointer_cast<MemMapFile>(asset);
	//		int64 bufferSize = file->GetSize();
	//		(void)bufferSize;
	//		//char * buf = (char*)::scalable_malloc(bufferSize);
	//		listener->OnLoad();
	//	}, objPath);
	//	return result;
	//}

	//void AssetManager::CommitAsynResourceTask(const kchar *fileName, BytesPackage &bp, std::semaphore &sp)
	//{
	//	auto result = m_pThreadPool->enqueue(
	//		[&bp, &sp](const kchar *name)
	//	{
	//		//    ++m_NumPendingObject;
	//		MemMapFile file;
	//		bool opened = file.Open(name, IORead);
	//		if (opened)
	//		{
	//			int bytes_length = (int)file.GetSize();
	//			bp.Bytes.reserve(bytes_length);
	//			file.Read((char*)&bp.Bytes[0], bytes_length);
	//			file.Close();
	//		}
	//		else
	//		{
	//			Log::Out(LogLevel::Error, "AssetManager", "CommitSynResourceTask failed. Cannot find file %s.", name);
	//		}
	//		sp.notify();
	//		//    --m_NumPendingObject;
	//	},
	//		fileName
	//		);
	//}

	void AssetManager::CommitAsynResourceTask(const kchar *fileName, BytesPackage &bp, std::atomic<bool> &finished)
	{
		//auto result = m_pThreadPool->enqueue(
		//	[&bp, &finished](const kchar *name)
		//{
		//	//    ++m_NumPendingObject;
		//	MemMapFile file;
		//	bool opened = file.Open(name, IORead);
		//	if (opened)
		//	{
		//		int bytes_length = (int)file.GetSize();
		//		bp.Bytes.reserve(bytes_length);
		//		file.Read((char*)&bp.Bytes[0], bytes_length);
		//		file.Close();
		//	}
		//	else
		//	{
		//		Log::Out(LogLevel::Error, "AssetManager", "CommitSynResourceTask failed. Cannot find file %s.", name);
		//	}
		//	finished.store(true, std::memory_order_release);

		//	//    --m_NumPendingObject;
		//},
		//	fileName
		//	);
	}

	void AssetManager::CommitSynResourceTask(const kchar *fileName, BytesPackage &bp)
	{
		Os::File file;
		if (file.Open(fileName, IORead))
		{
			int64 length = file.GetSize();
			bp.Bytes.resize(length);
			int64 __read = file.Read((char*)&bp.Bytes[0], length);
			assert(__read == length);
			file.Close();
		}
	}

	void AssetManager::AppendMesh(SpMesh meshPtr)
	{
		KLOG(Info, "AssetManager","Mesh (%s) Appended.", meshPtr->Name());
		//KLOG(Info, "AssetManager", meshPtr->DumpMeshInfo());
		m_MeshMap[meshPtr->Name()] = meshPtr;
	}

	void AssetManager::Free(char *byte_ptr)
	{
		::free(byte_ptr);
	}

	std::shared_ptr<MeshData> AssetManager::FindMesh(const char *meshName)
	{
		MapMeshIter iter = m_MeshMap.find(string(meshName));
		if (iter == m_MeshMap.end())
			return std::shared_ptr<MeshData>();
		return (iter->second);
	}

	std::shared_ptr<ImageData> AssetManager::FindImage(const char *imgName)
	{
		MapImageIter iter = m_ImageMap.find(string(imgName));
		if (iter == m_ImageMap.end())
			return std::shared_ptr<ImageData>();
		return (iter->second);
	}


	AssetManager::SpIODevice  AssetManager::OpenAsset(const kchar *assetPath, IOFlag flag, bool fastMode)
	{
		kString rawPath = AssetPath(assetPath);
		SpIODevice fileObj = nullptr;
		if (fastMode) 
		{
			fileObj = SpIODevice(new Os::MemMapFile);
		}
		else 
		{
			fileObj = SpIODevice(new Os::File);
		}
        if(fileObj->Open(rawPath.c_str(), flag))
            return fileObj;
        KLOG(Error, "AssetManager","Cann't find file (%s).", rawPath.c_str());
		fileObj->Close();
		return nullptr;
	}

	kString AssetManager::AssetPath(const kchar * assetRelativePath)
	{
		if(s_envAssetPath.empty()) {
			KLOG(Error, "AssetManager", "Kaleido3D_Dir Not Found.");
			return assetRelativePath;
		}
		return s_envAssetPath + assetRelativePath;
	}

	class MemmapedAsset : public IAsset
	{
	public:
		MemmapedAsset(const char * path)
		{
			m_File = std::make_shared<Os::MemMapFile>();
			m_File->Open(path, IORead);
		}

		~MemmapedAsset() override {}

		uint64 GetLength() override
		{	return m_File->GetSize();	}

		const void * GetBuffer() override
		{	return m_File->FileData(); }

		uint64 Read(void *data, uint64 size) override { return m_File->Read((char*)data, (size_t)size); }
		bool Seek(uint64 offset) override { return m_File->Seek((size_t)offset); }

	private:
		std::shared_ptr<Os::MemMapFile> m_File;
	};

#if K3DPLATFORM_OS_ANDROID
	class AndroidAsset : public IAsset
	{
	public:
		AndroidAsset(AAsset * asset) : m_Asset(asset) {}
		~AndroidAsset() override { if(m_Asset) AAsset_close(m_Asset); }

		uint64 		GetLength() override { return (uint64)AAsset_getLength64(m_Asset); }
		const void* GetBuffer() override { return AAsset_isAllocated(m_Asset) ? AAsset_getBuffer(m_Asset) : nullptr; }

		uint64 Read(void *data, uint64 size) override { return (uint64)AAsset_read(m_Asset, data, (size_t)size); }
		bool Seek(uint64 offset) override { return AAsset_seek64(m_Asset, offset, SEEK_SET)!=-1; }

	private:
		AAsset	* m_Asset;
	};
#endif

	IAsset *AssetManager::Open(const char *path)
	{
		if(strncmp(path, "asset://", 8)==0) {
#if K3DPLATFORM_OS_ANDROID
			AAssetManager* mgr = GetEnv()->GetAssets();
			std::string relpath(path);
			relpath = relpath.substr(8, relpath.size() - 8);
			AAsset * asset = AAssetManager_open(mgr, relpath.c_str(), AASSET_MODE_STREAMING);
			K3D_ASSERT(asset!= nullptr);
			return new AndroidAsset(asset);
#else
            std::string relpath(path);
			relpath = "../../Data/" + relpath.substr(8, relpath.size() - 8);
			return new MemmapedAsset(relpath.c_str());
#endif
		}
		return nullptr;
	}

}