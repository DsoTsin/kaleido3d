#pragma once

namespace k3d
{
	namespace d3d12
	{
		/**
		* DescriptorHandle from [MiniEngine Demo](Microsoft)
		*/
		class DescriptorHandle
		{
		public:
			DescriptorHandle()
			{
				CpuHandle.ptr = ~0ull;
				GpuHandle.ptr = ~0ull;
			}

			DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle)
				: CpuHandle(CpuHandle)
			{
				GpuHandle.ptr = ~0ull;
			}

			DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle)
				: CpuHandle(CpuHandle), GpuHandle(GpuHandle)
			{
			}

			DescriptorHandle operator+ (ULONG OffsetScaledByDescriptorSize) const
			{
				DescriptorHandle ret = *this;
				ret += OffsetScaledByDescriptorSize;
				return ret;
			}

			void operator += (ULONG OffsetScaledByDescriptorSize)
			{
				if (CpuHandle.ptr != ~0ull)
					CpuHandle.ptr += OffsetScaledByDescriptorSize;
				if (GpuHandle.ptr != ~0ull)
					GpuHandle.ptr += OffsetScaledByDescriptorSize;
			}

			bool IsNull() const { return CpuHandle.ptr == ~0ull; }
			bool IsShaderVisible() const { return GpuHandle.ptr != ~0ull; }

			D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
			D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;
		};
	}
}
