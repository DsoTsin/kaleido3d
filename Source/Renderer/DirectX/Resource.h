#pragma once
#include "DXCommon.h"

namespace k3d {
	namespace d3d12 {

		class Resource {
		protected:
			PtrResource m_Resource;
		};

		class CommittedResource : public Resource
		{
		protected:
			void _Create(
				ID3D12Device* device,
				int64_t size,
				_In_opt_ const void* data);

		public:
			void UploadData(const void* data, int64_t size);

			auto Get() const { return m_Resource; }
			auto GetSize() const { return m_Size; }

		protected:
			int64_t m_Size;
		};

	}
}