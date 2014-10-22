#include "DDSHelper.h"
#include <Config/OSHeaders.h>
#include <GL/gl.h>
#ifdef K3DPLATFORM_OS_WIN
#include <GL/wglext.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#endif
#include <Core/k3dDbg.h>

#ifdef max
#undef max
#endif

#include <algorithm>

//copy from nv_dds
// surface description flags
const unsigned long DDSF_CAPS           = 0x00000001l;
const unsigned long DDSF_HEIGHT         = 0x00000002l;
const unsigned long DDSF_WIDTH          = 0x00000004l;
const unsigned long DDSF_PITCH          = 0x00000008l;
const unsigned long DDSF_PIXELFORMAT    = 0x00001000l;
const unsigned long DDSF_MIPMAPCOUNT    = 0x00020000l;
const unsigned long DDSF_LINEARSIZE     = 0x00080000l;
const unsigned long DDSF_DEPTH          = 0x00800000l;

// pixel format flags
const unsigned long DDSF_ALPHAPIXELS    = 0x00000001l;
const unsigned long DDSF_FOURCC         = 0x00000004l;
const unsigned long DDSF_RGB            = 0x00000040l;
const unsigned long DDSF_RGBA           = 0x00000041l;

// dwCaps1 flags
const unsigned long DDSF_COMPLEX         = 0x00000008l;
const unsigned long DDSF_TEXTURE         = 0x00001000l;
const unsigned long DDSF_MIPMAP          = 0x00400000l;

// dwCaps2 flags
const unsigned long DDSF_CUBEMAP         = 0x00000200l;
const unsigned long DDSF_CUBEMAP_POSITIVEX  = 0x00000400l;
const unsigned long DDSF_CUBEMAP_NEGATIVEX  = 0x00000800l;
const unsigned long DDSF_CUBEMAP_POSITIVEY  = 0x00001000l;
const unsigned long DDSF_CUBEMAP_NEGATIVEY  = 0x00002000l;
const unsigned long DDSF_CUBEMAP_POSITIVEZ  = 0x00004000l;
const unsigned long DDSF_CUBEMAP_NEGATIVEZ  = 0x00008000l;
const unsigned long DDSF_CUBEMAP_ALL_FACES  = 0x0000FC00l;
const unsigned long DDSF_VOLUME          = 0x00200000l;

#ifndef MAKEFOURCC
#define MAKEFOURCC(c0,c1,c2,c3) \
    ((uint32)(uint8)(c0)| \
    ((uint32)(uint8)(c1) << 8)| \
    ((uint32)(uint8)(c2) << 16)| \
    ((uint32)(uint8)(c3) << 24))
#endif

const uint32_t FOURCC_R8G8B8        = 20;
const uint32_t FOURCC_A8R8G8B8      = 21;
const uint32_t FOURCC_X8R8G8B8      = 22;
const uint32_t FOURCC_R5G6B5        = 23;
const uint32_t FOURCC_X1R5G5B5      = 24;
const uint32_t FOURCC_A1R5G5B5      = 25;
const uint32_t FOURCC_A4R4G4B4      = 26;
const uint32_t FOURCC_R3G3B2        = 27;
const uint32_t FOURCC_A8            = 28;
const uint32_t FOURCC_A8R3G3B2      = 29;
const uint32_t FOURCC_X4R4G4B4      = 30;
const uint32_t FOURCC_A2B10G10R10   = 31;
const uint32_t FOURCC_A8B8G8R8      = 32;
const uint32_t FOURCC_X8B8G8R8      = 33;
const uint32_t FOURCC_G16R16        = 34;
const uint32_t FOURCC_A2R10G10B10   = 35;
const uint32_t FOURCC_A16B16G16R16  = 36;

const uint32_t FOURCC_L8            = 50;
const uint32_t FOURCC_A8L8          = 51;
const uint32_t FOURCC_A4L4          = 52;
const uint32_t FOURCC_DXT1          = 0x31545844l; //(MAKEFOURCC('D','X','T','1'))
const uint32_t FOURCC_DXT2          = 0x32545844l; //(MAKEFOURCC('D','X','T','1'))
const uint32_t FOURCC_DXT3          = 0x33545844l; //(MAKEFOURCC('D','X','T','3'))
const uint32_t FOURCC_DXT4          = 0x34545844l; //(MAKEFOURCC('D','X','T','3'))
const uint32_t FOURCC_DXT5          = 0x35545844l; //(MAKEFOURCC('D','X','T','5'))
const uint32_t FOURCC_ATI1          = MAKEFOURCC('A','T','I','1');
const uint32_t FOURCC_ATI2          = MAKEFOURCC('A','T','I','2');
const uint32_t FOURCC_BC4U          = MAKEFOURCC('B','C','4','U');
const uint32_t FOURCC_BC4S          = MAKEFOURCC('B','C','4','S');
const uint32_t FOURCC_BC5S          = MAKEFOURCC('B','C','5','S');

const uint32_t FOURCC_D16_LOCKABLE  = 70;
const uint32_t FOURCC_D32           = 71;
const uint32_t FOURCC_D24X8         = 77;
const uint32_t FOURCC_D16           = 80;

const uint32_t FOURCC_D32F_LOCKABLE = 82;

const uint32_t FOURCC_L16           = 81;

const uint32_t FOURCC_DX10          = MAKEFOURCC('D','X','1','0');

// signed normalized formats
const uint32_t FOURCC_Q16W16V16U16  = 110;

// Floating point surface formats

// s10e5 formats (16-bits per channel)
const uint32_t FOURCC_R16F          = 111;
const uint32_t FOURCC_G16R16F       = 112;
const uint32_t FOURCC_A16B16G16R16F = 113;

// IEEE s23e8 formats (32-bits per channel)
const uint32_t FOURCC_R32F          = 114;
const uint32_t FOURCC_G32R32F       = 115;
const uint32_t FOURCC_A32B32G32R32F = 116;
const uint32_t FOURCC_UNKNOWN = 0;

struct DXTColBlock
{
    unsigned short col0;
    unsigned short col1;

    unsigned char row[4];
};

struct DXT3AlphaBlock
{
    unsigned short row[4];
};

struct DXT5AlphaBlock
{
    unsigned char alpha0;
    unsigned char alpha1;

    unsigned char row[6];
};

struct DDS_PIXELFORMAT
{
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwFourCC;
    unsigned long dwRGBBitCount;
    unsigned long dwRBitMask;
    unsigned long dwGBitMask;
    unsigned long dwBBitMask;
    unsigned long dwABitMask;
};

struct DDS_HEADER
{
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwHeight;
    unsigned long dwWidth;
    unsigned long dwPitchOrLinearSize;
    unsigned long dwDepth;
    unsigned long dwMipMapCount;
    unsigned long dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    unsigned long dwCaps1;
    unsigned long dwCaps2;
    unsigned long dwReserved2[3];
};


struct DDS_HEADER_10
{
    uint32_t dxgiFormat;  // check type
    uint32_t resourceDimension; //check type
    uint32_t miscFlag;
    uint32_t arraySize;
    uint32_t reserved;
};


bool DDSHelper::upperLeftOrigin = true;

DDSHelper::DDSHelper()
{
  m_ElementSize = 0;
}

DDSHelper::~DDSHelper()
{
  Clear();
}

bool DDSHelper::Load(uint8 *dataPtr, uint32 length)
{
  assert(dataPtr && length && "DDS data == nullptr!!");
  if (nullptr == dataPtr)
    return false;

  // read in file marker, make sure its a DDS file
  char filecode[4];
  memcpy(filecode, dataPtr, 4);
  dataPtr += 4;

  if (strncmp(filecode, "DDS ", 4) != 0)
  {
    kDebug("DDSHelper::Error, not a dds image!\n");
    return false;
  }

  // read in DDS header
  DDS_HEADER ddsh;
  DDS_HEADER_10 ddsh10;

  memcpy(&ddsh, dataPtr, sizeof(DDS_HEADER));
  dataPtr += sizeof(DDS_HEADER);

  // check if image is a volume texture
  if ((ddsh.dwCaps2 & DDSF_VOLUME) && (ddsh.dwDepth > 0))
    m_ImgDepth = ddsh.dwDepth;
  else
    m_ImgDepth = 0;

  if ((ddsh.ddspf.dwFlags & DDSF_FOURCC) && (ddsh.ddspf.dwFourCC == FOURCC_DX10)) {
    //This DDS file uses the DX10 header extension
    memcpy(&ddsh10, dataPtr, sizeof(DDS_HEADER_10));
    dataPtr+=sizeof(DDS_HEADER_10);
  }

  // There are flags that are supposed to mark these fields as valid, but some dds files don't set them properly
  m_ImgWidth  = ddsh.dwWidth;
  m_ImgHeight = ddsh.dwHeight;

  if (ddsh.dwFlags & DDSF_MIPMAPCOUNT) {
    m_MipLev = ddsh.dwMipMapCount;
  }
  else
    m_MipLev = 1;

  //check cube-map faces, the DX10 parser will override this
  if ( ddsh.dwCaps2 & DDSF_CUBEMAP && !(ddsh.ddspf.dwFlags & DDSF_FOURCC && ddsh.ddspf.dwFourCC == FOURCC_DX10)) {
    //this is a cubemap, count the faces
    m_ImgLayers = 0;
    m_ImgLayers += (ddsh.dwCaps2 & DDSF_CUBEMAP_POSITIVEX) ? 1 : 0;
    m_ImgLayers += (ddsh.dwCaps2 & DDSF_CUBEMAP_NEGATIVEX) ? 1 : 0;
    m_ImgLayers += (ddsh.dwCaps2 & DDSF_CUBEMAP_POSITIVEY) ? 1 : 0;
    m_ImgLayers += (ddsh.dwCaps2 & DDSF_CUBEMAP_NEGATIVEY) ? 1 : 0;
    m_ImgLayers += (ddsh.dwCaps2 & DDSF_CUBEMAP_POSITIVEZ) ? 1 : 0;
    m_ImgLayers += (ddsh.dwCaps2 & DDSF_CUBEMAP_NEGATIVEZ) ? 1 : 0;

    //check for a complete cubemap
    if ( (m_ImgLayers != 6) || (m_ImgWidth != m_ImgHeight) ) {
      return false;
    }

    m_IsCubeMap = true;
  }
  else {
    //not a cubemap
    m_ImgLayers = 1;
    m_IsCubeMap = false;
  }

  bool btcCompressed = false;
  int32_t bytesPerElement = 0;

  // figure out what the image format is
  if (ddsh.ddspf.dwFlags & DDSF_FOURCC)
  {
    switch(ddsh.ddspf.dwFourCC)
    {
    case FOURCC_DXT1:
      m_FillFmt = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
      m_InternalFmt = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
      m_DataType = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
      bytesPerElement = 8;
      btcCompressed = true;
      break;

    case FOURCC_DXT2:
    case FOURCC_DXT3:
      m_FillFmt = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
      m_InternalFmt = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
      m_DataType = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
      bytesPerElement = 16;
      btcCompressed = true;
      break;

    case FOURCC_DXT4:
    case FOURCC_DXT5:
      m_FillFmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
      m_InternalFmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
      m_DataType = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
      bytesPerElement = 16;
      btcCompressed = true;
      break;

    case FOURCC_ATI1:
      m_FillFmt = GL_COMPRESSED_RED_RGTC1;
      m_InternalFmt = GL_COMPRESSED_RED_RGTC1;
      m_DataType = GL_COMPRESSED_RED_RGTC1;
      bytesPerElement = 8;
      btcCompressed = true;
      break;

    case FOURCC_BC4U:
      m_FillFmt = GL_COMPRESSED_RED_RGTC1;
      m_InternalFmt = GL_COMPRESSED_RED_RGTC1;
      m_DataType = GL_COMPRESSED_RED_RGTC1;
      bytesPerElement = 8;
      btcCompressed = true;
      break;

    case FOURCC_BC4S:
      m_FillFmt = GL_COMPRESSED_SIGNED_RED_RGTC1;
      m_InternalFmt = GL_COMPRESSED_SIGNED_RED_RGTC1;
      m_DataType = GL_COMPRESSED_SIGNED_RED_RGTC1;
      bytesPerElement = 8;
      btcCompressed = true;
      break;

    case FOURCC_ATI2:
      m_FillFmt = GL_COMPRESSED_RG_RGTC2; //GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
      m_InternalFmt = GL_COMPRESSED_RG_RGTC2; //GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
      m_DataType = GL_COMPRESSED_RG_RGTC2; //GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
      bytesPerElement = 16;
      btcCompressed = true;
      break;

    case FOURCC_BC5S:
      m_FillFmt = GL_COMPRESSED_SIGNED_RG_RGTC2; //GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
      m_InternalFmt = GL_COMPRESSED_SIGNED_RG_RGTC2; //GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
      m_DataType = GL_COMPRESSED_SIGNED_RG_RGTC2; //GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
      bytesPerElement = 16;
      btcCompressed = true;
      break;

    case FOURCC_R8G8B8:
      m_FillFmt = GL_BGR;
      m_InternalFmt = GL_RGB8;
      m_DataType = GL_UNSIGNED_BYTE;
      bytesPerElement = 3;
      break;

    case FOURCC_A8R8G8B8:
      m_FillFmt = GL_BGRA;
      m_InternalFmt = GL_RGBA8;
      m_DataType = GL_UNSIGNED_BYTE;
      bytesPerElement = 4;
      break;

    case FOURCC_X8R8G8B8:
      m_FillFmt = GL_BGRA;
      m_InternalFmt = GL_RGB8;
      m_DataType = GL_UNSIGNED_INT_8_8_8_8;
      bytesPerElement = 4;
      break;

    case FOURCC_R5G6B5:
      m_FillFmt = GL_BGR;
      m_InternalFmt = GL_RGB5;
      m_DataType = GL_UNSIGNED_SHORT_5_6_5;
      bytesPerElement = 2;
      break;

    case FOURCC_A8:
      m_FillFmt = GL_ALPHA;
      m_InternalFmt = GL_ALPHA8;
      m_DataType = GL_UNSIGNED_BYTE;
      bytesPerElement = 1;
      break;

    case FOURCC_A2B10G10R10:
      m_FillFmt = GL_RGBA;
      m_InternalFmt = GL_RGB10_A2;
      m_DataType = GL_UNSIGNED_INT_10_10_10_2;
      bytesPerElement = 4;
      break;

    case FOURCC_A8B8G8R8:
      m_FillFmt = GL_RGBA;
      m_InternalFmt = GL_RGBA8;
      m_DataType = GL_UNSIGNED_BYTE;
      bytesPerElement = 4;
      break;

    case FOURCC_X8B8G8R8:
      m_FillFmt = GL_RGBA;
      m_InternalFmt = GL_RGB8;
      m_DataType = GL_UNSIGNED_INT_8_8_8_8;
      bytesPerElement = 4;
      break;

    case FOURCC_A2R10G10B10:
      m_FillFmt = GL_BGRA;
      m_InternalFmt = GL_RGB10_A2;
      m_DataType = GL_UNSIGNED_INT_10_10_10_2;
      bytesPerElement = 4;
      break;

    case FOURCC_G16R16:
      m_FillFmt = GL_RG;
      m_InternalFmt = GL_RG16;
      m_DataType = GL_UNSIGNED_SHORT;
      bytesPerElement = 4;
      break;

    case FOURCC_A16B16G16R16:
      m_FillFmt = GL_RGBA;
      m_InternalFmt = GL_RGBA16;
      m_DataType = GL_UNSIGNED_SHORT;
      bytesPerElement = 8;
      break;

    case FOURCC_L8:
      m_FillFmt = GL_LUMINANCE;
      m_InternalFmt = GL_LUMINANCE8;
      m_DataType = GL_UNSIGNED_BYTE;
      bytesPerElement = 1;
      break;

    case FOURCC_A8L8:
      m_FillFmt = GL_LUMINANCE_ALPHA;
      m_InternalFmt = GL_LUMINANCE8_ALPHA8;
      m_DataType = GL_UNSIGNED_BYTE;
      bytesPerElement = 2;
      break;

    case FOURCC_L16:
      m_FillFmt = GL_LUMINANCE;
      m_InternalFmt = GL_LUMINANCE16;
      m_DataType = GL_UNSIGNED_SHORT;
      bytesPerElement = 2;
      break;

    case FOURCC_Q16W16V16U16:
      m_FillFmt = GL_RGBA;
      m_InternalFmt = GL_RGBA16_SNORM;
      m_DataType = GL_SHORT;
      bytesPerElement = 8;
      break;

    case FOURCC_R16F:
      m_FillFmt = GL_RED;
      m_InternalFmt = GL_R16F;
      m_DataType = GL_HALF_FLOAT_ARB;
      bytesPerElement = 2;
      break;

    case FOURCC_G16R16F:
      m_FillFmt = GL_RG;
      m_InternalFmt = GL_RG16F;
      m_DataType = GL_HALF_FLOAT_ARB;
      bytesPerElement = 4;
      break;

    case FOURCC_A16B16G16R16F:
      m_FillFmt = GL_RGBA;
      m_InternalFmt = GL_RGBA16F_ARB;
      m_DataType = GL_HALF_FLOAT_ARB;
      bytesPerElement = 8;
      break;

    case FOURCC_R32F:
      m_FillFmt = GL_RED;
      m_InternalFmt = GL_R32F;
      m_DataType = GL_FLOAT;
      bytesPerElement = 4;
      break;

    case FOURCC_G32R32F:
      m_FillFmt = GL_RG;
      m_InternalFmt = GL_RG32F;
      m_DataType = GL_FLOAT;
      bytesPerElement = 8;
      break;

    case FOURCC_A32B32G32R32F:
      m_FillFmt = GL_RGBA;
      m_InternalFmt = GL_RGBA32F_ARB;
      m_DataType = GL_FLOAT;
      bytesPerElement = 16;
      break;

    case FOURCC_DX10:
    case FOURCC_UNKNOWN:
    case FOURCC_X1R5G5B5:
    case FOURCC_A1R5G5B5:
    case FOURCC_A4R4G4B4:
    case FOURCC_R3G3B2:
    case FOURCC_A8R3G3B2:
    case FOURCC_X4R4G4B4:
    case FOURCC_A4L4:
    case FOURCC_D16_LOCKABLE:
    case FOURCC_D32:
    case FOURCC_D24X8:
    case FOURCC_D16:
    case FOURCC_D32F_LOCKABLE:
      //these are unsupported for now
    default:
      return false;
    }
  }
  else if (ddsh.ddspf.dwFlags == DDSF_RGBA && ddsh.ddspf.dwRGBBitCount == 32)
  {
    if ( ddsh.ddspf.dwRBitMask == 0xff && ddsh.ddspf.dwGBitMask == 0xff00 && ddsh.ddspf.dwBBitMask == 0xff0000 && ddsh.ddspf.dwABitMask == 0xff000000 ) {
      //RGBA8 order
      m_FillFmt = GL_RGBA;
      m_InternalFmt = GL_RGBA8;
      m_DataType = GL_UNSIGNED_BYTE;
    }
    else if ( ddsh.ddspf.dwRBitMask == 0xff0000 && ddsh.ddspf.dwGBitMask == 0xff00 && ddsh.ddspf.dwBBitMask == 0xff && ddsh.ddspf.dwABitMask == 0xff000000 ) {
      //BGRA8 order
      m_FillFmt = GL_BGRA;
      m_InternalFmt = GL_RGBA8;
      m_DataType = GL_UNSIGNED_BYTE;
    }
    else if ( ddsh.ddspf.dwRBitMask == 0x3ff00000 && ddsh.ddspf.dwGBitMask == 0xffc00 && ddsh.ddspf.dwBBitMask == 0x3ff && ddsh.ddspf.dwABitMask == 0xc0000000 ) {
      //BGR10_A2 order
      m_FillFmt = GL_RGBA;
      m_InternalFmt = GL_RGB10_A2;
      m_DataType = GL_UNSIGNED_INT_2_10_10_10_REV; //GL_UNSIGNED_INT_10_10_10_2;
    }
    else if ( ddsh.ddspf.dwRBitMask == 0x3ff && ddsh.ddspf.dwGBitMask == 0xffc00 && ddsh.ddspf.dwBBitMask == 0x3ff00000 && ddsh.ddspf.dwABitMask == 0xc0000000 ) {
      //RGB10_A2 order
      m_FillFmt = GL_RGBA;
      m_InternalFmt = GL_RGB10_A2;
      m_DataType = GL_UNSIGNED_INT_10_10_10_2;
    }
    else {
      //we'll just guess BGRA8, because that is the common legacy format for improperly labeled files
      m_FillFmt = GL_BGRA;
      m_InternalFmt = GL_RGBA8;
      m_DataType = GL_UNSIGNED_BYTE;
    }
    bytesPerElement = 4;
  }
  else if (ddsh.ddspf.dwFlags == DDSF_RGB  && ddsh.ddspf.dwRGBBitCount == 32)
  {
    if ( ddsh.ddspf.dwRBitMask == 0xffff && ddsh.ddspf.dwGBitMask == 0xffff0000 && ddsh.ddspf.dwBBitMask == 0x00 && ddsh.ddspf.dwABitMask == 0x00 ) {
      m_FillFmt = GL_RG;
      m_InternalFmt = GL_RG16;
      m_DataType = GL_UNSIGNED_SHORT;
    }
    else if ( ddsh.ddspf.dwRBitMask == 0xff && ddsh.ddspf.dwGBitMask == 0xff00 && ddsh.ddspf.dwBBitMask == 0xff0000 && ddsh.ddspf.dwABitMask == 0x00 ) {
      m_FillFmt = GL_RGB;
      m_InternalFmt = GL_RGBA8;
      m_DataType = GL_UNSIGNED_INT_8_8_8_8;
    }
    else if ( ddsh.ddspf.dwRBitMask == 0xff0000 && ddsh.ddspf.dwGBitMask == 0xff00 && ddsh.ddspf.dwBBitMask == 0xff && ddsh.ddspf.dwABitMask == 0x00 ) {
      m_FillFmt = GL_BGR;
      m_InternalFmt = GL_RGBA8;
      m_DataType = GL_UNSIGNED_INT_8_8_8_8;
    }
    else {
      // probably a poorly labeled file with BGRX semantics
      m_FillFmt = GL_BGR;
      m_InternalFmt = GL_RGBA8;
      m_DataType = GL_UNSIGNED_INT_8_8_8_8;
    }
    bytesPerElement = 4;
  }
  else if (ddsh.ddspf.dwFlags == DDSF_RGB  && ddsh.ddspf.dwRGBBitCount == 24)
  {
    m_FillFmt = GL_BGR;
    m_InternalFmt = GL_RGB8;
    m_DataType = GL_UNSIGNED_BYTE;
    bytesPerElement = 3;
  }
  // these cases revived from NVHHDDS...
  else if ((ddsh.ddspf.dwRGBBitCount == 16) &&
           (ddsh.ddspf.dwRBitMask == 0x0000F800) &&
           (ddsh.ddspf.dwGBitMask == 0x000007E0) &&
           (ddsh.ddspf.dwBBitMask == 0x0000001F) &&
           (ddsh.ddspf.dwABitMask == 0x00000000))
  {
    // We support D3D's R5G6B5, which is actually RGB in linear
    // memory.  It is equivalent to GL's GL_UNSIGNED_SHORT_5_6_5
    m_FillFmt = GL_BGR;
    m_InternalFmt = GL_RGB5;
    m_DataType = GL_UNSIGNED_SHORT_5_6_5;
    bytesPerElement = 2;
  }
  else if ((ddsh.ddspf.dwRGBBitCount == 8) &&
           (ddsh.ddspf.dwRBitMask == 0x00000000) &&
           (ddsh.ddspf.dwGBitMask == 0x00000000) &&
           (ddsh.ddspf.dwBBitMask == 0x00000000) &&
           (ddsh.ddspf.dwABitMask == 0x000000FF))
  {
    // We support D3D's A8
    m_FillFmt = GL_ALPHA;
    m_InternalFmt = GL_ALPHA8;
    m_DataType = GL_UNSIGNED_BYTE;
    bytesPerElement = 1;
  }
  else if ((ddsh.ddspf.dwRGBBitCount == 8) &&
           (ddsh.ddspf.dwRBitMask == 0x000000FF) &&
           (ddsh.ddspf.dwGBitMask == 0x00000000) &&
           (ddsh.ddspf.dwBBitMask == 0x00000000) &&
           (ddsh.ddspf.dwABitMask == 0x00000000))
  {
    // We support D3D's L8 (flagged as 8 bits of red only)
    m_FillFmt = GL_LUMINANCE;
    m_InternalFmt = GL_LUMINANCE8;
    m_DataType = GL_UNSIGNED_BYTE;
    bytesPerElement = 1;
  }
  else if ((ddsh.ddspf.dwRGBBitCount == 16) &&
           (((ddsh.ddspf.dwRBitMask == 0x000000FF) &&
             (ddsh.ddspf.dwGBitMask == 0x00000000) &&
             (ddsh.ddspf.dwBBitMask == 0x00000000) &&
             (ddsh.ddspf.dwABitMask == 0x0000FF00)) ||
            ((ddsh.ddspf.dwRBitMask == 0x000000FF) && // GIMP header for L8A8
             (ddsh.ddspf.dwGBitMask == 0x000000FF) &&  // Ugh
             (ddsh.ddspf.dwBBitMask == 0x000000FF) &&
             (ddsh.ddspf.dwABitMask == 0x0000FF00)))
           )
  {
    // We support D3D's A8L8 (flagged as 8 bits of red and 8 bits of alpha)
    m_FillFmt = GL_LUMINANCE_ALPHA;
    m_InternalFmt = GL_LUMINANCE8_ALPHA8;
    m_DataType = GL_UNSIGNED_BYTE;
    bytesPerElement = 2;
  }
  // else fall back to L8 generic handling if capable.
  else if (ddsh.ddspf.dwRGBBitCount == 8)
  {
    m_FillFmt = GL_LUMINANCE;
    m_InternalFmt = GL_LUMINANCE8;
    m_DataType = GL_UNSIGNED_BYTE;
    bytesPerElement = 1;
  }
  // else, we can't decode this file... :-(
  else
  {
    // TODO better error?
    kDebug("! Error decoding DDS file.");
    return false;
  }

  m_ElementSize = bytesPerElement;

  m_ImgData.clear();

  for (uint32 face = 0; face < m_ImgLayers; face++) {
    int w = m_ImgWidth, h = m_ImgHeight, d = (m_ImgDepth) ? m_ImgDepth : 1;
    for (uint32 level = 0; level < m_ImgLayers; level++) {
      int32_t bw = (btcCompressed) ? (w+3)/4 : w;
      int32_t bh = (btcCompressed) ? (h+3)/4 : h;
      int32_t size = bw*bh*d*bytesPerElement;

      uint8 *pixels = new uint8[size];

      memcpy(pixels, dataPtr, size);
      dataPtr += size;

      m_ImgData.push_back(pixels);

      if ( DDSHelper::upperLeftOrigin && !m_IsCubeMap)
        flipSurface( pixels, w, h, d);

      //reduce mip sizes
      w = ( w > 1) ? w >> 1 : 1;
      h = ( h > 1) ? h >> 1 : 1;
      d = ( d > 1) ? d >> 1 : 1;
    }
  }

  return true;
}

bool DDSHelper::IsCompressed() const
{
  switch(m_FillFmt) {
      case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
      case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
      case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
      case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
      case GL_COMPRESSED_RG_RGTC2:
      case GL_COMPRESSED_RED_RGTC1:
      case GL_COMPRESSED_SIGNED_RG_RGTC2:
      case GL_COMPRESSED_SIGNED_RED_RGTC1:
          return true;
  }
  return false;
}

void DDSHelper::Clear()
{
  if(m_ImgData.empty()) return;
  for (ByteVec::iterator it = m_ImgData.begin(); it != m_ImgData.end(); it++) {
    delete []*it;
  }
  m_ImgData.clear();
}

void DDSHelper::flipSurface(uint8 *surf, int32 width, int32 height, int32 depth)
{
  uint32 lineSize;

  depth = (depth) ? depth : 1;

  if (!IsCompressed()) {
    lineSize = m_ElementSize * width;
    uint32 sliceSize = lineSize * height;

    uint8 *tempBuf = new uint8[lineSize];

    for ( int ii = 0; ii < depth; ii++) {
      uint8 *top = surf + ii*sliceSize;
      uint8 *bottom = top + (sliceSize - lineSize);

      for ( int jj = 0; jj < (height >> 1); jj++) {
        memcpy( tempBuf, top, lineSize);
        memcpy( top, bottom, lineSize);
        memcpy( bottom, tempBuf, lineSize);

        top += lineSize;
        bottom -= lineSize;
      }
    }

    delete []tempBuf;
  }
  else
  {
    void (*flipblocks)(uint8_t*, uint32_t);
    width = (width + 3) / 4;
    height = (height + 3) / 4;
    uint32_t blockSize = 0;

    switch (m_FillFmt)
    {
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      blockSize = 8;
      flipblocks = &DDSHelper::flipBlocksDxtc1;
      break;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      blockSize = 16;
      flipblocks = &DDSHelper::flipBlocksDxtc3;
      break;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      blockSize = 16;
      flipblocks = &DDSHelper::flipBlocksDxtc5;
      break;
    case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
    case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
    case GL_COMPRESSED_RED_RGTC1:
    case GL_COMPRESSED_SIGNED_RED_RGTC1:
      blockSize = 8;
      flipblocks = &DDSHelper::flipBlocksBc4;
      break;
    case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
    case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
    case GL_COMPRESSED_RG_RGTC2:
    case GL_COMPRESSED_SIGNED_RG_RGTC2:
      blockSize = 16;
      flipblocks = &DDSHelper::flipBlocksBc5;
      break;
    default:
      return;
    }

    lineSize = width * blockSize;
    uint8 *tempBuf = new uint8[lineSize];

    uint8 *top = surf;
    uint8 *bottom = surf + (height-1) * lineSize;

    for (uint32 j = 0; j < std::max( (uint32)height >> 1, (uint32)1); j++)
    {
      if (top == bottom)
      {
        flipblocks(top, width);
        break;
      }

      flipblocks(top, width);
      flipblocks(bottom, width);

      memcpy( tempBuf, top, lineSize);
      memcpy( top, bottom, lineSize);
      memcpy( bottom, tempBuf, lineSize);

      top += lineSize;
      bottom -= lineSize;
    }
    delete []tempBuf;
  }
}

void DDSHelper::flipBlocksDxtc1(uint8 *ptr, uint32 numBlocks)
{
  DXTColBlock *curblock = (DXTColBlock*)ptr;
  uint8 temp;

  for (uint32 i = 0; i < numBlocks; i++) {
    temp = curblock->row[0];
    curblock->row[0] = curblock->row[3];
    curblock->row[3] = temp;
    temp = curblock->row[1];
    curblock->row[1] = curblock->row[2];
    curblock->row[2] = temp;

    curblock++;
  }
}

void DDSHelper::flipBlocksDxtc3(uint8 *ptr, uint32 numBlocks)
{
  DXTColBlock *curblock = (DXTColBlock*)ptr;
  DXT3AlphaBlock *alphablock;
  uint16 tempS;
  uint8 tempB;

  for (uint32 i = 0; i < numBlocks; i++)
  {
    alphablock = (DXT3AlphaBlock*)curblock;

    tempS = alphablock->row[0];
    alphablock->row[0] = alphablock->row[3];
    alphablock->row[3] = tempS;
    tempS = alphablock->row[1];
    alphablock->row[1] = alphablock->row[2];
    alphablock->row[2] = tempS;

    curblock++;

    tempB = curblock->row[0];
    curblock->row[0] = curblock->row[3];
    curblock->row[3] = tempB;
    tempB = curblock->row[1];
    curblock->row[1] = curblock->row[2];
    curblock->row[2] = tempB;

    curblock++;
  }
}


//
// flip a DXT5 alpha block
////////////////////////////////////////////////////////////
void flip_dxt5_alpha(DXT5AlphaBlock *block)
{
  uint8 gBits[4][4];

  const uint32 mask = 0x00000007;          // bits = 00 00 01 11
  uint32 bits = 0;
  memcpy(&bits, &block->row[0], sizeof(uint8) * 3);

  gBits[0][0] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[0][1] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[0][2] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[0][3] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[1][0] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[1][1] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[1][2] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[1][3] = (uint8_t)(bits & mask);

  bits = 0;
  memcpy(&bits, &block->row[3], sizeof(uint8_t) * 3);

  gBits[2][0] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[2][1] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[2][2] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[2][3] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[3][0] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[3][1] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[3][2] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[3][3] = (uint8_t)(bits & mask);

  // clear existing alpha bits
  memset(block->row, 0, sizeof(uint8) * 6);

  uint32 *pBits = ((uint32*) &(block->row[0]));

  *pBits = *pBits | (gBits[3][0] << 0);
  *pBits = *pBits | (gBits[3][1] << 3);
  *pBits = *pBits | (gBits[3][2] << 6);
  *pBits = *pBits | (gBits[3][3] << 9);

  *pBits = *pBits | (gBits[2][0] << 12);
  *pBits = *pBits | (gBits[2][1] << 15);
  *pBits = *pBits | (gBits[2][2] << 18);
  *pBits = *pBits | (gBits[2][3] << 21);

  pBits = ((uint32*) &(block->row[3]));

  *pBits = *pBits | (gBits[1][0] << 0);
  *pBits = *pBits | (gBits[1][1] << 3);
  *pBits = *pBits | (gBits[1][2] << 6);
  *pBits = *pBits | (gBits[1][3] << 9);

  *pBits = *pBits | (gBits[0][0] << 12);
  *pBits = *pBits | (gBits[0][1] << 15);
  *pBits = *pBits | (gBits[0][2] << 18);
  *pBits = *pBits | (gBits[0][3] << 21);
}

void DDSHelper::flipBlocksDxtc5(uint8 *ptr, uint32 numBlocks)
{
  DXTColBlock *curblock = (DXTColBlock*)ptr;
  DXT5AlphaBlock *alphablock;
  uint8 temp;

  for (uint32 i = 0; i < numBlocks; i++)
  {
    alphablock = (DXT5AlphaBlock*)curblock;

    flip_dxt5_alpha(alphablock);

    curblock++;

    temp = curblock->row[0];
    curblock->row[0] = curblock->row[3];
    curblock->row[3] = temp;
    temp = curblock->row[1];
    curblock->row[1] = curblock->row[2];
    curblock->row[2] = temp;

    curblock++;
  }
}

void DDSHelper::flipBlocksBc4(uint8 *ptr, uint32 numBlocks)
{
  DXT5AlphaBlock *alphablock = (DXT5AlphaBlock*)ptr;

  for (uint32 i = 0; i < numBlocks; i++)
  {
    flip_dxt5_alpha(alphablock);

    alphablock++;
  }
}

void DDSHelper::flipBlocksBc5(uint8 *ptr, uint32 numBlocks)
{
  DXT5AlphaBlock *alphablock = (DXT5AlphaBlock*)ptr;

  for (uint32 i = 0; i < numBlocks; i++)
  {
    flip_dxt5_alpha(alphablock);

    alphablock++;
    flip_dxt5_alpha(alphablock);

    alphablock++;
  }
}
