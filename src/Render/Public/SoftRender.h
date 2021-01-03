/**
 * MIT License
 *
 * Copyright (c) 2021 Zhou Qin
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

#if __cplusplus
#include "CoreMinimal.h"
#define unmasked
#define uniform
#define export extern "C"
#define programCount 8
#elif ISPC
#define class struct
#define public
#define private
#define U32 uint32
#define I32 int32
#endif

#if __cplusplus
namespace k3d {
#endif

enum ClipFlag {
  ClipNone = 0,
  ClipLeft = 1,
  ClipRight = 2,
  ClipTop = 4,
  ClipBottom = 8,
  ClipNear = 16,
  ClipDiscard = 32, // Polygon using this vertex should be discarded
};

struct SBoxInt {
  I32 minX;
  I32 minY;
  I32 maxX;
  I32 maxY;
};

struct SScreenTriangle {
  float sx0;
  float sy0;

  float sx1;
  float sy1;

  float sx2;
  float sy2;

  // aabb
  float minX;
  float minY;
  float maxX;
  float maxY;

  // clipSpaceZ
  float cz0;
  float cz1;
  float cz2;

  // clipZ bound
  float minDepth;
  float maxDepth;

#ifdef __cplusplus
  ClipFlag flag;
#else
  I32 flag;
#endif

#ifdef __cplusplus
  bool intersect(const SBoxInt &box) const;
#endif
};

class SoftRenderer {
public:
private:
};
#if __cplusplus
} // namespace k3d
#endif