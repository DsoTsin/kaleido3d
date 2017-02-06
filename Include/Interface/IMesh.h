#pragma once
#include "../Math/kGeometry.hpp"

enum class VtxFormat : uint32 
{
	POS3_F32 = 0,
	POS4_F32,
	POS3_F32_UV2_F32,
	POS3_F32_NOR3_F32,
	POS3_F32_NOR3_F32_UV2_F32,
	POS3_F32_NOR3_F32_UV2X2_F32,
	POS3_F32_NOR3_F32_UV2X3_F32,
	PER_INSTANCE // all components are seperated
};

enum class PrimType : uint32 
{
	POINTS = 0,
	TRIANGLES,
	TRIANGLE_STRIPS
};

struct K3D_API IMesh 
{
	virtual ~IMesh() {}
	virtual kMath::AABB GetBoundingBox() const = 0;
	virtual uint32		GetMaterialID() const = 0;
	virtual int			GetIndexNum() const = 0;
	virtual uint32*		GetIndexBuffer() const = 0;
	virtual PrimType	GetPrimType() const = 0;
	virtual int			GetVertexNum() const = 0;
	virtual VtxFormat	GetVertexFormat() const = 0;
	virtual float *		GetVertexBuffer() const = 0;
};