#include "Kaleido3D.h"
#include "Bundle.h"
#include "MeshData.h"
#include "CameraData.h"
#include "ImageData.h"
#include "Os.h"
#include "LogUtil.h"
#include "Utils/StringUtils.h"
#include <list>

using namespace std;

namespace k3d
{
	class AssetBundleImpl
	{
	public:
		Os::File			BundleFile;
		Archive				Archv;

		kString				BundleDir;
		kString				CacheDir;
		kString				BundleName;
		list<MeshData*>		Meshes;
		list<CameraData*>	Cameras;
		list<AssetChunk*>	Chunks;
		bool				Opened;

		void Initialize()
		{
			CacheDir = BundleDir + BundleName;
			auto bundlePath = BundleDir + BundleName + KT(".bundle");
			Opened = BundleFile.Open(bundlePath.c_str(), IOWrite);
			if (Opened)
			{
				KLOG(Info, AssetBundleImpl, "Initialize");
				Archv.SetIODevice(&BundleFile);
			}
		}

		void Close()
		{
			if (Opened)
			{
				KLOG(Info, AssetBundleImpl, "Close");
				BundleFile.Close();
			}
		}

		uint64 GetMergedTableSize()
		{
			return Chunks.size() * sizeof(AssetChunk);
		}

		void Serialize(MeshData * mesh)
		{
			if (!mesh)
				return;
#if K3DPLATFORM_OS_WIN
			wchar_t name[128];
			StringUtil::CharToWchar(mesh->Name(), name, 128);
			auto path = CacheDir + KT("/") + name;
#else
			auto path = CacheDir + KT("/") + mesh->Name();
#endif
			Os::File file;
			KLOG(Info, AssetBundleImpl, "Serialize Mesh: %s", mesh->Name());
			file.Open(path.c_str(),IOWrite);
			Archive archive;
			archive.SetIODevice(&file);
			EMeshVersion mVer = EMeshVersion::VERSION_1_1;
			archive << mVer;
			archive << *mesh;
			AssetChunk* chunk = new AssetChunk;
			memset(chunk, 0, sizeof(AssetChunk));
			chunk->Type = EAssetType::EMesh;
			chunk->Size = file.GetSize();
			strncpy(chunk->Name, mesh->Name(), 64);
			Chunks.push_back(chunk);
			file.Close();
		}

		void Serialize(CameraData * camera)
		{
			if (!camera)
				return;
#if K3DPLATFORM_OS_WIN
			wchar_t name[128];
			StringUtil::CharToWchar(camera->Name(), name, 128);
			auto path = CacheDir + KT("/") + name;
#else
			auto path = CacheDir + KT("/") + camera->Name();
#endif
			Os::File file;
			KLOG(Info, AssetBundleImpl, "Serialize Camera: %s", camera->Name());
			file.Open(path.c_str(), IOWrite);
			Archive archive;
			archive.SetIODevice(&file);
			ECamVersion cVer = ECamVersion::VERSION_1_0;
			archive << cVer;
			archive << *camera;
			AssetChunk* chunk = new AssetChunk;
			memset(chunk, 0, sizeof(AssetChunk));
			chunk->Type = EAssetType::ECamera;
			chunk->Size = file.GetSize();
			strncpy(chunk->Name, camera->Name(), 64);
			Chunks.push_back(chunk);
			file.Close();
		}

		// write chunk table
		void DumpChunkTable()
		{
			Archv << GetMergedTableSize();
			for (auto Chunk : Chunks)
			{
				Archv << *Chunk;
			}
		}

		void DumpChunk(AssetChunk * chunk)
		{
			if (!chunk)
				return;

#if K3DPLATFORM_OS_WIN
			wchar_t name[128];
			StringUtil::CharToWchar(chunk->Name, name, 128);
			auto path = CacheDir + KT("/") + name;
#else
			auto path = CacheDir + KT("/")+ chunk->Name;
#endif
			Os::File file;
			bool opened = file.Open(path.c_str(), IORead);
            if(!opened)
                return;
			int64 szFile = file.GetSize();
			char * data = new char[szFile];
			file.Read(data, szFile);
			// Write Chunk header
			Archv << chunk->Type;
			Archv.ArrayIn(data, szFile);
			// Write Chunk tail
			Archv << EAssetType::EChunkEnd;
			file.Close();
			delete[] data;
		}
	};

	AssetBundle* AssetBundle::CreateBundle(const kchar * bundleName, const kchar * bundleDir)
	{
		return new AssetBundle(bundleName, bundleDir);
	}

	void AssetBundle::Serialize(MeshData * mesh)
	{
		d->Serialize(mesh);
	}
	
	void AssetBundle::Serialize(CameraData * camera)
	{
		d->Serialize(camera);
	}
	
	void AssetBundle::MergeAndBundle(bool deleteCache)
	{
		m_IsBundling = true;
		EAssetVersion bundleVer = EAssetVersion::E20161210u;
		d->Archv << bundleVer;
		d->DumpChunkTable();
		for (auto c : d->Chunks)
		{
			d->DumpChunk(c);
			delete c;
		}
		if (deleteCache) 
		{
			Os::Remove(d->CacheDir.c_str());
		}
		d->Close();
		m_IsBundling = false;
	}

	AssetBundle::AssetBundle(const kchar * bundleName, const kchar * bundleDir)
		: d(new AssetBundleImpl)
		, m_IsBundling(false)
	{
		d->BundleDir = bundleDir;
		kString _Name(bundleName);
		size_t dotpos = _Name.find_last_of(KT("."));
		d->BundleName = _Name.substr(0, dotpos);
		d->Initialize();
	}

	AssetBundle::~AssetBundle()
	{
		if (d)
		{
			d->Close();
			delete d;
			d = nullptr;
		}
	}

	void AssetBundle::Prepare()
	{
		auto bundleTmpCache = d->BundleDir + d->BundleName;
		Os::MakeDir(bundleTmpCache.c_str());
	}

}
