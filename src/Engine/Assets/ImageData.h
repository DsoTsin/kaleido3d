#ifndef __Image_h__
#define __Image_h__
#pragma once

#include "Bundle.h"
#include <KTL/String.hpp>
#include <KTL/DynArray.hpp>

namespace k3d 
{
	/// \brief The Image class
	/// \class Image : client side texture data
	/// \see  k3dTexture
	class ImageData
	{
	public:
		enum Type {
			TEXTURE_2D,
			TEXTURE_3D,
			TEXTURE_CUBE,
			UNKNOWN
		};

		ImageData();
		virtual ~ImageData();

		using string = String;

		KOBJECT_PROPERTY(Name, string);
		KOBJECT_PROPERTY_GET(Width, uint32);
		KOBJECT_PROPERTY_GET(Height, uint32);
		KOBJECT_PROPERTY_GET(Depth, uint32);
		KOBJECT_PROPERTY_GET(MipLevs, uint32);
		KOBJECT_PROPERTY_GET(Layers, uint32);

		KOBJECT_PROPERTY_GET(InternalFmt, uint32);
		KOBJECT_PROPERTY_GET(FillFmt, uint32);
		KOBJECT_PROPERTY_GET(DataType, uint32);
		KOBJECT_PROPERTY_GET(Data, kByte*);


		KOBJECT_CLASSNAME(ImageData)

		uint32 GetImageSize(uint32 level) const;
		const void * GetLevel(uint32 level, uint32 face) const;

		virtual bool Load(uint8 *dataPtr, uint32 length);
		virtual bool IsCompressed() const;
		virtual bool IsCubeMap() const;

		friend class k3dAssetManager;
		typedef DynArray<kByte*> ByteVec;

	protected:

		uint32    m_ImgWidth;
		uint32    m_ImgHeight;
		uint32    m_ImgDepth;
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

		string	  m_ImgName;
	};

}
#endif
