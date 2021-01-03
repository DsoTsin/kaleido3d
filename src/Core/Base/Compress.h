/**
 * MIT License
 *
 * Copyright (c) 2022 Zhou Qin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once
#ifndef __k3d_Compress_h__
#define __k3d_Compress_h__
#include "Types.h"

namespace k3d {
enum class CompressAlgorithm {
  ZLIB,
  LZ4,
  LZFSE,
};

K3D_CORE_API size_t compression_encode_buffer(U8 *destBuffer, size_t destSize,
                                              const U8 *srcBuffer,
                                              size_t srcSize,
                                              CompressAlgorithm algorithm);
K3D_CORE_API size_t compression_decode_buffer(U8 *destBuffer, size_t destSize,
                                              const U8 *srcBuffer,
                                              size_t srcSize,
                                              CompressAlgorithm algorithm);

} // namespace k3d

#endif