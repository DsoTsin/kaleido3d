#include "Kaleido3D.h"
#include "Helper.h"

#include <d3d9.h>
#include <d3d11.h>

namespace k3d
{
namespace d3d11
{
	void Helper::ConvertVertexFormatToInputElementDesc(VtxFormat const & format, std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc)
	{
		switch (format) {
		case VtxFormat::POS3_F32_NOR3_F32_UV2_F32:
			inputDesc.push_back({ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 });
			inputDesc.push_back({ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	12,	D3D11_INPUT_PER_VERTEX_DATA, 0 });
			inputDesc.push_back({ "TEXCOOD0",	0, DXGI_FORMAT_R32G32_FLOAT,	0,	24,	D3D11_INPUT_PER_VERTEX_DATA, 0 });
			break;
		case VtxFormat::POS3_F32_NOR3_F32:
			inputDesc.push_back({ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
			inputDesc.push_back({ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
			break;
		default:
			break;
		}
	}

	//--------------------------------------------------------------------------------------
	// Helper functions to create SRGB formats from typeless formats and vice versa
	//--------------------------------------------------------------------------------------
	DXGI_FORMAT MAKE_SRGB(DXGI_FORMAT format)
	{
		/*if (!DXUTIsInGammaCorrectMode())
		return format;*/

		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
			return DXGI_FORMAT_BC1_UNORM_SRGB;
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
			return DXGI_FORMAT_BC2_UNORM_SRGB;
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
			return DXGI_FORMAT_BC3_UNORM_SRGB;

		};

		return format;
	}


	//--------------------------------------------------------------------------------------
	static HRESULT LoadTextureDataFromFile(__in_z const WCHAR* szFileName, BYTE** ppHeapData,
		DDS_HEADER** ppHeader,
		BYTE** ppBitData, UINT* pBitSize)
	{
		// open the file
		HANDLE hFile = CreateFileW(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
			return HRESULT_FROM_WIN32(GetLastError());

		// Get the file size
		LARGE_INTEGER FileSize = { 0 };
		GetFileSizeEx(hFile, &FileSize);

		// File is too big for 32-bit allocation, so reject read
		if (FileSize.HighPart > 0)
		{
			CloseHandle(hFile);
			return E_FAIL;
		}

		// Need at least enough data to fill the header and magic number to be a valid DDS
		if (FileSize.LowPart < (sizeof(DDS_HEADER) + sizeof(DWORD)))
		{
			CloseHandle(hFile);
			return E_FAIL;
		}

		// create enough space for the file data
		*ppHeapData = new BYTE[FileSize.LowPart];
		if (!(*ppHeapData))
		{
			CloseHandle(hFile);
			return E_OUTOFMEMORY;
		}

		// read the data in
		DWORD BytesRead = 0;
		if (!ReadFile(hFile, *ppHeapData, FileSize.LowPart, &BytesRead, NULL))
		{
			CloseHandle(hFile);
			SAFE_DELETE_ARRAY(*ppHeapData);
			return HRESULT_FROM_WIN32(GetLastError());
		}

		if (BytesRead < FileSize.LowPart)
		{
			CloseHandle(hFile);
			SAFE_DELETE_ARRAY(*ppHeapData);
			return E_FAIL;
		}

		// DDS files always start with the same magic number ("DDS ")
		DWORD dwMagicNumber = *(DWORD*)(*ppHeapData);
		if (dwMagicNumber != DDS_MAGIC)
		{
			CloseHandle(hFile);
			SAFE_DELETE_ARRAY(*ppHeapData);
			return E_FAIL;
		}

		DDS_HEADER* pHeader = reinterpret_cast<DDS_HEADER*>(*ppHeapData + sizeof(DWORD));

		// Verify header to validate DDS file
		if (pHeader->dwSize != sizeof(DDS_HEADER)
			|| pHeader->ddspf.dwSize != sizeof(DDS_PIXELFORMAT))
		{
			CloseHandle(hFile);
			SAFE_DELETE_ARRAY(*ppHeapData);
			return E_FAIL;
		}

		// Check for DX10 extension
		bool bDXT10Header = false;
		if ((pHeader->ddspf.dwFlags & DDS_FOURCC)
			&& (MAKEFOURCC('D', 'X', '1', '0') == pHeader->ddspf.dwFourCC))
		{
			// Must be long enough for both headers and magic value
			if (FileSize.LowPart < (sizeof(DDS_HEADER) + sizeof(DWORD) + sizeof(DDS_HEADER_DXT10)))
			{
				CloseHandle(hFile);
				SAFE_DELETE_ARRAY(*ppHeapData);
				return E_FAIL;
			}

			bDXT10Header = true;
		}

		// setup the pointers in the process request
		*ppHeader = pHeader;
		INT offset = sizeof(DWORD) + sizeof(DDS_HEADER)
			+ (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);
		*ppBitData = *ppHeapData + offset;
		*pBitSize = FileSize.LowPart - offset;

		CloseHandle(hFile);

		return S_OK;
	}

	static UINT BitsPerPixel(DXGI_FORMAT fmt)
	{
		switch (fmt)
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return 128;

		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			return 96;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return 64;

		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return 32;

		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return 16;

		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_A8_UNORM:
			return 8;

		case DXGI_FORMAT_R1_UNORM:
			return 1;

		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
			return 4;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return 8;

		default:
			assert(FALSE); // unhandled format
			return 0;
		}
	}

	static void GetSurfaceInfo(UINT width, UINT height, DXGI_FORMAT fmt, UINT* pNumBytes, UINT* pRowBytes, UINT* pNumRows)
	{
		UINT numBytes = 0;
		UINT rowBytes = 0;
		UINT numRows = 0;

		bool bc = true;
		int bcnumBytesPerBlock = 16;
		switch (fmt)
		{
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			bcnumBytesPerBlock = 8;
			break;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			break;

		default:
			bc = false;
			break;
		}

		if (bc)
		{
			int numBlocksWide = 0;
			if (width > 0)
				numBlocksWide = max(1, width / 4);
			int numBlocksHigh = 0;
			if (height > 0)
				numBlocksHigh = max(1, height / 4);
			rowBytes = numBlocksWide * bcnumBytesPerBlock;
			numRows = numBlocksHigh;
		}
		else
		{
			UINT bpp = BitsPerPixel(fmt);
			rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
			numRows = height;
		}
		numBytes = rowBytes * numRows;
		if (pNumBytes != NULL)
			*pNumBytes = numBytes;
		if (pRowBytes != NULL)
			*pRowBytes = rowBytes;
		if (pNumRows != NULL)
			*pNumRows = numRows;
	}

	#define ISBITMASK( r,g,b,a ) ( ddpf.dwRBitMask == r && ddpf.dwGBitMask == g && ddpf.dwBBitMask == b && ddpf.dwABitMask == a )

	static DXGI_FORMAT GetDXGIFormat(const DDS_PIXELFORMAT& ddpf)
	{
		if (ddpf.dwFlags & DDS_RGB)
		{
			switch (ddpf.dwRGBBitCount)
			{
			case 32:
				// DXGI_FORMAT_B8G8R8A8_UNORM_SRGB & DXGI_FORMAT_B8G8R8X8_UNORM_SRGB should be
				// written using the DX10 extended header instead since these formats require
				// DXGI 1.1
				//
				// This code will use the fallback to swizzle RGB to BGR in memory for standard
				// DDS files which works on 10 and 10.1 devices with WDDM 1.0 drivers
				//
				// NOTE: We don't use DXGI_FORMAT_B8G8R8X8_UNORM or DXGI_FORMAT_B8G8R8X8_UNORM
				// here because they were defined for DXGI 1.0 but were not required for D3D10/10.1

				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
					return DXGI_FORMAT_R8G8B8A8_UNORM;
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000))
					return DXGI_FORMAT_R8G8B8A8_UNORM; // No D3DFMT_X8B8G8R8 in DXGI

													   // Note that many common DDS reader/writers swap the
													   // the RED/BLUE masks for 10:10:10:2 formats. We assumme
													   // below that the 'correct' header mask is being used. The
													   // more robust solution is to use the 'DX10' header extension and
													   // specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly
				if (ISBITMASK(0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000))
					return DXGI_FORMAT_R10G10B10A2_UNORM;

				if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
					return DXGI_FORMAT_R16G16_UNORM;

				if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
					// Only 32-bit color channel format in D3D9 was R32F
					return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
				break;

			case 24:
				// No 24bpp DXGI formats
				break;

			case 16:
				// 5:5:5 & 5:6:5 formats are defined for DXGI, but are deprecated for D3D10+

				if (ISBITMASK(0x0000f800, 0x000007e0, 0x0000001f, 0x00000000))
					return DXGI_FORMAT_B5G6R5_UNORM;
				if (ISBITMASK(0x00007c00, 0x000003e0, 0x0000001f, 0x00008000))
					return DXGI_FORMAT_B5G5R5A1_UNORM;
				if (ISBITMASK(0x00007c00, 0x000003e0, 0x0000001f, 0x00000000))
					return DXGI_FORMAT_B5G5R5A1_UNORM; // No D3DFMT_X1R5G5B5 in DXGI

													   // No 4bpp or 3:3:2 DXGI formats
				break;
			}
		}
		else if (ddpf.dwFlags & DDS_LUMINANCE)
		{
			if (8 == ddpf.dwRGBBitCount)
			{
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
					return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension

												 // No 4bpp DXGI formats
			}

			if (16 == ddpf.dwRGBBitCount)
			{
				if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
					return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
					return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
			}
		}
		else if (ddpf.dwFlags & DDS_ALPHA)
		{
			if (8 == ddpf.dwRGBBitCount)
			{
				return DXGI_FORMAT_A8_UNORM;
			}
		}
		else if (ddpf.dwFlags & DDS_FOURCC)
		{
			if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.dwFourCC)
				return DXGI_FORMAT_BC1_UNORM;
			if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.dwFourCC)
				return DXGI_FORMAT_BC2_UNORM;
			if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.dwFourCC)
				return DXGI_FORMAT_BC3_UNORM;

			if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.dwFourCC)
				return DXGI_FORMAT_BC4_UNORM;
			if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.dwFourCC)
				return DXGI_FORMAT_BC4_SNORM;

			if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.dwFourCC)
				return DXGI_FORMAT_BC5_UNORM;
			if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.dwFourCC)
				return DXGI_FORMAT_BC5_SNORM;

			if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.dwFourCC)
				return DXGI_FORMAT_R8G8_B8G8_UNORM;
			if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.dwFourCC)
				return DXGI_FORMAT_G8R8_G8B8_UNORM;

			// Check for D3DFORMAT enums being set here
			switch (ddpf.dwFourCC)
			{
			case D3DFMT_A16B16G16R16: // 36
				return DXGI_FORMAT_R16G16B16A16_UNORM;

			case D3DFMT_Q16W16V16U16: // 110
				return DXGI_FORMAT_R16G16B16A16_SNORM;

			case D3DFMT_R16F: // 111
				return DXGI_FORMAT_R16_FLOAT;

			case D3DFMT_G16R16F: // 112
				return DXGI_FORMAT_R16G16_FLOAT;

			case D3DFMT_A16B16G16R16F: // 113
				return DXGI_FORMAT_R16G16B16A16_FLOAT;

			case D3DFMT_R32F: // 114
				return DXGI_FORMAT_R32_FLOAT;

			case D3DFMT_G32R32F: // 115
				return DXGI_FORMAT_R32G32_FLOAT;

			case D3DFMT_A32B32G32R32F: // 116
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	static D3DFORMAT GetD3D9Format(const DDS_PIXELFORMAT& ddpf)
	{
		if (ddpf.dwFlags & DDS_RGB)
		{
			switch (ddpf.dwRGBBitCount)
			{
			case 32:
				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
					return D3DFMT_A8R8G8B8;
				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
					return D3DFMT_X8R8G8B8;
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
					return D3DFMT_A8B8G8R8;
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000))
					return D3DFMT_X8B8G8R8;

				// Note that many common DDS reader/writers swap the
				// the RED/BLUE masks for 10:10:10:2 formats. We assumme
				// below that the 'correct' header mask is being used
				if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
					return D3DFMT_A2R10G10B10;
				if (ISBITMASK(0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000))
					return D3DFMT_A2B10G10R10;

				if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
					return D3DFMT_G16R16;
				if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
					return D3DFMT_R32F; // D3DX writes this out as a FourCC of 114
				break;

			case 24:
				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
					return D3DFMT_R8G8B8;
				break;

			case 16:
				if (ISBITMASK(0x0000f800, 0x000007e0, 0x0000001f, 0x00000000))
					return D3DFMT_R5G6B5;
				if (ISBITMASK(0x00007c00, 0x000003e0, 0x0000001f, 0x00008000))
					return D3DFMT_A1R5G5B5;
				if (ISBITMASK(0x00007c00, 0x000003e0, 0x0000001f, 0x00000000))
					return D3DFMT_X1R5G5B5;
				if (ISBITMASK(0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000))
					return D3DFMT_A4R4G4B4;
				if (ISBITMASK(0x00000f00, 0x000000f0, 0x0000000f, 0x00000000))
					return D3DFMT_X4R4G4B4;
				if (ISBITMASK(0x000000e0, 0x0000001c, 0x00000003, 0x0000ff00))
					return D3DFMT_A8R3G3B2;
				break;
			}
		}
		else if (ddpf.dwFlags & DDS_LUMINANCE)
		{
			if (8 == ddpf.dwRGBBitCount)
			{
				if (ISBITMASK(0x0000000f, 0x00000000, 0x00000000, 0x000000f0))
					return D3DFMT_A4L4;
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
					return D3DFMT_L8;
			}

			if (16 == ddpf.dwRGBBitCount)
			{
				if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
					return D3DFMT_L16;
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
					return D3DFMT_A8L8;
			}
		}
		else if (ddpf.dwFlags & DDS_ALPHA)
		{
			if (8 == ddpf.dwRGBBitCount)
			{
				return D3DFMT_A8;
			}
		}
		else if (ddpf.dwFlags & DDS_FOURCC)
		{
			if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.dwFourCC)
				return D3DFMT_DXT1;
			if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.dwFourCC)
				return D3DFMT_DXT2;
			if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.dwFourCC)
				return D3DFMT_DXT3;
			if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.dwFourCC)
				return D3DFMT_DXT4;
			if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.dwFourCC)
				return D3DFMT_DXT5;

			if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.dwFourCC)
				return D3DFMT_R8G8_B8G8;
			if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.dwFourCC)
				return D3DFMT_G8R8_G8B8;

			if (MAKEFOURCC('U', 'Y', 'V', 'Y') == ddpf.dwFourCC)
				return D3DFMT_UYVY;
			if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.dwFourCC)
				return D3DFMT_YUY2;

			// Check for D3DFORMAT enums being set here
			switch (ddpf.dwFourCC)
			{
			case D3DFMT_A16B16G16R16:
			case D3DFMT_Q16W16V16U16:
			case D3DFMT_R16F:
			case D3DFMT_G16R16F:
			case D3DFMT_A16B16G16R16F:
			case D3DFMT_R32F:
			case D3DFMT_G32R32F:
			case D3DFMT_A32B32G32R32F:
			case D3DFMT_CxV8U8:
				return (D3DFORMAT)ddpf.dwFourCC;
			}
		}

		return D3DFMT_UNKNOWN;
	}

	HRESULT Helper::CreateTextureFromDDS(ID3D11Device* pDev, DDS_HEADER* pHeader, __inout_bcount(BitSize) BYTE* pBitData, UINT BitSize, __out ID3D11ShaderResourceView** ppSRV, bool bSRGB)
	{
		HRESULT hr = S_OK;

		UINT iWidth = pHeader->dwWidth;
		UINT iHeight = pHeader->dwHeight;
		UINT iMipCount = pHeader->dwMipMapCount;
		if (0 == iMipCount)
			iMipCount = 1;

		// Bound miplevels (affects the memory usage below)
		if (iMipCount > D3D11_REQ_MIP_LEVELS)
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

		D3D11_TEXTURE2D_DESC desc;
		if ((pHeader->ddspf.dwFlags & DDS_FOURCC)
			&& (MAKEFOURCC('D', 'X', '1', '0') == pHeader->ddspf.dwFourCC))
		{
			DDS_HEADER_DXT10* d3d10ext = (DDS_HEADER_DXT10*)((char*)pHeader + sizeof(DDS_HEADER));

			// For now, we only support 2D textures
			if (d3d10ext->resourceDimension != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
				return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

			// Bound array sizes (affects the memory usage below)
			if (d3d10ext->arraySize > D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION)
				return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

			desc.ArraySize = d3d10ext->arraySize;
			desc.Format = d3d10ext->dxgiFormat;
		}
		else
		{
			desc.ArraySize = 1;
			desc.Format = GetDXGIFormat(pHeader->ddspf);

			if (pHeader->dwCubemapFlags != 0
				|| (pHeader->dwHeaderFlags & DDS_HEADER_FLAGS_VOLUME))
			{
				// For now only support 2D textures, not cubemaps or volumes
				return E_FAIL;
			}

			if (desc.Format == DXGI_FORMAT_UNKNOWN)
			{
				D3DFORMAT fmt = GetD3D9Format(pHeader->ddspf);

				// Swizzle some RGB to BGR common formats to be DXGI (1.0) supported
				switch (fmt)
				{
				case D3DFMT_X8R8G8B8:
				case D3DFMT_A8R8G8B8:
				{
					desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

					if (BitSize >= 3)
					{
						for (UINT i = 0; i < BitSize; i += 4)
						{
							BYTE a = pBitData[i];
							pBitData[i] = pBitData[i + 2];
							pBitData[i + 2] = a;
						}
					}
				}
				break;

				// Need more room to try to swizzle 24bpp formats
				// Could also try to expand 4bpp or 3:3:2 formats

				default:
					return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
				}
			}
		}

		if (bSRGB)
			desc.Format = MAKE_SRGB(desc.Format);

		// Create the texture
		desc.Width = iWidth;
		desc.Height = iHeight;
		desc.MipLevels = iMipCount;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA* pInitData = new D3D11_SUBRESOURCE_DATA[iMipCount * desc.ArraySize];
		if (!pInitData)
			return E_OUTOFMEMORY;

		UINT NumBytes = 0;
		UINT RowBytes = 0;
		UINT NumRows = 0;
		BYTE* pSrcBits = pBitData;

		UINT index = 0;
		for (UINT j = 0; j < desc.ArraySize; j++)
		{
			UINT w = iWidth;
			UINT h = iHeight;
			for (UINT i = 0; i < iMipCount; i++)
			{
				GetSurfaceInfo(w, h, desc.Format, &NumBytes, &RowBytes, &NumRows);
				pInitData[index].pSysMem = (void*)pSrcBits;
				pInitData[index].SysMemPitch = RowBytes;
				++index;

				pSrcBits += NumBytes;
				w = w >> 1;
				h = h >> 1;
				if (w == 0)
					w = 1;
				if (h == 0)
					h = 1;
			}
		}

		ID3D11Texture2D* pTex2D = NULL;
		hr = pDev->CreateTexture2D(&desc, pInitData, &pTex2D);
		if (SUCCEEDED(hr) && pTex2D)
		{
#if defined(DEBUG) || defined(PROFILE)
			pTex2D->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("DDSTextureLoader") - 1, "DDSTextureLoader");
#endif
			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
			ZeroMemory(&SRVDesc, sizeof(SRVDesc));
			SRVDesc.Format = desc.Format;
			SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = desc.MipLevels;
			hr = pDev->CreateShaderResourceView(pTex2D, &SRVDesc, ppSRV);
			SAFE_RELEASE(pTex2D);
		}

		SAFE_DELETE_ARRAY(pInitData);

		return hr;
	}

	HRESULT Helper::CreateDDSTextureFromFile(ID3D11Device * pDev, const WCHAR * szFileName, ID3D11ShaderResourceView ** ppSRV, bool sRGB)
	{
		if (!pDev || !szFileName || !ppSRV)
			return E_INVALIDARG;

		BYTE* pHeapData = NULL;
		DDS_HEADER* pHeader = NULL;
		BYTE* pBitData = NULL;
		UINT BitSize = 0;

		HRESULT hr = LoadTextureDataFromFile(szFileName, &pHeapData, &pHeader, &pBitData, &BitSize);
		if (FAILED(hr))
		{
			SAFE_DELETE_ARRAY(pHeapData);
			return hr;
		}

		hr = CreateTextureFromDDS(pDev, pHeader, pBitData, BitSize, ppSRV, sRGB);
		SAFE_DELETE_ARRAY(pHeapData);

#if defined(DEBUG) || defined(PROFILE)
		if (*ppSRV)
		{
			CHAR strFileA[MAX_PATH];
			WideCharToMultiByte(CP_ACP, 0, szFileName, -1, strFileA, MAX_PATH, NULL, FALSE);
			CHAR* pstrName = strrchr(strFileA, '\\');
			if (pstrName == NULL)
				pstrName = strFileA;
			else
				pstrName++;

			(*ppSRV)->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(pstrName), pstrName);
		}
#endif

		return hr;
	}
}
}
