#pragma once

#include <Core/KTL/Archive.hpp>

namespace k3d
{
	enum class EAssetVersion : uint64
	{
		E20161210u,
	};

	/// \brief asset type : include shader, mesh, camera, material, image
	/// \class EAssetType 
	enum class EAssetType : uint32
	{
		EImage			 = 0x84,
		EShaderSource	 = 0x85,
		EShaderBytes	 = 0x86,
		EMesh			 = 0x87,
		ECamera			 = 0x88,

		EChunkEnd		 = 0xFF
	};

	struct AssetHeader
	{
		EAssetVersion Version;
	};

	struct AssetTable
	{
		uint64 Size;
	};

	/// \brief identifier for a chunk of asset
	/// \class AssetChunk 
	struct AssetChunk
	{
		EAssetType	Type;
		int64		Size;
		char		Name[64];
	};

	class ImageData;
	class MeshData;
	class CameraData;
	class ShaderData;

	class K3D_CORE_API AssetBundle
	{
	public:
		static AssetBundle * CreateBundle(const char * bundleName, const char * bundleDir);
		~AssetBundle();

		void Prepare();

		void Serialize(MeshData *);
		void Serialize(CameraData *);

		void MergeAndBundle(bool deleteCache);

	private:
		AssetBundle(const char * bundleName, const char * bundleDir);
		class AssetBundleImpl * d;

	private:
		bool m_IsBundling;
	};
}