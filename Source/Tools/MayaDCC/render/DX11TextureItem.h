#ifndef DX11TextureItem_h_
#define DX11TextureItem_h_

#if defined(DX11_SUPPORTED)

#include <maya/MObjectHandle.h>
#include <maya/MMessage.h> // For monitoring geometry list nodes
#include <list>

#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>
#include <d3dx11.h>

//
// A class for a D3D texture
//
class D3DTexture
{
public:
	inline D3DTexture( MObject Node = MObject::kNullObj) : MayaTexture( Node), HwTexture( NULL) 
	{
	}
	inline ~D3DTexture() { Release(); }
	void				Populate( MObject MayaTextureNode) { Release(); MayaTexture = MayaTextureNode; }
	void				Release() { if( HwTexture) { /* MGlobal::displayInfo( "Releasing D3D texture\n"); */ HwTexture->Release(); HwTexture = NULL; } }
	inline MObject		Node() const { return MayaTexture; }
	inline ID3D11Resource* Texture( ID3D11Device* D3D) { if( !HwTexture) Bind( D3D); return HwTexture; }
	void	setTextureState( ID3D11Device* D3D );

private:
	void				Bind( ID3D11Device* D3D);
	ID3D11Resource*		HwTexture;
	MObject				MayaTexture;
};

// Class to associate D3DTexture with Maya texture nodes
struct TextureItem {
	MObjectHandle			m_mayaNode;
	D3DTexture*				m_texture;
	MCallbackId				m_objectDeleteMonitor;
	MCallbackId				m_objectChangeMonitor;
};
typedef std::list<TextureItem*> TextureItemList;

#endif

#endif /* DX11TextureItem_h_ */

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

