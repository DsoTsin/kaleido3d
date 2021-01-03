#include "CoreMinimal.h"
#include "Compress.h"

#include "Compressors/lz4/lz4.h"
#include "Compressors/lzfse/lzfse.h"
#include "zlib.h"

namespace k3d {
size_t compression_encode_buffer(U8 *destBuffer, size_t destSize,
                                 const U8 *srcBuffer, size_t srcSize,
                                 CompressAlgorithm algorithm) {
  switch (algorithm) {
  case CompressAlgorithm::LZ4:
    return LZ4_compress_default((const char *)srcBuffer, (char *)destBuffer,
                                srcSize, destSize);
  case CompressAlgorithm::LZFSE:
    return lzfse_encode_buffer(destBuffer, destSize, srcBuffer, srcSize, NULL);
  case CompressAlgorithm::ZLIB: {
    uLongf destSz = destSize;
    auto ret = compress2(destBuffer, &destSz, srcBuffer, srcSize, 5);
    return ret == Z_OK ? destSz : 0;
  }
  default:
    return 0;
  }
}

size_t compression_decode_buffer(U8 *destBuffer, size_t destSize,
                                 const U8 *srcBuffer, size_t srcSize,
                                 CompressAlgorithm algorithm) {
  switch (algorithm) {
  case CompressAlgorithm::LZ4:
    return LZ4_decompress_safe((const char *)srcBuffer, (char *)destBuffer,
                               srcSize, destSize);
  case CompressAlgorithm::LZFSE:
    return lzfse_decode_buffer(destBuffer, destSize, srcBuffer, srcSize, NULL);
  case CompressAlgorithm::ZLIB: {
    uLongf destSz = destSize;
    auto ret = uncompress(destBuffer, &destSz, srcBuffer, srcSize);
    return ret == Z_OK ? destSz : 0;
  }
  default:
    return 0;
  }
}
} // namespace k3d