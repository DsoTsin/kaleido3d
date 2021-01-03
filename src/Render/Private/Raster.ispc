/**
 * MIT License
 *
 * Copyright (c) 2019 Zhou Qin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "Raster.isph"

#define EXPLICIT_VECTOR 1
#include "Vector.isph"

#define USE_TILED 1
#define USE_ATOMIC_DEPTH_WRITE 0

export uniform int ispc_lane_width() { return programCount; }

//@see simd::ClipTrisCache
struct ClipTrisCache {
  int32 trisNum;
  int32 alignedTrisNum;
  float *dataPtr;
};

inline int8 is_back_face(float x0, float x1, float x2, float y0, float y1,
                         float y2) {
  float v0m0 = x0 * y1;
  float v1m0 = x1 * y2;
  float v2m0 = x2 * y0;
  float v0m1 = x1 * y0;
  float v1m1 = x2 * y1;
  float v2m1 = x0 * y2;
  float v0s0 = v0m0 - v0m1;
  float v1s0 = v1m0 - v1m1;
  float v2s0 = v2m0 - v2m1;
  float area = v0s0 + v1s0 + v2s0;
  return area < 0.0f; // todo check hand
}

inline uint32 testVertex(float Vx, float Vy, float Vw, float WClip) {
  uint32 Flag = 0;
  if (Vw < WClip) {
    Flag |= ClipNear;
  }
  if (Vx < -Vw) {
    Flag |= ClipLeft;
  }
  if (Vx > Vw) {
    Flag |= ClipRight;
  }
  if (Vy < -Vw) {
    Flag |= ClipTop;
  }
  if (Vy > Vw) {
    Flag |= ClipBottom;
  }
  return Flag;
}

inline float mulX3(const float &X, const float &Y, const float &Z,
                   const uniform float M[16]) {
  return X * M[0] + Y * M[4] + Z * M[8] + M[12]; // 3 fma instructions
}

inline float mulY3(const float &X, const float &Y, const float &Z,
                   const uniform float M[16]) {
  return X * M[1] + Y * M[5] + Z * M[9] + M[13];
}

inline float mulZ3(const float &X, const float &Y, const float &Z,
                   const uniform float M[16]) {
  return X * M[2] + Y * M[6] + Z * M[10] + M[14];
}

inline float mulW3(const float &X, const float &Y, const float &Z,
                   const uniform float M[16]) {
  return X * M[3] + Y * M[7] + Z * M[11] + M[15];
}

export void clearDepth32(uniform float depth[], uniform float value,
                         uniform int32 width, uniform int32 height) {
  foreach (i = 0...width * height) { depth[i] = value; }
}

export void clearDepth16(uniform int16 depth[], uniform float value,
                         uniform int32 width, uniform int32 height) {
  uniform int16 hValue = float_to_half(value);
  foreach (i = 0...width * height) { depth[i] = hValue; }
}

/*
{
    v0x0,v0x1,v0x2,v0x3
    v0y0,v0y1,v0y2,v0y3
    v0z0,v0z1,v0z2,v0z3

    v1x0,v1x1,v1x2,v1x3
    v1y0,v1y1,v1y2,v1y3
    v1z0,v1z1,v1z2,v1z3

    v2x0,v2x1,v2x2,v2x3
    v2y0,v2y1,v2y2,v2y3
    v2z0,v2z1,v2z2,v2z3
} 4 triangles paralleled
!!! a gang has 9*programCount floats
*/
export void transformVerticesAndCullTriangles(
    uniform float inAoSTris[], const uniform int trisOffset,
    const uniform int numTris, const uniform float local2clip[16],
    const uniform SRenderParams *uniform params,
    uniform SRenderContextIspc *uniform ispcCtx, void *uniform userData) {
  uniform float *uniform ptr = (uniform float *uniform)inAoSTris;
  uniform float wclip = params->wClip;
  uniform float width = params->width;
  uniform float height = params->height;
  const uniform int gangFloats = programCount * 9;
  for (uniform int i = trisOffset; i < numTris; i += programCount) {
    // programIndex
    uniform int gangId = i / programCount;
    uniform float *uniform gangPtr = ptr + gangId * gangFloats;

    float v0x = streaming_load(gangPtr);
    // float v0x = gangPtr[programIndex];
    float v0y = streaming_load(gangPtr + programCount);
    float v0z = streaming_load(gangPtr + programCount * 2);

    float v1x = streaming_load(gangPtr + programCount * 3);
    float v1y = streaming_load(gangPtr + programCount * 4);
    float v1z = streaming_load(gangPtr + programCount * 5);

    float v2x = streaming_load(gangPtr + programCount * 6);
    float v2y = streaming_load(gangPtr + programCount * 7);
    float v2z = streaming_load(gangPtr + programCount * 8);

    float v0xc = mulX3(v0x, v0y, v0z, local2clip);
    float v0yc = mulY3(v0x, v0y, v0z, local2clip);
    float v0zc = mulZ3(v0x, v0y, v0z, local2clip);
    float v0wc = mulW3(v0x, v0y, v0z, local2clip);

    float v1xc = mulX3(v1x, v1y, v1z, local2clip);
    float v1yc = mulY3(v1x, v1y, v1z, local2clip);
    float v1zc = mulZ3(v1x, v1y, v1z, local2clip);
    float v1wc = mulW3(v1x, v1y, v1z, local2clip);

    float v2xc = mulX3(v2x, v2y, v2z, local2clip);
    float v2yc = mulY3(v2x, v2y, v2z, local2clip);
    float v2zc = mulZ3(v2x, v2y, v2z, local2clip);
    float v2wc = mulW3(v2x, v2y, v2z, local2clip);

    // test vertex clip flags
    uint32 v0f =
        testVertex(v0xc, v0yc, v0wc, wclip); // WClip = ViewProj.M[3][2];
    uint32 v1f = testVertex(v1xc, v1yc, v1wc, wclip);
    uint32 v2f = testVertex(v2xc, v2yc, v2wc, wclip);

    uint32 isFullyClipped = (v0f & v1f) & v2f;
    uint32 tf = (v0f | v1f) | v2f;

    float v0iwc = rcp(v0wc);
    float v0xs = (v0xc * v0iwc + 1.0f) * 0.5f * width;
    float v0ys = (v0yc * v0iwc + 1.0f) * 0.5f * height;

    float v1iwc = rcp(v1wc);
    float v1xs = (v1xc * v1iwc + 1.0f) * 0.5f * width;
    float v1ys = (v1yc * v1iwc + 1.0f) * 0.5f * height;

    float v2iwc = rcp(v2wc);
    float v2xs = (v2xc * v2iwc + 1.0f) * 0.5f * width;
    float v2ys = (v2yc * v2iwc + 1.0f) * 0.5f * height;

    float cz0 = v0zc * v0iwc;
    float cz1 = v1zc * v1iwc;
    float cz2 = v2zc * v2iwc;

    // calculate bounding box for triangle
    float minX = min(v0xs, min(v1xs, v2xs));
    float maxX = max(v0xs, max(v1xs, v2xs));

    minX = min(max(minX, 0.f), width - 1.f);
    maxX = min(max(maxX, 0.f), width - 1.f);

    float minY = min(v0ys, min(v1ys, v2ys));
    float maxY = max(v0ys, max(v1ys, v2ys));

    minY = min(max(minY, 0.f), height - 1.f);
    maxY = min(max(maxY, 0.f), height - 1.f);

    float minZ = min(min(cz0, cz1), cz2);
    float maxZ = max(max(cz0, cz1), cz2);

    // compute tri plane, considering winding order, Ax + By + Cz + D = 0
    // (scanline mode)

    // test triangle back facing [be aware of winding order]
    // int8 backFace = is_back_face(v0xc, v1xc, v2xc, v0yc, v1yc, v2yc);
    int8 frontFace = testFrontface(v0xc, v1xc, v2xc, v0yc, v1yc, v2yc);
    foreach_active(j) {
      // uniform int8 backfaced = extract(backFace, j);
      uniform int8 bFrontFace = extract(frontFace, j);
      uniform uint8 fullyClip = extract(isFullyClipped, j);
      if (bFrontFace && fullyClip == 0) { // visible
        uniform int triId = i + j;
        if (triId < numTris) {
          uniform SScreenTriangle screenTri;
          screenTri.sx0 = extract(v0xs, j);
          screenTri.sy0 = extract(v0ys, j);
          screenTri.sx1 = extract(v1xs, j);
          screenTri.sy1 = extract(v1ys, j);
          screenTri.sx2 = extract(v2xs, j);
          screenTri.sy2 = extract(v2ys, j);
          screenTri.minX = extract(minX, j);
          screenTri.maxX = extract(maxX, j);
          screenTri.minY = extract(minY, j);
          screenTri.maxY = extract(maxY, j);
          screenTri.cz0 = extract(cz0, j);
          screenTri.cz1 = extract(cz1, j);
          screenTri.cz2 = extract(cz2, j);
          screenTri.minDepth = extract(minZ, j);
          screenTri.maxDepth = extract(maxZ, j);
          screenTri.flag = extract(tf, j);
          ispcCtx->recv(ispcCtx->ctx, userData, triId, &screenTri);
        }
      } // backface test
    }
  }
}

void rasterTriangles32FImplReverseZ(uniform SScreenTriangle tris[],
                                    uniform int32 numTris,
                                    uniform SDepthBufferIspc32 *uniform depth) {
#if USE_TILED
  for (uniform int i = 0; i < numTris; i++) {
    uniform SScreenTriangle st = tris[i];
    uniform float area =
        edgeFunctionUniform(st.sx0, st.sy0, st.sx1, st.sy1, st.sx2, st.sy2);
    foreach_tiled(y = (uniform int32)st.minY...(uniform int32) st.maxY + 1,
                  x = (uniform int32)st.minX...(uniform int32) st.maxX + 1) {
      float pixelX = x + 0.5f;
      float pixelY = y + 0.5f;
      float w0 =
          edgeFunctionVarying(st.sx1, st.sy1, st.sx2, st.sy2, pixelX, pixelY);
      float w1 =
          edgeFunctionVarying(st.sx2, st.sy2, st.sx0, st.sy0, pixelX, pixelY);
      float w2 =
          edgeFunctionVarying(st.sx0, st.sy0, st.sx1, st.sy1, pixelX, pixelY);
      if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
        w0 /= area;
        w1 /= area;
        w2 /= area;
        float oneOverZ =
            1.0f / st.cz0 * w0 + 1.0f / st.cz0 * w1 + 1.0f / st.cz0 * w2;
        float z = 1.0f / oneOverZ;
// less equal
#pragma ignore warning(perf)
        depth->depth[y * depth->width + x] =
#pragma ignore warning(perf)
            max(z, depth->depth[y * depth->width + x]);
      }
    }
  }
#else
  foreach (i = 0...numTris) {
    SScreenTriangle st = tris[i];
    float area = edgeFunction(st.sx0, st.sy0, st.sx1, st.sy1, st.sx2, st.sy2);
    for (int32 x = (int32)st.minX; x < (int32)st.maxX; x++)
      for (int32 y = (int32)st.minY; y < (int32)st.maxY; y++) {
        float pixelX = x + 0.5f;
        float pixelY = y + 0.5f;
        float w0 = edgeFunction(st.sx1, st.sy1, st.sx2, st.sy2, pixelX, pixelY);
        float w1 = edgeFunction(st.sx2, st.sy2, st.sx0, st.sy0, pixelX, pixelY);
        float w2 = edgeFunction(st.sx0, st.sy0, st.sx1, st.sy1, pixelX, pixelY);
        if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
          w0 /= area;
          w1 /= area;
          w2 /= area;
          float oneOverZ =
              1.0f / st.cz0 * w0 + 1.0f / st.cz0 * w1 + 1.0f / st.cz0 * w2;
          float z = 1.0f / oneOverZ;
          /*if (z > depth_->load(x, y)) {
              depth_->store(x, y, z);
          }*/
        }
      }
  }
#endif
}

export void rasterTriangles32F(uniform SScreenTriangle tris[],
                               uniform int32 numTris, uniform uint8 useReverseZ,
                               uniform SDepthBufferIspc32 *uniform depth) {
  if (useReverseZ) {
    rasterTriangles32FImplReverseZ(tris, numTris, depth);
  }
}

export void rasterTriangles32FImplReverseZTiled(
    uniform SScreenTriangle tris[], uniform int32 trisIds[],
    uniform int32 numTris, uniform SBoxInt *uniform tileBox,
    uniform SDepthBufferIspc32 *uniform depth) {
  for (uniform int i = 0; i < numTris; i++) {
    uniform SScreenTriangle st = tris[trisIds[i]];
    uniform float area =
        edgeFunctionUniform(st.sx0, st.sy0, st.sx1, st.sy1, st.sx2, st.sy2);
    foreach_tiled(y = clamp((uniform int32)st.minY, tileBox->minY,
                            tileBox->maxY)... clamp((uniform int32)st.maxY + 1,
                                                    tileBox->minY,
                                                    tileBox->maxY),
                  x = (uniform int32)st.minX...(uniform int32) st.maxX + 1) {
      float pixelX = x + 0.5f;
      float pixelY = y + 0.5f;
      float w0 =
          edgeFunctionVarying(st.sx1, st.sy1, st.sx2, st.sy2, pixelX, pixelY);
      float w1 =
          edgeFunctionVarying(st.sx2, st.sy2, st.sx0, st.sy0, pixelX, pixelY);
      float w2 =
          edgeFunctionVarying(st.sx0, st.sy0, st.sx1, st.sy1, pixelX, pixelY);
      if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
        w0 /= area;
        w1 /= area;
        w2 /= area;
        float oneOverZ =
            1.0f / st.cz0 * w0 + 1.0f / st.cz0 * w1 + 1.0f / st.cz0 * w2;
        float z = 1.0f / oneOverZ;
        // less equal
#pragma ignore warning(perf)
        depth->depth[y * depth->width + x] =
#pragma ignore warning(perf)
            max(z, depth->depth[y * depth->width + x]);
      }
    }
  }
}

export void rasterTriangles32FImplReverseZTiled2(
    uniform SScreenTriangle tris[], uniform int32 numTris,
    uniform SBoxInt *uniform tileBox,
    uniform SDepthBufferIspc32 *uniform depth) {
  for (uniform int i = 0; i < numTris; i++) {
    uniform SScreenTriangle st = tris[i];
    uniform float area =
        edgeFunctionUniform(st.sx0, st.sy0, st.sx1, st.sy1, st.sx2, st.sy2);
    foreach_tiled(y = clamp((uniform int32)st.minY, tileBox->minY,
                            tileBox->maxY)... clamp((uniform int32)st.maxY + 1,
                                                    tileBox->minY,
                                                    tileBox->maxY),
                  x = (uniform int32)st.minX...(uniform int32) st.maxX + 1) {
      float pixelX = x + 0.5f;
      float pixelY = y + 0.5f;
      float w0 =
          edgeFunctionVarying(st.sx1, st.sy1, st.sx2, st.sy2, pixelX, pixelY);
      float w1 =
          edgeFunctionVarying(st.sx2, st.sy2, st.sx0, st.sy0, pixelX, pixelY);
      float w2 =
          edgeFunctionVarying(st.sx0, st.sy0, st.sx1, st.sy1, pixelX, pixelY);
      if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
        w0 /= area;
        w1 /= area;
        w2 /= area;
        float oneOverZ =
            1.0f / st.cz0 * w0 + 1.0f / st.cz0 * w1 + 1.0f / st.cz0 * w2;
        float z = 1.0f / oneOverZ;
        // less equal
#pragma ignore warning(perf)
        depth->depth[y * depth->width + x] =
#pragma ignore warning(perf)
            max(z, depth->depth[y * depth->width + x]);
      }
    }
  }
}

export void binTriangles(uniform SScreenTriangle tris[], uniform int32 numTris,
                         uniform SBoxInt *uniform tileBox,
                         uniform int32 numTiles, uniform FnBinTiles fnBinTiles,
                         void *uniform self) {
  foreach (i = 0...numTris) {
#pragma ignore warning(perf)
    SScreenTriangle tri = tris[i];
    for (uniform int tId = 0; tId < numTiles; tId++) {
#pragma ignore warning(perf)
      uniform SBoxInt box = tileBox[tId];
      if ((tri.minY < box.maxY && tri.minY >= box.minY) ||
          (tri.maxY <= box.maxY && tri.maxY >= box.minY) ||
          (tri.maxY >= box.maxY && tri.minY <= box.minY)) {
        foreach_active(j) {
          uniform int triId = extract(i, j);
          // if (triId + j < numTris) {
          fnBinTiles(self, triId, tId);
          //}
        }
      }
    }
  }
}

uniform SScreenTriangle extract(const SScreenTriangle& tri, uniform int j) {
    uniform SScreenTriangle screenTri;
    screenTri.sx0 = extract(tri.sx0, j);
    screenTri.sy0 = extract(tri.sy0, j);
    screenTri.sx1 = extract(tri.sx1, j);
    screenTri.sy1 = extract(tri.sy1, j);
    screenTri.sx2 = extract(tri.sx2, j);
    screenTri.sy2 = extract(tri.sy2, j);
    screenTri.minX = extract(tri.minX, j);
    screenTri.maxX = extract(tri.maxX, j);
    screenTri.minY = extract(tri.minY, j);
    screenTri.maxY = extract(tri.maxY, j);
    screenTri.cz0 = extract(tri.cz0, j);
    screenTri.cz1 = extract(tri.cz1, j);
    screenTri.cz2 = extract(tri.cz2, j);
    screenTri.minDepth = extract(tri.minDepth, j);
    screenTri.maxDepth = extract(tri.maxDepth, j);
    screenTri.flag = extract(tri.flag, j);
    return screenTri;
}

export void
binTriangles2TileImpl(uniform SScreenTriangle tris[], uniform int32 numTris,
                      uniform SBoxInt *uniform tileBox, uniform int32 numTiles,
                      uniform FnBinTriangle2Tiles fnBinTriangle2Tiles,
                      SRenderContext *uniform self) {
  foreach (i = 0...numTris) {
#pragma ignore warning(perf)
    SScreenTriangle tri = tris[i];
    for (uniform int tId = 0; tId < numTiles; tId++) {
#pragma ignore warning(perf)
      uniform SBoxInt box = tileBox[tId];
      if ((tri.minY < box.maxY && tri.minY >= box.minY) ||
          (tri.maxY <= box.maxY && tri.maxY >= box.minY) ||
          (tri.maxY >= box.maxY && tri.minY <= box.minY)) {
        foreach_active(j) {
          uniform int triId = extract(i, j);
          // if (triId + j < numTris) {
          uniform SScreenTriangle st = extract(tri, j);
          fnBinTriangle2Tiles(self, &st, tId);
          //}
        }
      }
    }
  }
}

/*
cuda code
for (int x = minc.x; x < maxc.x; ++x) {
    for (int y = minc.y; y < maxc.y; ++y) {
        glm::vec2 ndc = glm::vec2(
        (x * 2 - resolution.x) / resolution.x,
        (y * 2 - resolution.y) / resolution.y);
        glm::vec3 bary = calculateBarycentricCoordinate(tri, ndc);
        if (isBarycentricCoordInBounds(bary)) {
            int i = (int) ((resolution.y - y - 1) * resolution.x + x);

            float depthnew = getZAtCoordinate(bary, tri);
            fragment frag;
            frag.pn = glm::vec3(ndc, depthnew);
            frag.pw = baryinterp(bary, tri.v[0].pw, tri.v[1].pw, tri.v[2].pw);
            frag.c  = baryinterp(bary, tri.v[0].c , tri.v[1].c , tri.v[2].c );
            frag.nw = baryinterp(bary, tri.v[0].nw, tri.v[1].nw, tri.v[2].nw);

            int mydepth = (int) (depthnew * INT_MAX);
            atomicMin(&deptharray[i], mydepth);
            if (deptharray[i] == mydepth) {
                depthbuffer[i] = frag;
            }
        }
    }
}
*/

// depth from [1, 0]
void exportDepthBufferRGBAReverseZ(uniform float depthValues[],
                                   uniform int width, uniform int height,
                                   uniform uint8 cachePixels[]) {
  uniform float minDepth = 10000000.f;
  uniform float maxDepth = -10000000.f;

  uniform float infinity = floatbits(0x7f800000);
  uniform float infinity_neg = floatbits(0xff800000);
  foreach (i = 0...width * height) {
    float depth = /*1.0f - */ depthValues[i];
    if (depth != infinity_neg && depth != infinity) {
      minDepth = min(reduce_min(depth), minDepth);
      maxDepth = max(reduce_max(depth), maxDepth);
    }
  }

  foreach (i = 0...width * height) {
    float depth = /*1.0f -*/ depthValues[i];
    int32 scaledDepth =
#pragma ignore warning(perf)
        min(255, (int32)((depth - minDepth) / (maxDepth - minDepth) * 255 * 3));
    if (depth != infinity_neg && depth != infinity) {
#pragma ignore warning(perf)
      cachePixels[4 * i] = scaledDepth;
#pragma ignore warning(perf)
      cachePixels[4 * i + 1] = scaledDepth;
#pragma ignore warning(perf)
      cachePixels[4 * i + 2] = scaledDepth;
#pragma ignore warning(perf)
      cachePixels[4 * i + 3] = 255;
    } else {
#pragma ignore warning(perf)
      cachePixels[4 * i] = 255;
#pragma ignore warning(perf)
      cachePixels[4 * i + 1] = 0;
#pragma ignore warning(perf)
      cachePixels[4 * i + 2] = 0;
#pragma ignore warning(perf)
      cachePixels[4 * i + 3] = 128;
    }
  }
}

export void exportDepthBufferRGBA(uniform float depthValues[],
                                  uniform int width, uniform int height,
                                  uniform uint8 useReverseZ,
                                  uniform uint8 cachePixels[]) {
  if (useReverseZ) {
    exportDepthBufferRGBAReverseZ(depthValues, width, height, cachePixels);
  }
}