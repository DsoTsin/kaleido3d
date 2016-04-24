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
#if defined(DX11_SUPPORTED)
#if _MSC_VER >= 1700
#pragma warning( disable: 4005 )
#endif

#pragma warning (disable:4239)
#include <stdio.h>

#include "DX11ResourceManager.h"

#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MFnCamera.h>
#include <maya/MAngle.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MItDag.h>
#include <maya/MMatrix.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MBoundingBox.h>
#include <maya/MNodeMessage.h> // For monitor geometry list
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnLight.h>
#include <maya/MFnSpotLight.h>
#include <maya/MColor.h>
#include <maya/MFloatMatrix.h>

//////////////////////////////////////////////////////////////////////////////////////
//
// Monitors on Maya scene graph
//

// Handle node dirty changes
void geometryDirtyCallback( void* clientData )
{
	GeometryItem *item = (GeometryItem *)clientData;
	if (item)
	{
		MMessage::removeCallback( item->m_objectDirtyMonitor );	
		item->m_objectDirtyMonitor = 0;
		item->m_objectPath = MDagPath(); // Assign in valid dag path to mark as "bad"
	}
}

// Handle node attr change 
void geomteryChangedCallback( MNodeMessage::AttributeMessage msg, MPlug & plug, MPlug & otherPlug, void* clientData)
{
	GeometryItem *item = (GeometryItem *)clientData;
	if (item)
	{
		MMessage::removeCallback( item->m_objectChangeMonitor );	
		item->m_objectChangeMonitor = 0;
		item->m_objectPath = MDagPath(); // Assign in valid dag path to mark as "bad"
	}
}
void textureChangedCallback( MNodeMessage::AttributeMessage msg, MPlug & plug, MPlug & otherPlug, void* clientData)
{
	TextureItem *item = (TextureItem *)clientData;
	if (item)
	{
		MMessage::removeCallback( item->m_objectChangeMonitor );	
		item->m_objectChangeMonitor = 0;
		item->m_mayaNode = MObject::kNullObj; // Assign in valid dag path to mark as "bad"
	}
}

// Handle node delete
void geometryDeleteCallback( MObject &node,
						MDGModifier& modifier,
						void* clientData )
{
	GeometryItem *item = (GeometryItem *)clientData;
	if (item)
	{
		MMessage::removeCallback( item->m_objectDeleteMonitor );	
		item->m_objectDeleteMonitor = 0;
		item->m_objectPath = MDagPath(); // Assign in valid dag path to mark as "bad"
	}
}

void textureDeleteCallback( MObject &node,
						MDGModifier& modifier,
						void* clientData )
{
	TextureItem *item = (TextureItem *)clientData;
	if (item)
	{
		MMessage::removeCallback( item->m_objectDeleteMonitor );	
		item->m_objectDeleteMonitor = 0;
		item->m_mayaNode = MObject::kNullObj; // Assign in valid dag path to mark as "bad"
	}
}

///////////////////////////////

DX11ResourceManager::DX11ResourceManager()
{
	initializeDefaultCamera();
}

/* virtual */
DX11ResourceManager::~DX11ResourceManager()
{
	const bool onlyInvalidItems = false;
	clearResources(onlyInvalidItems, true);
}

void					
DX11ResourceManager::initializeDefaultCamera()
{
	// Set the default camera being 10 up and 10 back, with Y-up (to match Maya).
    m_camera.m_vEyePt = XMFLOAT3( 0.0f, 10.0f, -10.0f );
    m_camera.m_vLookatPt = XMFLOAT3( 0.0f, 0.0f, 0.0f );
    m_camera.m_vUpVec = XMFLOAT3( 0.0f, 1.0f, 0.0f );

	// Set up default clip planes, and FOV to match Maya's
	m_camera.m_FieldOfView = 45.0f;
	m_camera.m_nearClip = 0.1f;
	m_camera.m_farClip = 1000.0f;
}

bool					
DX11ResourceManager::translateCamera( const MDagPath &cameraPath )
//
// Description:
//		Translate Maya's camera 
//
{
	bool translatedCamera = false;
	if (cameraPath.isValid())
	{
		MStatus status;
		MFnCamera camera (cameraPath, &status);
		if ( !status ) {
			status.perror("MFnCamera constructor");
		}
		else
		{
			translatedCamera = true;

			MPoint eyePoint = camera.eyePoint( MSpace::kWorld );
			MPoint lookAtPt	= camera.centerOfInterestPoint( MSpace::kWorld );
			MVector	upDirection = camera.upDirection ( MSpace::kWorld );
			MFloatMatrix projMatrix = camera.projectionMatrix();

			double horizontalFieldOfView = MAngle( /* camera.verticalFieldOfView() / */ camera.horizontalFieldOfView()
				).asDegrees();
			double nearClippingPlane = camera.nearClippingPlane();
			double farClippingPlane = camera.farClippingPlane();

			// Convert API values to internal native storage.
			//
			m_camera.m_vEyePt = XMFLOAT3((float)eyePoint.x, (float)eyePoint.y, (float)eyePoint.z);
			m_camera.m_vLookatPt = XMFLOAT3((float)lookAtPt.x, (float)lookAtPt.y, (float)lookAtPt.z);
			m_camera.m_vUpVec = XMFLOAT3((float)upDirection.x, (float)upDirection.y, (float)upDirection.z);
			m_camera.m_FieldOfView = (float)horizontalFieldOfView;
			m_camera.m_nearClip = (float)nearClippingPlane;
			m_camera.m_farClip = (float)farClippingPlane;
			m_camera.m_isOrtho = camera.isOrtho();
		}
	}
	else
	{
		initializeDefaultCamera();
	}
	return translatedCamera;
}

//
// Get the geometry buffers for this bad boy
//
D3DGeometry* DX11ResourceManager::getGeometry( const MDagPath& dagPath, ID3D11Device* D3D)
{
	D3DGeometry* Geometry = NULL;

	// Look for a cached mesh ...
	//
	// Check to see if object is in the list, if not added a
	// new item and cache some geometry
	GeometryItem *itemFound = NULL;

	GeometryItemList::const_iterator it, end_it;
	end_it = m_geometryItemList.end();
	for (it = m_geometryItemList.begin(); it != end_it;  it++)
	{
		GeometryItem *item = *it;
		if (item && item->m_objectPath == dagPath)
		{
			itemFound = item;
		}
	}
	// Build a new item, and add it to the list
	if (!itemFound)
	{
		itemFound = new GeometryItem;
		itemFound->m_objectPath = dagPath;
		Geometry = itemFound->m_objectGeometry = new D3DGeometry();
		if (Geometry)
		{
			Geometry->Populate( dagPath, D3D);
		}

		MFnDagNode dagNode(dagPath);
		MObject &obj = (MObject &) dagNode.object();
		itemFound->m_objectDeleteMonitor = 
			MNodeMessage::addNodeAboutToDeleteCallback( obj, geometryDeleteCallback, (void *)itemFound ); // Add callback for node deleted.
		// Don't get attr change messages during playback, so use node dirty also..sigh
		//itemFound->m_objectChangeMonitor = 
		//	MNodeMessage::addAttributeChangedCallback( obj, geometryChangedCallback, (void *)itemFound); // Add callback for attr changed.
		itemFound->m_objectChangeMonitor = 0;
		itemFound->m_objectChangeMonitor = 
			MNodeMessage::addNodeDirtyCallback( obj, geometryDirtyCallback, (void *)itemFound); // Add callback for node changed.		
		m_geometryItemList.push_back( itemFound );
	}
	else
	{
		Geometry = itemFound->m_objectGeometry;
	}

	// Create a new set of buffers for this mesh
	//
	return Geometry;
}


//
// Get the DirectX texture for a Maya texture node
//
D3DTexture* DX11ResourceManager::getTexture( MObject& textureNode)
{
	D3DTexture* Texture = NULL;

	// Look for a cached texture ...
	//
	// Check to see if object is in the list, if not added a
	// new item and cache some texture
	TextureItem *itemFound = NULL;

	TextureItemList::const_iterator it, end_it;
	end_it = m_textureItemList.end();
	for (it = m_textureItemList.begin(); it != end_it;  it++)
	{
		TextureItem *item = *it;
		if (item && item->m_mayaNode.object() == textureNode)
		{
			itemFound = item;
		}
	}
	// Build a new item, and add it to the list
	if (!itemFound)
	{
		itemFound = new TextureItem();
		itemFound->m_mayaNode = textureNode;
		Texture = itemFound->m_texture = new D3DTexture( textureNode);

		itemFound->m_objectDeleteMonitor = 
			MNodeMessage::addNodeAboutToDeleteCallback( textureNode, textureDeleteCallback, (void *)itemFound ); // Add callback for node deleted.
		itemFound->m_objectChangeMonitor = 
			MNodeMessage::addAttributeChangedCallback( textureNode, textureChangedCallback, (void *)itemFound); // Add callback for attr changed.

		m_textureItemList.push_back( itemFound );
	}
	else
	{
		Texture = itemFound->m_texture;
	}

	// Create a new set of buffers for this mesh
	//
	return Texture;
}

//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
static HRESULT CompileShaderFromFile( const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

#ifndef D3DCOMPILE_ENABLE_STRICTNESS
// allow the plug-in to compile against the Maya 2011 default DXSDK version (August 2009)
#define D3DCOMPILE_ENABLE_STRICTNESS D3D10_SHADER_ENABLE_STRICTNESS
#define D3DCOMPILE_DEBUG D3D10_SHADER_DEBUG
#endif
	
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            MGlobal::displayInfo( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}

bool					
DX11ResourceManager::initializeDefaultSurfaceEffect( const MString& effectsLocation, ID3D11Device* D3D,
												    const MString& effectName, 
													const MString& vsName, const MString& psName,
													const D3D11_INPUT_ELEMENT_DESC* layout, int numLayoutElements )
//
// Description:
//		Initialize default surface effects found in a given directory.
//
{
	HRESULT hres;
	MString effectLocation = effectsLocation + "\\" + effectName + ".hlsl";

    ID3DBlob* pVSBlob = NULL;
    hres = CompileShaderFromFile( effectLocation.asChar(), vsName.asChar(), "vs_4_0", &pVSBlob );
	if (FAILED(hres))
	{
		MGlobal::displayInfo("Failed to compile vertex shader " + vsName + " in file: " + effectLocation);
		return false;
	}	
	ID3D11VertexShader* pVertexShader = NULL;
	hres = D3D->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &pVertexShader );
	if (FAILED(hres))
	{
		MGlobal::displayInfo("Failed to create vertex shader " + vsName + " in file: " + effectLocation);
		pVSBlob->Release();
		return false;
	}
	ID3D11InputLayout* pVertexLayout = NULL;
    hres = D3D->CreateInputLayout( layout, numLayoutElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &pVertexLayout );
	pVSBlob->Release();
	if (FAILED(hres))
	{
		MGlobal::displayInfo("Failed to create input layout for file: " + effectLocation);
		return false;
	}

    ID3DBlob* pPSBlob = NULL;
    hres = CompileShaderFromFile( effectLocation.asChar(), psName.asChar(), "ps_4_0", &pPSBlob );
	if (FAILED(hres))
	{
		MGlobal::displayInfo("Failed to compile pixel shader " + psName + " in file: " + effectLocation);
		pVertexShader->Release();
		pVertexLayout->Release();
		return false;
	}	
	ID3D11PixelShader* pPixelShader = NULL;
	hres = D3D->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pPixelShader );
	pPSBlob->Release();
	if (FAILED(hres))
	{
		MGlobal::displayInfo("Failed to create pixel shader " + psName + " in file: " + effectLocation);
		pVertexShader->Release();
		pVertexLayout->Release();
		return false;
	}	

		// Create a new effect item
		//
		MGlobal::displayInfo("Maya default pixel shader loaded: " + effectLocation);
		SurfaceEffectItem *pei = new SurfaceEffectItem;
		if (pei)
		{
			pei->fVertexShader = pVertexShader;
			pei->fPixelShader = pPixelShader;
			pei->fInputLayout = pVertexLayout;

			m_SurfaceEffectItemList[ effectName.asChar() ] = pei;
		}

	return true;
}

//////////////////////////////////////////////////////////
void					
DX11ResourceManager::clearResources(bool onlyInvalidItems, bool clearShaders)
{
	GeometryItemList::const_iterator git, end_git;
	end_git = m_geometryItemList.end();
	for (git = m_geometryItemList.begin(); git != end_git;  git++)
	{
		GeometryItem *item = *git;
		if (item)
		{
			if (!onlyInvalidItems || (onlyInvalidItems && !(item->m_objectPath.isValid() )))
			{
				if (item->m_objectGeometry)
				{
					delete item->m_objectGeometry;
					item->m_objectGeometry = NULL;
				}
				item->m_objectPath = MDagPath(); // Assign invalid dag path

				// Kill the delete monitor
				if (item->m_objectDeleteMonitor)
				{
					MMessage::removeCallback( item->m_objectDeleteMonitor );
					item->m_objectDeleteMonitor = 0;
				}
				// Kill the attr changed monitor
				if (item->m_objectChangeMonitor)
				{
					MMessage::removeCallback( item->m_objectChangeMonitor );	
					item->m_objectChangeMonitor = 0;					
				}
				// Kill node dirty monitor
				if (item->m_objectDirtyMonitor)
				{
					MMessage::removeCallback( item->m_objectDirtyMonitor );	
					item->m_objectDirtyMonitor = 0;										
				}
			}
		}
	}
	if (!onlyInvalidItems)
		m_geometryItemList.clear();

	TextureItemList::const_iterator tit, end_tit;
	end_tit = m_textureItemList.end();
	for (tit = m_textureItemList.begin(); tit != end_tit;  tit++)
	{
		TextureItem *item = *tit;
		if (item)
		{
			if (!onlyInvalidItems || (onlyInvalidItems && !(item->m_mayaNode.isValid() )))
			{
				if (item->m_texture)
				{
					delete item->m_texture;
					item->m_texture = NULL;
				}

				item->m_mayaNode = MObject::kNullObj;

				// Kill the delete monitor
				if (item->m_objectDeleteMonitor)
				{
					MMessage::removeCallback( item->m_objectDeleteMonitor );
					item->m_objectDeleteMonitor = 0;
				}
				// Kill the attr changed monitor
				if (item->m_objectChangeMonitor)
				{
					MMessage::removeCallback( item->m_objectChangeMonitor );	
					item->m_objectChangeMonitor = 0;					
				}
			}
		}
	}
	if (!onlyInvalidItems)
		m_textureItemList.clear();

	if (clearShaders)
	{
		// Clean up surface effects list
		{
			SurfaceEffectItemList::const_iterator eit, end_eit;
			end_eit = m_SurfaceEffectItemList.end();
			for (eit = m_SurfaceEffectItemList.begin(); eit != end_eit;  eit++)
			{
				SurfaceEffectItem *item = eit->second;
				if (item)
				{
					if (item->fVertexShader)
						item->fVertexShader->Release();
					if (item->fPixelShader)
						item->fPixelShader->Release();
					if (item->fInputLayout)
						item->fInputLayout->Release();
					delete item;
				}
			}
			m_SurfaceEffectItemList.clear();
		}
	}
}


#endif /* DX11_SUPPORTED */



