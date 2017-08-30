#include "CoreMinimal.h"
#include "AssetManager.h"
#include "XPlatform/Os.h"
//#include "Core/LogUtil.h"
#include "XPlatform/App.h"

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

#if 0

namespace k3d
{
	AssetManager::AssetManager()/* : m_pThreadPool(nullptr) */
	{
	}

	void AssetManager::Init()
	{
	}

	void AssetManager::Shutdown()
	{
		KLOG(Info, "AssetManager", "Shutdown.");
	}


	class MemmapedAsset : public IAsset
	{
	public:
		MemmapedAsset(const char * path)
		{
			m_File = MakeShared<Os::MemMapFile>();
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
		SharedPtr<Os::MemMapFile> m_File;
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
            String RealPath(path);
            RealPath = Os::Path::Join(GetEnv()->GetDataDir(), RealPath.SubStr(8, RealPath.Length()-8));
			return new MemmapedAsset(RealPath.CStr());
#endif
		}
		return nullptr;
	}

}
#endif