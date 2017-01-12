#include "Common.h"
#include <Core/MeshData.h>
#include <Core/CameraData.h>

using namespace k3d;
using namespace std;

threadptr TestBundle()
{
	auto ret = std::make_shared<std::thread>([]() {
		Os::File file("../../Data/Test/test.bundle");
		file.Open(IORead);
		Archive arch;
		arch.SetIODevice(&file);
		EAssetVersion bundleVer;
		arch >> bundleVer;
		int64 szTable;
		arch >> szTable;
		uint64 chunkCnt = szTable / sizeof(AssetChunk);
		vector<AssetChunk> chunks(chunkCnt);
		for (uint32 i = 0; i < chunkCnt; i++)
		{
			arch >> chunks[i];
		}

		for (uint32 i = 0; i < chunkCnt; i++)
		{
			EAssetType type;
			arch >> type;
			K3D_ASSERT(type == chunks[i].Type);
			MeshData meshData;
			CameraData camData;
			switch (type)
			{
			case EAssetType::EMesh:
				EMeshVersion meshVer;
				arch >> meshVer;
				file.Skip( 64);//class name
				arch >> meshData;
				break;
			case EAssetType::ECamera:
				ECamVersion camVer;
				arch >> camVer;
				file.Skip( 64);//class name
				arch >> camData;
				break;
			}
			arch >> type;
			K3D_ASSERT(type == EAssetType::EChunkEnd);
		}

		file.Close();
	});
	return ret;
}