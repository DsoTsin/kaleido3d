#ifndef DX11GeometryItem_h_
#define DX11GeometryItem_h_

#if defined(DX11_SUPPORTED)

#include <maya/MDagPath.h>
#include <maya/MObjectHandle.h>
#include <maya/MMessage.h> // For monitoring geometry list nodes
#include <list>

	#define WIN32_LEAN_AND_MEAN
	#include <d3d11.h>
	// for VS 2012, Win8 SDK includes DX sdk with some headers removed
	#if _MSC_VER >= 1700
		#include <dxgi.h>
	#else
		#include <d3dx11.h>
	#endif


//
// A class to manage populate and render D3D buffers representing Maya geometry
//
class D3DGeometry
{
public:
	D3DGeometry()
		: VertexBuffer( NULL), IndexBuffer( NULL), Stride( 0), NumVertices( 0), NumIndices( 0)
	{
	}
	~D3DGeometry() 
	{ 
		Release(); 
	}
	bool Populate( const MDagPath& dagPath, ID3D11Device* D3D);
	bool Render( ID3D11DeviceContext* D3D)
	{
		UINT stride = Stride;
		UINT offset = 0;
		D3D->IASetVertexBuffers( 0, 1, &VertexBuffer, &stride, &offset );
		D3D->IASetIndexBuffer( IndexBuffer, DXGI_FORMAT_R32_UINT, 0 );
		D3D->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		D3D->DrawIndexed( NumIndices, 0, 0 );
		return true;
	}
	void Release()
	{
		if( VertexBuffer) 
			VertexBuffer->Release(); 
		if( IndexBuffer) 
			IndexBuffer->Release();	
		VertexBuffer = NULL;
		IndexBuffer = NULL;
	}

private:
	ID3D11Buffer*			VertexBuffer;
	ID3D11Buffer*			IndexBuffer;
	unsigned int			NumVertices;
	unsigned int			NumIndices;
	int						Stride;
};

// Geometry item use to associate a D3DGeometry with Maya geometry
//
struct GeometryItem {
	MDagPath				m_objectPath;
	D3DGeometry*			m_objectGeometry;
	MCallbackId				m_objectDeleteMonitor;
	MCallbackId				m_objectDirtyMonitor;
	MCallbackId				m_objectChangeMonitor;
};
typedef std::list<GeometryItem*> GeometryItemList;

#endif

#endif /* DX11GeometryItem_h_ */

//-
// ==========================================================================
// Copyright 2010 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

