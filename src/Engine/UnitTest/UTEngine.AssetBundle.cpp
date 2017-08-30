#include "Common.h"
#include <Engine/Assets/MeshData.h>
#include <Engine/Assets/CameraData.h>

#if K3DPLATFORM_OS_WIN
#pragma comment(linker,"/subsystem:console")
#endif

using namespace k3d;
using namespace std;

void TestBundle()
{
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
}

int main(int argc, char**argv)
{
	TestBundle();
	return 0;
}