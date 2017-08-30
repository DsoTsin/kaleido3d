#include "Kaleido3D.h"
#include <Core/Os.h>
#include <Core/LogUtil.h>
#include <KTL/String.hpp>
#include "Bundle.h"
#include "MeshData.h"
#include "CameraData.h"
#include "ImageData.h"
#include <list>

using namespace std;

namespace k3d
{
	class AssetBundleImpl
	{
	public:
		Os::File			BundleFile;
		Archive				Archv;

		String				BundleDir;
		String				CacheDir;
		String				BundleName;
		list<MeshData*>		Meshes;
		list<CameraData*>	Cameras;
		list<AssetChunk*>	Chunks;
		bool				Opened;

		void Initialize()
		{
			CacheDir = BundleDir + BundleName;
			auto bundlePath = BundleDir + BundleName + ".bundle";
			Opened = BundleFile.Open(bundlePath.CStr(), IOWrite);
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
			auto path = CacheDir + "/" + mesh->Name();
			Os::File file;
			KLOG(Info, AssetBundleImpl, "Serialize Mesh: %s", mesh->Name());
			file.Open(path.CStr(),IOWrite);
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
			auto path = CacheDir + "/" + camera->Name();
			Os::File file;
			KLOG(Info, AssetBundleImpl, "Serialize Camera: %s", camera->Name());
			file.Open(path.CStr(), IOWrite);
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
			auto path = CacheDir + "/"+ chunk->Name;
			Os::File file;
			bool opened = file.Open(path.CStr(), IORead);
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

	AssetBundle* AssetBundle::CreateBundle(const char * bundleName, const char * bundleDir)
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
			Os::Path::Remove(d->CacheDir.CStr());
		}
		d->Close();
		m_IsBundling = false;
	}

	AssetBundle::AssetBundle(const char * bundleName, const char * bundleDir)
		: d(new AssetBundleImpl)
		, m_IsBundling(false)
	{
		d->BundleDir = bundleDir;
//		String _Name(bundleName);
        /*size_t dotpos = _Name.find_last_of(KT("."));
        d->BundleName = _Name.substr(0, dotpos);*/
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
		Os::Path::MakeDir(bundleTmpCache.CStr());
	}

}
