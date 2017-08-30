#ifndef __AssetManager_h__
#define __AssetManager_h__
#pragma once

#include "KTL/Singleton.hpp"

#if 0
//#include <Core/Interface/IIODevice.h>

//#include "MeshData.h"

#include <atomic>
#include <memory>

namespace k3d
{
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
	class K3D_CORE_API AssetManager : public Singleton<AssetManager>
	{
	public:
		void Init();

		void Shutdown();

		AssetManager();

	public:
		
        static IAsset * 		Open(const char* path);
	protected:
	};
}

#endif
#endif
