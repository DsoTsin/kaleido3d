#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <stdint.h>
#include <assert.h>
#include <algorithm>
#include <memory>
#include "../Helper.h"

#pragma pack(push,1)
const uint32_t DDS_MAGIC = 0x20534444; // "DDS "
#pragma pack(pop)

struct handle_closer
{
  void operator()(HANDLE h)
  {
    if (h) CloseHandle (h);
  }
};

typedef public std::unique_ptr<void, handle_closer> ScopedHandle;

inline HANDLE safe_handle (HANDLE h)
{
  return (h==INVALID_HANDLE_VALUE) ? 0 : h;
}

#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA

#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

namespace DirectX
{

  struct DDS_PIXELFORMAT
  {
    uint32_t    size;
    uint32_t    flags;
    uint32_t    fourCC;
    uint32_t    RGBBitCount;
    uint32_t    RBitMask;
    uint32_t    GBitMask;
    uint32_t    BBitMask;
    uint32_t    ABitMask;
  };

  enum DDS_MISC_FLAGS2
  {
    DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
  };

  struct DDS_HEADER
  {
    uint32_t        size;
    uint32_t        flags;
    uint32_t        height;
    uint32_t        width;
    uint32_t        pitchOrLinearSize;
    uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
    uint32_t        mipMapCount;
    uint32_t        reserved1[11];
    DDS_PIXELFORMAT ddspf;
    uint32_t        caps;
    uint32_t        caps2;
    uint32_t        caps3;
    uint32_t        caps4;
    uint32_t        reserved2;
  };

  struct DDS_HEADER_DXT10
  {
    DXGI_FORMAT     dxgiFormat;
    uint32_t        resourceDimension;
    uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
    uint32_t        arraySize;
    uint32_t        miscFlags2;
  };

}


namespace k3d
{
  namespace d3d12
  {
    static DXGI_FORMAT GetDXGIFormat (const DirectX::DDS_PIXELFORMAT& ddpf)
    {
      if (ddpf.flags & DDS_RGB)
      {
        // Note that sRGB formats are written using the "DX10" extended header

        switch (ddpf.RGBBitCount)
        {
        case 32:
          if (ISBITMASK (0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
          {
            return DXGI_FORMAT_R8G8B8A8_UNORM;
          }

          if (ISBITMASK (0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
          {
            return DXGI_FORMAT_B8G8R8A8_UNORM;
          }

          if (ISBITMASK (0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
          {
            return DXGI_FORMAT_B8G8R8X8_UNORM;
          }

          // No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8

          // Note that many common DDS reader/writers (including D3DX) swap the
          // the RED/BLUE masks for 10:10:10:2 formats. We assume
          // below that the 'backwards' header mask is being used since it is most
          // likely written by D3DX. The more robust solution is to use the 'DX10'
          // header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

          // For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
          if (ISBITMASK (0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
          {
            return DXGI_FORMAT_R10G10B10A2_UNORM;
          }

          // No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

          if (ISBITMASK (0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
          {
            return DXGI_FORMAT_R16G16_UNORM;
          }

          if (ISBITMASK (0xffffffff, 0x00000000, 0x00000000, 0x00000000))
          {
            // Only 32-bit color channel format in D3D9 was R32F
            return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
          }
          break;

        case 24:
          // No 24bpp DXGI formats aka D3DFMT_R8G8B8
          break;

        case 16:
          if (ISBITMASK (0x7c00, 0x03e0, 0x001f, 0x8000))
          {
            return DXGI_FORMAT_B5G5R5A1_UNORM;
          }
          if (ISBITMASK (0xf800, 0x07e0, 0x001f, 0x0000))
          {
            return DXGI_FORMAT_B5G6R5_UNORM;
          }

          // No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5

          if (ISBITMASK (0x0f00, 0x00f0, 0x000f, 0xf000))
          {
            return DXGI_FORMAT_B4G4R4A4_UNORM;
          }

          // No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4

          // No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
          break;
        }
      }
      else if (ddpf.flags & DDS_LUMINANCE)
      {
        if (8==ddpf.RGBBitCount)
        {
          if (ISBITMASK (0x000000ff, 0x00000000, 0x00000000, 0x00000000))
          {
            return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
          }

          // No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4
        }

        if (16==ddpf.RGBBitCount)
        {
          if (ISBITMASK (0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
          {
            return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
          }
          if (ISBITMASK (0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
          {
            return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
          }
        }
      }
      else if (ddpf.flags & DDS_ALPHA)
      {
        if (8==ddpf.RGBBitCount)
        {
          return DXGI_FORMAT_A8_UNORM;
        }
      }
      else if (ddpf.flags & DDS_FOURCC)
      {
        if (MAKEFOURCC ('D', 'X', 'T', '1')==ddpf.fourCC)
        {
          return DXGI_FORMAT_BC1_UNORM;
        }
        if (MAKEFOURCC ('D', 'X', 'T', '3')==ddpf.fourCC)
        {
          return DXGI_FORMAT_BC2_UNORM;
        }
        if (MAKEFOURCC ('D', 'X', 'T', '5')==ddpf.fourCC)
        {
          return DXGI_FORMAT_BC3_UNORM;
        }

        // While pre-multiplied alpha isn't directly supported by the DXGI formats,
        // they are basically the same as these BC formats so they can be mapped
        if (MAKEFOURCC ('D', 'X', 'T', '2')==ddpf.fourCC)
        {
          return DXGI_FORMAT_BC2_UNORM;
        }
        if (MAKEFOURCC ('D', 'X', 'T', '4')==ddpf.fourCC)
        {
          return DXGI_FORMAT_BC3_UNORM;
        }

        if (MAKEFOURCC ('A', 'T', 'I', '1')==ddpf.fourCC)
        {
          return DXGI_FORMAT_BC4_UNORM;
        }
        if (MAKEFOURCC ('B', 'C', '4', 'U')==ddpf.fourCC)
        {
          return DXGI_FORMAT_BC4_UNORM;
        }
        if (MAKEFOURCC ('B', 'C', '4', 'S')==ddpf.fourCC)
        {
          return DXGI_FORMAT_BC4_SNORM;
        }

        if (MAKEFOURCC ('A', 'T', 'I', '2')==ddpf.fourCC)
        {
          return DXGI_FORMAT_BC5_UNORM;
        }
        if (MAKEFOURCC ('B', 'C', '5', 'U')==ddpf.fourCC)
        {
          return DXGI_FORMAT_BC5_UNORM;
        }
        if (MAKEFOURCC ('B', 'C', '5', 'S')==ddpf.fourCC)
        {
          return DXGI_FORMAT_BC5_SNORM;
        }

        // BC6H and BC7 are written using the "DX10" extended header

        if (MAKEFOURCC ('R', 'G', 'B', 'G')==ddpf.fourCC)
        {
          return DXGI_FORMAT_R8G8_B8G8_UNORM;
        }
        if (MAKEFOURCC ('G', 'R', 'G', 'B')==ddpf.fourCC)
        {
          return DXGI_FORMAT_G8R8_G8B8_UNORM;
        }

        if (MAKEFOURCC ('Y', 'U', 'Y', '2')==ddpf.fourCC)
        {
          return DXGI_FORMAT_YUY2;
        }

        // Check for D3DFORMAT enums being set here
        switch (ddpf.fourCC)
        {
        case 36: // D3DFMT_A16B16G16R16
          return DXGI_FORMAT_R16G16B16A16_UNORM;

        case 110: // D3DFMT_Q16W16V16U16
          return DXGI_FORMAT_R16G16B16A16_SNORM;

        case 111: // D3DFMT_R16F
          return DXGI_FORMAT_R16_FLOAT;

        case 112: // D3DFMT_G16R16F
          return DXGI_FORMAT_R16G16_FLOAT;

        case 113: // D3DFMT_A16B16G16R16F
          return DXGI_FORMAT_R16G16B16A16_FLOAT;

        case 114: // D3DFMT_R32F
          return DXGI_FORMAT_R32_FLOAT;

        case 115: // D3DFMT_G32R32F
          return DXGI_FORMAT_R32G32_FLOAT;

        case 116: // D3DFMT_A32B32G32R32F
          return DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
      }

      return DXGI_FORMAT_UNKNOWN;
    }

    static size_t BitsPerPixel (_In_ DXGI_FORMAT fmt)
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
      case DXGI_FORMAT_Y416:
      case DXGI_FORMAT_Y210:
      case DXGI_FORMAT_Y216:
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
      case DXGI_FORMAT_AYUV:
      case DXGI_FORMAT_Y410:
      case DXGI_FORMAT_YUY2:
        return 32;

      case DXGI_FORMAT_P010:
      case DXGI_FORMAT_P016:
        return 24;

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
      case DXGI_FORMAT_A8P8:
      case DXGI_FORMAT_B4G4R4A4_UNORM:
        return 16;

      case DXGI_FORMAT_NV12:
      case DXGI_FORMAT_420_OPAQUE:
      case DXGI_FORMAT_NV11:
        return 12;

      case DXGI_FORMAT_R8_TYPELESS:
      case DXGI_FORMAT_R8_UNORM:
      case DXGI_FORMAT_R8_UINT:
      case DXGI_FORMAT_R8_SNORM:
      case DXGI_FORMAT_R8_SINT:
      case DXGI_FORMAT_A8_UNORM:
      case DXGI_FORMAT_AI44:
      case DXGI_FORMAT_IA44:
      case DXGI_FORMAT_P8:
        return 8;

      case DXGI_FORMAT_R1_UNORM:
        return 1;

      case DXGI_FORMAT_BC1_TYPELESS:
      case DXGI_FORMAT_BC1_UNORM:
      case DXGI_FORMAT_BC1_UNORM_SRGB:
      case DXGI_FORMAT_BC4_TYPELESS:
      case DXGI_FORMAT_BC4_UNORM:
      case DXGI_FORMAT_BC4_SNORM:
        return 4;

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
        return 8;

      default:
        return 0;
      }
    }

    //--------------------------------------------------------------------------------------
    // Get surface information for a particular format
    //--------------------------------------------------------------------------------------
    static void GetSurfaceInfo (_In_ size_t width,
                                _In_ size_t height,
                                _In_ DXGI_FORMAT fmt,
                                _Out_opt_ size_t* outNumBytes,
                                _Out_opt_ size_t* outRowBytes,
                                _Out_opt_ size_t* outNumRows,
                                _Out_opt_ BOOL* outBC)
    {
      size_t numBytes = 0;
      size_t rowBytes = 0;
      size_t numRows = 0;

      bool bc = false;
      bool packed = false;
      bool planar = false;
      size_t bpe = 0;
      switch (fmt)
      {
      case DXGI_FORMAT_BC1_TYPELESS:
      case DXGI_FORMAT_BC1_UNORM:
      case DXGI_FORMAT_BC1_UNORM_SRGB:
      case DXGI_FORMAT_BC4_TYPELESS:
      case DXGI_FORMAT_BC4_UNORM:
      case DXGI_FORMAT_BC4_SNORM:
        bc = true;
        bpe = 8;
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
        bc = true;
        bpe = 16;
        break;

      case DXGI_FORMAT_R8G8_B8G8_UNORM:
      case DXGI_FORMAT_G8R8_G8B8_UNORM:
      case DXGI_FORMAT_YUY2:
        packed = true;
        bpe = 4;
        break;

      case DXGI_FORMAT_Y210:
      case DXGI_FORMAT_Y216:
        packed = true;
        bpe = 8;
        break;

      case DXGI_FORMAT_NV12:
      case DXGI_FORMAT_420_OPAQUE:
        planar = true;
        bpe = 2;
        break;

      case DXGI_FORMAT_P010:
      case DXGI_FORMAT_P016:
        planar = true;
        bpe = 4;
        break;
      }

      if (bc)
      {
        size_t numBlocksWide = 0;
        if (width>0)
        {
          numBlocksWide = std::max<size_t> (1, (width+3)/4);
        }
        size_t numBlocksHigh = 0;
        if (height>0)
        {
          numBlocksHigh = std::max<size_t> (1, (height+3)/4);
        }
        rowBytes = numBlocksWide * bpe;
        numRows = numBlocksHigh;
        numBytes = rowBytes * numBlocksHigh;
      }
      else if (packed)
      {
        rowBytes = ((width+1)>>1) * bpe;
        numRows = height;
        numBytes = rowBytes * height;
      }
      else if (fmt==DXGI_FORMAT_NV11)
      {
        rowBytes = ((width+3)>>2)*4;
        numRows = height*2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
        numBytes = rowBytes * numRows;
      }
      else if (planar)
      {
        rowBytes = ((width+1)>>1) * bpe;
        numBytes = (rowBytes * height)+((rowBytes * height+1)>>1);
        numRows = height+((height+1)>>1);
      }
      else
      {
        size_t bpp = BitsPerPixel (fmt);
        rowBytes = (width * bpp+7)/8; // round up to nearest byte
        numRows = height;
        numBytes = rowBytes * height;
      }

      if (outNumBytes)
      {
        *outNumBytes = numBytes;
      }
      if (outRowBytes)
      {
        *outRowBytes = rowBytes;
      }
      if (outNumRows)
      {
        *outNumRows = numRows;
      }
      if (outBC)
      {
        *outBC = bc;
      }
    }


    static HRESULT FillInitData (_In_ size_t width,
                                 _In_ size_t height,
                                 _In_ size_t depth,
                                 _In_ size_t mipCount,
                                 _In_ size_t arraySize,
                                 _In_ DXGI_FORMAT format,
                                 _In_ size_t maxsize,
                                 _In_ size_t bitSize,
                                 _In_reads_bytes_ (bitSize) const uint8_t* bitData,
                                 _Out_ size_t& twidth,
                                 _Out_ size_t& theight,
                                 _Out_ size_t& tdepth,
                                 _Out_ size_t& skipMip,
                                 _Out_ BOOL& bc,
                                 _Out_writes_ (mipCount*arraySize) D3D12_SUBRESOURCE_DATA* initData)
    {
      if (!bitData||!initData)
      {
        return E_POINTER;
      }

      skipMip = 0;
      twidth = 0;
      theight = 0;
      tdepth = 0;
      bc = false;

      size_t NumBytes = 0;
      size_t RowBytes = 0;
      const uint8_t* pSrcBits = bitData;
      const uint8_t* pEndBits = bitData+bitSize;

      size_t index = 0;
      for (size_t j = 0; j<arraySize; j++)
      {
        size_t w = width;
        size_t h = height;
        size_t d = depth;
        for (size_t i = 0; i<mipCount; i++)
        {
          GetSurfaceInfo (w,
                          h,
                          format,
                          &NumBytes,
                          &RowBytes,
                          nullptr,
                          &bc
                          );

          if ((mipCount<=1)||!maxsize||(w<=maxsize && h<=maxsize && d<=maxsize))
          {
            if (!twidth)
            {
              twidth = w;
              theight = h;
              tdepth = d;
            }

            assert (index<mipCount * arraySize);
            _Analysis_assume_ (index < mipCount * arraySize);
            initData[index].pData = (const void*)pSrcBits;
            initData[index].RowPitch = static_cast<UINT>(RowBytes);
            initData[index].SlicePitch = static_cast<UINT>(NumBytes);
            ++index;
          }
          else if (!j)
          {
            // Count number of skipped mipmaps (first item only)
            ++skipMip;
          }

          if (pSrcBits+(NumBytes*d) > pEndBits)
          {
            return HRESULT_FROM_WIN32 (ERROR_HANDLE_EOF);
          }

          pSrcBits += NumBytes * d;

          w = w>>1;
          h = h>>1;
          d = d>>1;
          if (w==0)
          {
            w = 1;
          }
          if (h==0)
          {
            h = 1;
          }
          if (d==0)
          {
            d = 1;
          }
        }
      }

      return (index>0) ? S_OK : E_FAIL;
    }

    HRESULT LoadTextureDataFromFile (_In_z_ const wchar_t* fileName,
                                     std::unique_ptr<uint8_t[]>& ddsData,
                                     DirectX::DDS_HEADER** header,
                                     uint8_t** bitData,
                                     size_t* bitSize
                                     )
    {
      using namespace DirectX;

      if (!header||!bitData||!bitSize)
      {
        return E_POINTER;
      }

      ScopedHandle hFile (safe_handle (CreateFile2 (fileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        OPEN_EXISTING,
        nullptr)));

      if (!hFile)
      {
        return HRESULT_FROM_WIN32 (GetLastError ());
      }

      // Get the file size
      LARGE_INTEGER FileSize = { 0 };

      FILE_STANDARD_INFO fileInfo;
      if (!GetFileInformationByHandleEx (hFile.get (), FileStandardInfo, &fileInfo, sizeof (fileInfo)))
      {
        return HRESULT_FROM_WIN32 (GetLastError ());
      }
      FileSize = fileInfo.EndOfFile;

      // File is too big for 32-bit allocation, so reject read
      if (FileSize.HighPart>0)
      {
        return E_FAIL;
      }

      // Need at least enough data to fill the header and magic number to be a valid DDS
      if (FileSize.LowPart<(sizeof (DDS_HEADER)+sizeof (uint32_t)))
      {
        return E_FAIL;
      }

      // create enough space for the file data
      ddsData.reset (new (std::nothrow) uint8_t[FileSize.LowPart]);
      if (!ddsData)
      {
        return E_OUTOFMEMORY;
      }

      // read the data in
      DWORD BytesRead = 0;
      if (!ReadFile (hFile.get (),
        ddsData.get (),
        FileSize.LowPart,
        &BytesRead,
        nullptr
        ))
      {
        return HRESULT_FROM_WIN32 (GetLastError ());
      }

      if (BytesRead<FileSize.LowPart)
      {
        return E_FAIL;
      }

      // DDS files always start with the same magic number ("DDS ")
      uint32_t dwMagicNumber = *(const uint32_t*)(ddsData.get ());
      if (dwMagicNumber!=DDS_MAGIC)
      {
        return E_FAIL;
      }

      auto hdr = reinterpret_cast<DDS_HEADER*>(ddsData.get ()+sizeof (uint32_t));

      // Verify header to validate DDS file
      if (hdr->size!=sizeof (DDS_HEADER)||
          hdr->ddspf.size!=sizeof (DDS_PIXELFORMAT))
      {
        return E_FAIL;
      }

      // Check for DX10 extension
      bool bDXT10Header = false;
      if ((hdr->ddspf.flags & DDS_FOURCC)&&
          (MAKEFOURCC ('D', 'X', '1', '0')==hdr->ddspf.fourCC))
      {
        // Must be long enough for both headers and magic value
        if (FileSize.LowPart<(sizeof (DDS_HEADER)+sizeof (uint32_t)+sizeof (DDS_HEADER_DXT10)))
        {
          return E_FAIL;
        }

        bDXT10Header = true;
      }

      // setup the pointers in the process request
      *header = hdr;
      ptrdiff_t offset = sizeof (uint32_t)+sizeof (DDS_HEADER)
        +(bDXT10Header ? sizeof (DDS_HEADER_DXT10) : 0);
      *bitData = ddsData.get ()+offset;
      *bitSize = FileSize.LowPart-offset;

      return S_OK;
    }

    //-----------------------------------------------------------------------------------------------------------------
    //Here are the D3D12 functions that were thrown together.
    //-----------------------------------------------------------------------------------------------------------------

    // Align uLocation to the next multiple of uAlign.
    UINT64 Align (UINT64 uLocation, UINT64 uAlign)
    {
      bool valid = true;
      if ((0==uAlign)||(uAlign & (uAlign-1)))
      {
        valid = false;
      }
      assert (valid);

      return ((uLocation+(uAlign-1)) & ~(uAlign-1));
    }

    static HRESULT CreateD3DResources (_In_ ID3D12Device* d3dDevice,
                                _In_ ID3D12GraphicsCommandList* cmdList,
                                _In_ UploadBufferWrapper* uploadBuffer,
                                _In_ uint32_t resDim,
                                _In_ size_t width,
                                _In_ size_t height,
                                _In_ size_t depth,
                                _In_ size_t mipCount,
                                _In_ BOOL bc,
                                _In_ size_t arraySize,
                                _In_ DXGI_FORMAT format,
                                _In_ D3D12_RESOURCE_STATES usage,
                                _In_ D3D12_RESOURCE_FLAGS miscFlags,
                                _In_reads_opt_ (mipCount*arraySize) D3D12_SUBRESOURCE_DATA* initData,
                                _Outptr_opt_ ID3D12Resource** resourceOut)
    {
      HRESULT hr;

      //describe the default heap resource that will be the final location on the GPU for this texture
      D3D12_RESOURCE_DESC desc;
      memset (&desc, 0, sizeof (desc));
      desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(resDim);
      desc.Width = static_cast<UINT16>(width);
      desc.Height = static_cast<UINT16>(height);
      desc.MipLevels = static_cast<UINT16>(mipCount);
      desc.DepthOrArraySize = static_cast<UINT16>(arraySize);
      desc.Format = format;
      desc.SampleDesc.Count = 1;
      desc.SampleDesc.Quality = 0;
      desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
      desc.Flags = miscFlags;
      //get the allocation size of this resource
      D3D12_RESOURCE_ALLOCATION_INFO resInfo = d3dDevice->GetResourceAllocationInfo (1, 1, &desc);

      //check if the upload buffer has enough space
      if ((uploadBuffer->pDataCur+resInfo.SizeInBytes)>uploadBuffer->pDataEnd)
      {
        return E_OUTOFMEMORY;
      }

	  D3D12_HEAP_PROPERTIES heapProps;
	  heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	  heapProps.CreationNodeMask = 1;
	  heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	  heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	  heapProps.VisibleNodeMask = 1;

      //create the default heap texture resource
      hr = d3dDevice->CreateCommittedResource (&heapProps,
		  D3D12_HEAP_FLAG_NONE,
		  &desc,
		  D3D12_RESOURCE_STATE_COPY_DEST,
		  nullptr,
		  IID_PPV_ARGS(resourceOut)
		);

      //local pointers to the uploadbuffer memory locations
      UINT8* pDataCur = uploadBuffer->pDataCur;
      UINT8* pDataBegin = uploadBuffer->pDataBegin;

      //Initialize the pitched width and height to the texture's width and height
      //since mip 0 is always the full dimensions of the texture.
      UINT pitchedWidth = (UINT)desc.Width;
      UINT pitchedHeight = (UINT)desc.Height;
      //copy the data for each subresource into the upload buffer
      SIZE_T numSubResources = mipCount;
      for (UINT i = 0; i<numSubResources; ++i)
      {
        //the data for this subresource.
        D3D12_SUBRESOURCE_DATA subResData = initData[i];

        //compressed BCx format height/width are always a multiple of 4 since that is their compression block size (4x4)
        if (bc)
        {
          pitchedWidth = (UINT)Align (pitchedWidth, 4);
          pitchedHeight = (UINT)Align (pitchedHeight, 4);
        }

        //describe the pitched subresource that will be written to the upload buffer.
		D3D12_SUBRESOURCE_FOOTPRINT pitchedDesc;
        memset (&pitchedDesc, 0, sizeof (pitchedDesc));
        pitchedDesc.Format = desc.Format;
        pitchedDesc.Width = pitchedWidth;
        pitchedDesc.Height = pitchedHeight;
        pitchedDesc.Depth = 1;
        //The required row pitch on the GPU can differ from that in the DDS, it must be a multiple of D3D12_TEXTURE_DATA_PITCH_ALIGNMENT.
        pitchedDesc.RowPitch = (UINT)Align (subResData.RowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

        //align the current position in the buffer on the GPU so the subresource satisfies D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT
        pDataCur = reinterpret_cast<UINT8*>(Align (reinterpret_cast<SIZE_T>(pDataCur), D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT));

        //describes the location and the contents of the subresource in upload buffer.
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedTexture2D;
        placedTexture2D.Offset = pDataCur-pDataBegin;
        placedTexture2D.Footprint = pitchedDesc;


        UINT8* srcData = (UINT8*)subResData.pData; //subresource data location on CPU.
        UINT8* dstData = pDataCur; //subresource data location on GPU.
        SIZE_T rowCount = subResData.SlicePitch/subResData.RowPitch; //number of rows to copy.
                                                                     //copy a row at a time to the location in the upload buffer used for this subresource.
        for (UINT y = 0; y<rowCount; y++)
        {
          //set the memory address of the dst/src row according their respective row pitches.
          UINT8* dstDataRow = dstData+y * pitchedDesc.RowPitch;
          UINT8* srcDataRow = srcData+y * subResData.RowPitch;

          //copy the row
          memcpy (dstDataRow, srcDataRow, subResData.RowPitch);
        }

        //after all rows are copied, increase the current position in the upload buffer by the subresource size.
        SIZE_T sizeInBytes = rowCount * pitchedDesc.RowPitch;
        pDataCur += sizeInBytes;

        //Now the subresource data is in the upload buffer, record the command to copy the data to the default heap resource's subresource.
        //dst location
        D3D12_TEXTURE_COPY_LOCATION dstTexture;
        memset (&dstTexture, 0, sizeof (dstTexture));
        dstTexture.pResource = *resourceOut;
        dstTexture.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstTexture.SubresourceIndex = i;

        //src location
        D3D12_TEXTURE_COPY_LOCATION srcTexture;
        memset (&srcTexture, 0, sizeof (srcTexture));
        srcTexture.pResource = uploadBuffer->pBuf.Get();
        srcTexture.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcTexture.PlacedFootprint = placedTexture2D;
        //record the copy operation that moves the upload buffer data into default heap resource.
        cmdList->CopyTextureRegion (
          &dstTexture,
          0, 0, 0,
          &srcTexture,
          NULL
          );

        //Finally divide the subresource height and width by 2 to give the dimensions of the next mip level in this loop.
        pitchedWidth = pitchedWidth/2;
        pitchedHeight = pitchedHeight/2;
      }

      //update the current position in the upload buffer for writing new data.
      uploadBuffer->pDataCur = pDataCur;

      return hr;
    }


    HRESULT CreateTextureFromDDS (_In_ ID3D12Device* d3dDevice, _In_ ID3D12GraphicsCommandList* cmdList, _In_ UploadBufferWrapper* uploadBuffer,
                                  _In_ const DirectX::DDS_HEADER* header, _In_reads_bytes_ (bitSize) const uint8_t* bitData,
                                  _In_ size_t bitSize, _Outptr_opt_ ID3D12Resource** resourceOut)
    {
      using namespace DirectX;
      HRESULT hr = S_OK;

      UINT width = header->width;
      UINT height = header->height;
      UINT depth = header->depth;

      uint32_t resDim = D3D12_RESOURCE_DIMENSION_UNKNOWN;
      UINT arraySize = 1;
      DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

      size_t mipCount = header->mipMapCount;
      if (0==mipCount)
      {
        mipCount = 1;
      }

      if ((header->ddspf.flags & DDS_FOURCC)&&
          (MAKEFOURCC ('D', 'X', '1', '0')==header->ddspf.fourCC))
      {
        auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>((const char*)header+sizeof (DDS_HEADER));

        arraySize = d3d10ext->arraySize;
        if (arraySize==0)
        {
          return HRESULT_FROM_WIN32 (ERROR_INVALID_DATA);
        }

        format = d3d10ext->dxgiFormat;
        resDim = d3d10ext->resourceDimension;
      }
      else
      {
        //d3d9-style dds files only support dimension tex_2d in this ghetto.
        format = GetDXGIFormat (header->ddspf);
        depth = 1;
        resDim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      }


      // Bound sizes (for security purposes we don't trust DDS file metadata larger than the D3D 11.x hardware requirements)
      if (mipCount>D3D12_REQ_MIP_LEVELS)
      {
        return HRESULT_FROM_WIN32 (ERROR_NOT_SUPPORTED);
      }

      // Create the texture
      std::unique_ptr<D3D12_SUBRESOURCE_DATA[]> initData (new (std::nothrow) D3D12_SUBRESOURCE_DATA[mipCount]);
      if (!initData)
      {
        return E_OUTOFMEMORY;
      }

      size_t skipMip = 0;
      size_t twidth = 0;
      size_t theight = 0;
      size_t tdepth = 0;
      size_t maxsize = 16384;
      BOOL bc = false;
      hr = FillInitData (width, height, depth, mipCount, arraySize, format, maxsize, bitSize, bitData,
                         twidth, theight, tdepth, skipMip, bc, initData.get ());

      if (SUCCEEDED (hr))
      {
		  D3D12_RESOURCE_STATES usage = D3D12_RESOURCE_STATE_GENERIC_READ;
		  D3D12_RESOURCE_FLAGS miscFlags = D3D12_RESOURCE_FLAG_NONE;

		  hr = CreateD3DResources(d3dDevice, cmdList, uploadBuffer, resDim, twidth, theight, tdepth, mipCount - skipMip, bc, arraySize,
			  format, usage, miscFlags, initData.get(), resourceOut);
      }

      return hr;
    }

  }
}
