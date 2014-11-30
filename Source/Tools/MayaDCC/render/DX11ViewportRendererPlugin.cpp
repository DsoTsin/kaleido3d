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
#include <maya/MStringArray.h>
#include <maya/MFnPlugin.h>
#include <maya/MSceneMessage.h>

#include <stdio.h>

///////////////////////////////////////////////////
//
// Plug-in functions
//
///////////////////////////////////////////////////

static DX11ViewportRenderer *g_d3dRenderer = 0;
static __declspec(align(16)) char g_d3dRendererStorage[ sizeof(DX11ViewportRenderer) ] ;

#if defined(DX11_SUPPORTED)

static MCallbackId	 D3DfileNewCallbackId = 0;
static void D3DfileNewCallback( void* clientData )
{
	if (g_d3dRenderer)
		g_d3dRenderer->clearResources(false, false); /* keep shaders */
}

static MCallbackId	 D3DmayaExitCallbackId = 0;
static void D3DmayaExitCallback( void* clientData )
{
	if (g_d3dRenderer)
	{
		g_d3dRenderer->~DX11ViewportRenderer();
		g_d3dRenderer = 0;
	}
}
#endif

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status = MStatus::kFailure;

	MFnPlugin plugin( obj, PLUGIN_COMPANY, "11.0", "Any");

	// Register the renderer
	//
	g_d3dRenderer = new (g_d3dRendererStorage) DX11ViewportRenderer();
	if (g_d3dRenderer)
	{
		status = g_d3dRenderer->registerRenderer();
		if (status != MStatus::kSuccess)
		{
			status.perror("Failed to register DX11 renderer properly.");
		}

#if defined(DX11_SUPPORTED)
		// Must add in these callbacks as on exit Maya never calls uninitializePlugin() !!
		//
		if (D3DfileNewCallbackId == 0)
			D3DfileNewCallbackId = MSceneMessage::addCallback( MSceneMessage::kBeforeNew, D3DfileNewCallback );		
		if (D3DmayaExitCallbackId == 0)
			D3DmayaExitCallbackId = MSceneMessage::addCallback( MSceneMessage::kMayaExiting, D3DmayaExitCallback );		
#endif
	}
	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus   status = MStatus::kSuccess;

#if defined(DX11_SUPPORTED)
	// Remove callbacks when plugin is unloaded.
	//
	if (D3DfileNewCallbackId)
	{
		MMessage::removeCallback( D3DfileNewCallbackId );		
		D3DfileNewCallbackId = 0;
	}
	if (D3DmayaExitCallbackId)
	{
		MMessage::removeCallback( D3DmayaExitCallbackId );
		D3DmayaExitCallbackId = 0;
	}
#endif

	// Deregister the renderer
	//
	if (g_d3dRenderer)
	{
		status = g_d3dRenderer->deregisterRenderer();
		if (status != MStatus::kSuccess)
		{
			status.perror("Failed to deregister DX11 renderer properly.");
		}
		g_d3dRenderer->~DX11ViewportRenderer();
		g_d3dRenderer = 0;
	}
	return status;
}

