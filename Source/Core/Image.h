#ifndef __Image_h__
#define __Image_h__
#pragma once

#include <Config/Prerequisities.h>

namespace k3d {

	class ImagePrivate;

	/// \brief The Image class
	/// \class Image : client side texture data
	/// \see  k3dTexture
	class Image {
	public:
		enum Type {
			TEXTURE_2D,
			TEXTURE_3D,
			TEXTURE_CUBE,
			UNKNOWN
		};

		Image();
		virtual ~Image();

		KOBJECT_PROPERTY(Name, k3dString);
		KOBJECT_PROPERTY_GET(Width, uint32);
		KOBJECT_PROPERTY_GET(Height, uint32);
		KOBJECT_PROPERTY_GET(Depth, uint32);
		KOBJECT_PROPERTY_GET(MipLevs, uint32);
		KOBJECT_PROPERTY_GET(Layers, uint32);

		KOBJECT_PROPERTY_GET(InternalFmt, uint32);
		KOBJECT_PROPERTY_GET(FillFmt, uint32);
		KOBJECT_PROPERTY_GET(DataType, uint32);
		KOBJECT_PROPERTY_GET(Data, kByte*);

		uint32 GetImageSize(uint32 level) const;
		const void * GetLevel(uint32 level, uint32 face) const;

		virtual bool Load(uint8 *dataPtr, uint32 length);
		virtual bool IsCompressed() const;
		virtual bool IsCubeMap() const;

		friend class k3dAssetManager;
		typedef std::vector<kByte*> ByteVec;

	protected:

		uint32    m_ImgWidth;
		uint32    m_ImgHeight;
		uint32    m_ImgDepth;
		class ImagePrivate;
		uint32    m_ImgLayers;
		//  uint32    m_ImgCnt;
		int32     m_MipLev;
		//  Type      m_ImgType;
		uint32    m_InternalFmt;
		uint32    m_FillFmt;
		uint32    m_DataType;
		uint32    m_ElementSize;

		ByteVec   m_ImgData;

		bool      m_IsCubeMap;
		bool      m_IsCompressed;

		k3dString m_ImgName;
		ImagePrivate *d;
	};

}
#endif
