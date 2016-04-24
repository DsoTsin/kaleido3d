//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+
#if _MSC_VER >= 1700
#pragma warning( disable: 4005 )
#endif

#include <stdio.h>

#include "DX11ViewportRenderer.h"

#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MMatrix.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MBoundingBox.h>
#include <maya/MImage.h>
#include <maya/MDrawTraversal.h>
#include <maya/MGeometryManager.h>
#include <maya/MGeometry.h>
#include <maya/MGeometryData.h>
#include <maya/MGeometryPrimitive.h>
#include <maya/MNodeMessage.h> // For monitor geometry list
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnSet.h>
#include <maya/MFnNumericData.h>
#include <maya/MItDependencyGraph.h>

#include <stdio.h>

#if defined(DX11_SUPPORTED)

//
// Populate a D3DGeometry object from a Maya mesh
//
bool D3DGeometry::Populate( const MDagPath& dagPath, ID3D11Device* D3D)
{
	HRESULT hr;

	Release();
	MFnMesh mesh( dagPath.node());

	// Figure out texturing
	//
	MString pn = dagPath.fullPathName();
	//printf("Convert shape %s\n", pn.asChar());
	bool haveTexture = false;
	int	numUVsets = mesh.numUVSets();
	MString uvSetName;
	MObjectArray textures;
	if (numUVsets > 0)
	{
		mesh.getCurrentUVSetName( uvSetName );
		// Always send down uvs for now, since we don't dirty the populate
		// based on material texture connection.
		//
		//MStatus status = mesh.getAssociatedUVSetTextures(uvSetName, textures);
		//if (status == MS::kSuccess && textures.length())
		int numCoords = mesh.numUVs( uvSetName ); 
		if (numCoords > 0)
		{
			haveTexture = true;
		}
	}

	bool haveColors = false;
	int	numColors = mesh.numColorSets();
	MString colorSetName;
	if (numColors > 0)
	{
		haveColors = true;
		mesh.getCurrentColorSetName(colorSetName);
	}

	bool useNormals = true;

	// Setup our requirements needs.
	MGeometryRequirements requirements;
	requirements.addPosition();
	if (useNormals)
		requirements.addNormal();
	if (haveTexture)
		requirements.addTexCoord( uvSetName );
	if (haveColors)
		requirements.addColor( colorSetName );

	// Test for tangents and binormals
	bool testBinormal = false;
	if (testBinormal)
		requirements.addBinormal( uvSetName );
	bool testTangent= false;
	if (testTangent)
		requirements.addTangent( uvSetName );

	MGeometry geom = MGeometryManager::getGeometry( dagPath, requirements, NULL );

	unsigned int numPrims = geom.primitiveArrayCount();
	if( numPrims)
	{
		const MGeometryPrimitive prim = geom.primitiveArray(0);

		NumIndices = prim.elementCount();
		if( NumIndices)
		{
			//MGeometryData::ElementType primType = prim.dataType();
			unsigned int *idx = (unsigned int *) prim.data();

			// Get the position data
			const MGeometryData pos = geom.position();
			float * posPtr = (float * )pos.data();
			if( !idx || !posPtr) return false;
			NumVertices = pos.elementCount();

			// Start building our vertex format. We always have position, so
			// start with that and add in all the elements we find along the way
			Stride = sizeof( float) * 3;

			// Get the normals data
			float * normPtr = NULL;
			if( useNormals)
			{
				const MGeometryData norm = geom.normal();				
				normPtr = (float * )norm.data();
				Stride += sizeof( float) * 3;
			}

			// Get the texture coordinate data
			float *uvPtr = NULL;
			if( haveTexture)
			{
				const MGeometryData uvs = geom.texCoord( uvSetName );
				uvPtr = (float *)uvs.data();
				Stride += sizeof( float) * 2;
			}

			// Always using FixedFunctionVertex for now
			Stride = sizeof(FixedFunctionVertex);
			useNormals = true;
			haveTexture = true;

			// Copy our vertex data into the buffer
			//
			int FloatsPerVertex = Stride / sizeof( float);
			float* vertBuffer = new float[ NumVertices * FloatsPerVertex ];
			float* VertexData = vertBuffer;
			int StrideOffset = FloatsPerVertex - 3;
			//MGlobal::displayInfo( MString( "Allocating buffers for ") + NumVertices + MString( " verts and ") + NumIndices + MString( " indices\n"));
			for( unsigned int i = 0; i < NumVertices; i++)
			{
				*VertexData++ = *posPtr++;
				*VertexData++ = *posPtr++;
				*VertexData++ = *posPtr++;
				VertexData += StrideOffset;
			}
			VertexData -= NumVertices * FloatsPerVertex - 3;

			if( normPtr)
			{
				for( unsigned int i = 0; i < NumVertices; i++)
				{
					*VertexData++ = *normPtr++;
					*VertexData++ = *normPtr++;
					*VertexData++ = *normPtr++;
					VertexData += StrideOffset;
				}
				VertexData -= NumVertices * FloatsPerVertex - 3;
			}

			if( uvPtr)
			{
				StrideOffset = FloatsPerVertex - 2;
				for( unsigned int i = 0; i < NumVertices; i++)
				{
					*VertexData++ = *uvPtr++;
					*VertexData++ = 1.0f - *uvPtr++;
					VertexData += StrideOffset;
				}
				VertexData -= NumVertices * FloatsPerVertex - 2;
			}

			// Allocate our vertex buffer
			//
			D3D11_BUFFER_DESC bd;
			ZeroMemory( &bd, sizeof(bd) );
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = Stride * NumVertices;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;
			D3D11_SUBRESOURCE_DATA InitData;
			ZeroMemory( &InitData, sizeof(InitData) );
			InitData.pSysMem = vertBuffer;
			hr = D3D->CreateBuffer( &bd, &InitData, &VertexBuffer );
			if ( FAILED( hr ) )
			{
				MGlobal::displayWarning( "DX11 renderer : Unable to allocate vertex buffer\n");
				return false;
			}

			delete [] vertBuffer;

			// Allocate our index buffer
			//
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = NumIndices * sizeof( DWORD);
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;
			InitData.pSysMem = idx;
			hr = D3D->CreateBuffer( &bd, &InitData, &IndexBuffer );
			if ( FAILED( hr ) )
			{
				MGlobal::displayWarning( "DX11 renderer : Unable to allocate index buffer\n");
				return false;
			}
		}
	}
	return IndexBuffer && VertexBuffer;
}


#endif



