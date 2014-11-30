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

#include "DX11TextureItem.h"

#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnDependencyNode.h>

#if defined(DX11_SUPPORTED)

void D3DTexture::Bind( ID3D11Device* D3D)
{
	// Get the filename
	MString filename;
	MFnDependencyNode( MayaTexture).findPlug( "fileTextureName").getValue( filename);
	if( filename.length())
	{
		D3DX11CreateTextureFromFile( D3D, filename.asChar(), NULL, NULL, &HwTexture, NULL );
	}
}

void D3DTexture::setTextureState( ID3D11Device* D3D )
{
}

#endif





