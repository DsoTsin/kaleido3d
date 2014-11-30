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

#include <assert.h>
#include <stdio.h>
#include <iostream>

#include "DX11ViewportRenderer.h"

#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MRenderingInfo.h>
#include <maya/MRenderTarget.h>
#include <maya/MFnCamera.h>
#include <maya/MAngle.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MItDag.h>
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
#include <maya/MMatrix.h>

#include <stdio.h>
#include <maya/MFnLight.h>
#include <maya/MFnSpotLight.h>

#include <maya/MPxHardwareShader.h>
#include <maya/MRenderProfile.h>

#include <windows.h>	// DX11

#if defined(DX11_SUPPORTED)

// Screen space quad vertex
struct ScreenSpaceVertex
{
    XMVECTOR position; // position
    XMFLOAT2A texCoord; // texture coordinate
};

//////////////////////////////////

//
// Class : DX12ViewportRenderer
//
// Very simple renderer using D3D to render to an offscreen render target.
// The contents are read back into system memory to blit into an OpenGL context.
//
// This example has been test compiled against the both the Feb. and April 2006 
// DirectX developer SDKs. Define the DX11_SUPPORTED preprocessor directive 
// to compile D3D code in.
//
// These code items are work in progress:
//
// - camera is fixed to be perspective. No orthographic cameras yet.
// - offscreen surface is fixed in size.
// - surfaces can either be fixed RGBA8888 or floating point 16. Final output
//   is always fixed, though post-process tone-mapping can be applied before
//	 final output.
// - does not handle loss of device.
// - readback is for color only, no depth readback currently.
// - basic Maya material support in a fixed-function pipeline.
// - geometry support for polys for shaded with file texture on color channel.
//
//
#endif

DX12ViewportRenderer::DX12ViewportRenderer()
:	MViewportRenderer("DX12ViewportRenderer")
{
	// Set the ui name
	fUIName.set( "DX12 Renderer");

	// This renderer overrides all drawing but allows the HUD to be drawn on top
	fRenderingOverride = MViewportRenderer::kOverrideThenStandard;

	// Set API and version number
	m_API = MViewportRenderer::kDirect3D;
	m_Version = 12.0f;

	// Default to something reasonable.
	m_renderWidth = 640;
	m_renderHeight = 480;

#if defined(DX11_SUPPORTED)
	m_hWnd = 0;
	m_pD3DDevice = 0;
	m_pD3D12Device = nullptr;
	m_pD3DDeviceCtx = 0;

	m_pNormalRS = 0;
	m_pWireframeRS = 0;

	m_pTextureOutput = 0;
	m_pTextureOutputView = 0;
	
	m_readBackBuffer.create(m_renderWidth, m_renderHeight, 4/* MPixelType type = kByte */);
	m_readBackBuffer.setRGBA( true );

	m_pBoundsVertexBuffer = 0;
	m_pBoundsIndexBuffer = 0;
	m_pBoundsConstantBuffer = 0;

	m_pFixedFunctionConstantBuffer = 0;

	m_wantFloatingPointTargets = false;
	m_pTextureInterm = 0;
	m_pTextureIntermView = 0;

	m_pDepthStencil = 0;
	m_pDepthStencilView = 0;

	m_pTextureReadBack = 0;

	m_requireDepthStencilReadback = false;
#endif
}

/* virtual */
DX12ViewportRenderer::~DX12ViewportRenderer()
{
	uninitialize();
}

/* virtual */
unsigned int	DX12ViewportRenderer::overrideThenStandardExclusion() const
{
	// exclude everything apart from manipulators
	return ~(unsigned int)kExcludeManipulators;
}

// Dummy window proc.
LRESULT CALLBACK D3DWindowProc( HWND   hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
    switch( msg )
	{
 		case WM_CLOSE:
		{
			//PostQuitMessage(0);	-- can't allow this. Will kill Maya
		}
		break;

        case WM_DESTROY:
		{
            //PostQuitMessage(0);	-- can't allow this. Will kill Maya
		}
        break;
		
		default:
		{
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

#if defined(DX11_SUPPORTED)
bool
DX12ViewportRenderer::buildRenderTargets(unsigned int width, unsigned int height)
{
	HRESULT hr = -1;

	// Nothing to do, just return
	if (width == m_renderWidth &&
		height == m_renderHeight &&
		m_pTextureInterm && 
		m_pTextureOutput)
	{
		return true;
	}

	// Set the new width and height
	m_renderWidth = width;
	m_renderHeight = height;

	//printf("New size = %d,%d\n", m_renderWidth, m_renderHeight);

	//
	// Create target for intermediate rendering
	//
	if (m_pTextureInterm)
	{
		m_pTextureInterm->Release();
		m_pTextureInterm = NULL;
	}
	if (m_pTextureIntermView)
	{
		m_pTextureIntermView->Release();
		m_pTextureIntermView = NULL;
	}
	if (!m_pTextureInterm)
	{
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory( &td, sizeof td );
		td.Width = m_renderWidth;
		td.Height = m_renderHeight;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = m_intermediateTargetFormat; /* Use intermediate target format */
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;

		hr = m_pD3DDevice->CreateTexture2D( &td, NULL, &m_pTextureInterm );

		// Failed to get target with desired intermediate format. Try for
		// fixed as a default
		m_intermediateTargetFormat = m_outputTargetFormat;
		if ( FAILED(hr) )
		{
			td.Format = m_intermediateTargetFormat; /* Use output target format */

			hr = m_pD3DDevice->CreateTexture2D( &td, NULL, &m_pTextureInterm );
		}
		if ( FAILED(hr) )
		{
			MGlobal::displayWarning("DX11 renderer : Failed to create intermediate texture for offscreen render target.");
			return false;
		}
	}
	if (m_pTextureInterm)
	{
		hr = m_pD3DDevice->CreateRenderTargetView( m_pTextureInterm, NULL, &m_pTextureIntermView );
		if ( FAILED(hr) )
		{
			MGlobal::displayWarning("DX11 renderer : Failed to get view for off-screen render target.");
			return false;
		}
	}

	// Create depth buffer
	//
	if ( m_pDepthStencil )
	{
		m_pDepthStencil->Release();
		m_pDepthStencil = 0;
	}
	if ( m_pDepthStencilView )
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = 0;
	}
	if ( !m_pDepthStencil )
	{
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory( &td, sizeof td );
		td.Width = m_renderWidth;
		td.Height = m_renderHeight;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = m_depthStencilFormat;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;

		hr = m_pD3DDevice->CreateTexture2D( &td, NULL, &m_pDepthStencil );
		if ( FAILED(hr) )
		{
			MGlobal::displayWarning("DX11 renderer : Failed to create texture for depth/stencil target.");
			return false;
		}
	}
	if ( m_pDepthStencil )
	{
		hr = m_pD3DDevice->CreateDepthStencilView( m_pDepthStencil, NULL, &m_pDepthStencilView );
		if ( FAILED(hr) )
		{
			MGlobal::displayWarning("DX11 renderer : Failed to get view for depth/stencil target.");
			return false;
		}
	}

	//
	// 2. Create output render targets
	//
	// If we don't want floating point, then the intermediate is
	// the final output format, so don't bother creating another one.
	// Just make the output point to the intermediate target.
	assert( !m_wantFloatingPointTargets );
	if (m_wantFloatingPointTargets)
	{
	}
	else
	{
		m_pTextureOutput = m_pTextureInterm;
		m_pTextureOutputView = m_pTextureIntermView;
	}

	// 3. Create system memory surface for readback
	if (m_pTextureReadBack)
	{
		m_pTextureReadBack->Release();
		m_pTextureReadBack= 0;
	}
	if (!m_pTextureReadBack)
	{
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory( &td, sizeof td );
		td.Width = m_renderWidth;
		td.Height = m_renderHeight;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = m_intermediateTargetFormat; /* Use intermediate target format */
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_STAGING;
		td.BindFlags = 0;
		td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		td.MiscFlags = 0;

		hr = m_pD3DDevice->CreateTexture2D( &td, NULL, &m_pTextureReadBack );

		if (FAILED(hr))
		{
			MGlobal::displayWarning("DX11 renderer : Failed to create system memory readback surface.");
			return false;
		}
	}

	return (m_pTextureOutput && m_pTextureOutputView && m_pTextureInterm && 
			m_pTextureIntermView && m_pTextureReadBack);

#if defined(DEPTH_REQUIRED)
	// 4. Create depth stencil surface for access for readback.
	if (m_pDepthStencilSurface)
	{
		m_pDepthStencilSurface->Release();
		m_pDepthStencilSurface = 0;
	}
	if (m_requireDepthStencilReadback && !m_pDepthStencilSurface)
	{
		hr = m_pD3DDevice->CreateDepthStencilSurface( 
						m_renderWidth, m_renderHeight, m_depthStencilFormat, D3DMULTISAMPLE_NONE, 
						0, FALSE, 
						&m_pDepthStencilSurface, NULL );
		if (FAILED(hr))
		{
			MGlobal::displayWarning("DX11 renderer : Failed to create depth/stencil surface. Depth read back will not be available.");
		}
	}
#endif
}

bool
DX12ViewportRenderer::createBoundsBuffers()
{
	HRESULT hr;

	BoundsVertex vertices[] = 
	{
		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ) },
		{ XMFLOAT3( -1.0f, -1.0f,  1.0f ) },
		{ XMFLOAT3( -1.0f,  1.0f, -1.0f ) },
		{ XMFLOAT3( -1.0f,  1.0f,  1.0f ) },
		{ XMFLOAT3(  1.0f, -1.0f, -1.0f ) },
		{ XMFLOAT3(  1.0f, -1.0f,  1.0f ) },
		{ XMFLOAT3(  1.0f,  1.0f, -1.0f ) },
		{ XMFLOAT3(  1.0f,  1.0f,  1.0f ) },
	};

    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( BoundsVertex ) * 8;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = vertices;
    hr = m_pD3DDevice->CreateBuffer( &bd, &InitData, &m_pBoundsVertexBuffer );
    if ( FAILED( hr ) )
        return false;

	WORD indices[] =
    {
		0, 1, 
		1, 3, 
		3, 2, 
		2, 0,
		4, 5, 
		5, 7, 
		7, 6, 
		6, 4,
		0, 4, 
		1, 5, 
		2, 6, 
		3, 7,
	};

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( WORD ) * 24;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = m_pD3DDevice->CreateBuffer( &bd, &InitData, &m_pBoundsIndexBuffer );
    if ( FAILED( hr ) )
        return false;

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( BoundsConstants );
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = m_pD3DDevice->CreateBuffer( &bd, NULL, &m_pBoundsConstantBuffer );
    if ( FAILED( hr ) )
        return false;

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( FixedFunctionConstants );
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = m_pD3DDevice->CreateBuffer( &bd, NULL, &m_pFixedFunctionConstantBuffer );
    if ( FAILED( hr ) )
        return false;

	return true;
}

bool
DX12ViewportRenderer::createRasterizerStates()
{
	HRESULT hr;

	D3D11_RASTERIZER_DESC rd;
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_BACK;
	rd.FrontCounterClockwise = TRUE;
	rd.DepthBias = 0;
	rd.SlopeScaledDepthBias = 0.0f;
	rd.DepthBiasClamp = 0.0f;
	rd.DepthClipEnable = TRUE;
	rd.ScissorEnable = FALSE;
	rd.MultisampleEnable = FALSE;
	rd.AntialiasedLineEnable = FALSE;

	hr = m_pD3DDevice->CreateRasterizerState( &rd, &m_pNormalRS );
    if ( FAILED( hr ) )
        return false;

	rd.FillMode = D3D11_FILL_WIREFRAME;

	hr = m_pD3DDevice->CreateRasterizerState( &rd, &m_pWireframeRS );
    if ( FAILED( hr ) )
        return false;

	return true;
}
#endif

/* virtual */	
MStatus	
DX12ViewportRenderer::initialize()
{
	MStatus status = MStatus::kFailure;

	// Do we want floating point targets
	//
	MString wantFloatingPoint("D3D_RENDERER_FLOAT_TARGETS");
	int value;
	if (!MGlobal::getOptionVarValue(wantFloatingPoint, value))
	{
		m_wantFloatingPointTargets = true;
	}
	else
	{
		m_wantFloatingPointTargets = (value != 0);
	}
	m_wantFloatingPointTargets = false;

	// Create the window to contain our off-screen target.
	//
	if (!m_hWnd)
	{
		// Register the window class
		WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, (WNDPROC) D3DWindowProc, 0L, 0L, 
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
					  "DX12 Viewport Renderer", NULL };
		if (RegisterClassEx( &wc ))
		{
			m_hWnd = CreateWindow( "DX12 Viewport Renderer", "DX12 Viewport Renderer", 
									WS_OVERLAPPEDWINDOW, 0, 0, m_renderWidth, m_renderHeight,
									NULL, NULL, wc.hInstance, NULL );
		}
	}

	HRESULT hr;

	// Test for floating point buffer usage for render targets
	if (m_wantFloatingPointTargets)
	{
		m_intermediateTargetFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	}
	else
	{
		m_intermediateTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	// The output target is always fixed8 for now.
	m_outputTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	if (m_requireDepthStencilReadback)
	{
		m_depthStencilFormat = DXGI_FORMAT_D32_FLOAT; // Let's try for 32-bit depth, not stencil
	}
	else
		m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT; 

	// Create an appropriate device
	if (m_hWnd)
	{
		if (!m_pD3D12Device)
		{
			hr = ::D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pD3D12Device));
			if (FAILED(hr))
			{
				m_pD3D12Device = nullptr;
			}
		}

	}

	// Get the device context.
	if (m_pD3DDevice)
	{
		m_pD3DDevice->GetImmediateContext( &m_pD3DDeviceCtx );
	}

	// Create targets, buffers, states and load shaders.
	if (m_pD3DDevice)
	{
		bool success = buildRenderTargets(640, 480);
		if ( success )
			success = createBoundsBuffers();
		if ( success )
			success = createRasterizerStates();
		if ( success )
		{
			MString shaderLocation(MString(getenv("MAYA_LOCATION")) + MString("\\devkit\\plug-ins"));

			bool loaded;

			{
				D3D11_INPUT_ELEMENT_DESC layout[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				};
				const int numLayoutElements = sizeof layout/sizeof layout[0];
				loaded = m_resourceManager.initializeDefaultSurfaceEffect( shaderLocation, m_pD3DDevice, "Maya_fixedFunction", 
																			"mainVS", "mainPS", layout, numLayoutElements );
			}

			{
				D3D11_INPUT_ELEMENT_DESC layout[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				};
				const int numLayoutElements = sizeof layout/sizeof layout[0];
				loaded = m_resourceManager.initializeDefaultSurfaceEffect( shaderLocation, m_pD3DDevice, "Maya_unlit", 
																			"mainVS", "mainPS", layout, numLayoutElements );
			}

			// All elements must exist for success
			if (m_hWnd && m_pD3DDevice && m_pD3DDeviceCtx && m_pTextureOutput && m_pTextureOutputView &&
				m_pTextureInterm && m_pTextureIntermView )
			{
				status = MStatus::kSuccess;
			}
		}
	}

	// If for any reason we failed. Cleanup what we can.
	if (status != MStatus::kSuccess)
	{
		uninitialize();
	}
	status = MStatus::kSuccess;

	return status;
}

/* virtual */	
MStatus	
DX12ViewportRenderer::uninitialize()
{	
	if ( m_pTextureOutput )
	{
		if (m_pTextureOutput != m_pTextureInterm)		
			m_pTextureOutput->Release();
		m_pTextureOutput = 0;
	}

	if ( m_pTextureOutputView )
	{
		if (m_pTextureOutputView != m_pTextureIntermView)		
			m_pTextureOutputView->Release();
		m_pTextureOutputView = 0;
	}

	if ( m_pTextureInterm )
	{
        m_pTextureInterm->Release();
		m_pTextureInterm = 0;
	}

	if ( m_pTextureIntermView )
	{
        m_pTextureIntermView->Release();
		m_pTextureIntermView = 0;
	}

	if ( m_pTextureReadBack )
	{
		m_pTextureReadBack->Release();
		m_pTextureReadBack = 0;
	}

	if ( m_pDepthStencil )
	{
		m_pDepthStencil->Release();
		m_pDepthStencil = 0;
	}
	if ( m_pDepthStencilView )
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = 0;
	}

	if ( m_pBoundsVertexBuffer != NULL )
	{
        m_pBoundsVertexBuffer->Release(); 
		m_pBoundsVertexBuffer = 0;
	}
	if ( m_pBoundsIndexBuffer != NULL )
	{
        m_pBoundsIndexBuffer->Release(); 
		m_pBoundsIndexBuffer = 0;
	}
	if ( m_pBoundsConstantBuffer != NULL )
	{
        m_pBoundsConstantBuffer->Release(); 
		m_pBoundsConstantBuffer = 0;
	}
	if ( m_pFixedFunctionConstantBuffer != NULL )
	{
        m_pFixedFunctionConstantBuffer->Release(); 
		m_pFixedFunctionConstantBuffer = 0;
	}
	m_resourceManager.clearResources(false, true); /* wipe out shaders */

	if ( m_pNormalRS )
	{
		m_pNormalRS->Release();
		m_pNormalRS = 0;
	}
	if ( m_pWireframeRS )
	{
		m_pWireframeRS->Release();
		m_pWireframeRS = 0;
	}

	if ( m_pD3DDeviceCtx )
	{
		m_pD3DDeviceCtx->Release();
		m_pD3DDeviceCtx = 0;
	}

    if ( m_pD3DDevice )
	{
        m_pD3DDevice->Release();
		m_pD3DDevice = 0;
	}

	if (m_hWnd)
	{
		ReleaseDC( m_hWnd, GetDC(m_hWnd ));
		DestroyWindow(m_hWnd);
		UnregisterClass("DX12 Viewport Renderer", GetModuleHandle(NULL));
		m_hWnd = 0;
	}

	return MStatus::kSuccess;
}

/* virtual */ 
MStatus	
DX12ViewportRenderer::render(const MRenderingInfo &renderInfo)
{
	MStatus status;

	// Print some diagnostic information.
	//

	const MRenderTarget & target = renderInfo.renderTarget();
	unsigned int currentWidth = target.width();
	unsigned int currentHeight = target.height();
	
	if (!buildRenderTargets(currentWidth, currentHeight))
		return MStatus::kFailure;

	//printf("Render using (%s : %s) renderer\n", fName.asChar(), fUIName.asChar());
	//printf("Render region: %d,%d -> %d, %d into target of size %d,%d\n", 
	//	renderInfo.originX(), renderInfo.originY(), renderInfo.width(), renderInfo.height(),
	//	target.width(), target.height() );


#if defined(DX11_SUPPORTED)
	MViewportRenderer::RenderingAPI targetAPI = renderInfo.renderingAPI();
	//float targetVersion = renderInfo.renderingVersion();
	//printf("Render target API is %s (Version %g)\n", targetAPI == MViewportRenderer::kDirect3D ?
	//		"Direct3D" : "OpenGL", targetVersion);

	// Render if we get a valid camera
	const MDagPath &cameraPath = renderInfo.cameraPath();
	if ( m_resourceManager.translateCamera( cameraPath ) )
	{
		if ( renderToTarget( renderInfo ) )
		{
			// Read back results and set into an intermediate buffer,
			// if the target is not Direct3D. Also readback if we
			// want to debug the buffer.
			//
			bool requireReadBack = (targetAPI != MViewportRenderer::kDirect3D);
			if ( requireReadBack )
			{
				if (readFromTargetToSystemMemory())
				{
					// Blit image back to OpenGL 
					if (targetAPI == MViewportRenderer::kOpenGL)
					{
						// Center the image for now.
						//
						unsigned int targetW = target.width();
						unsigned int targetH = target.height();							
						unsigned int m_readBackBufferWidth, m_readBackBufferHeight;
						m_readBackBuffer.getSize(m_readBackBufferWidth, m_readBackBufferHeight);

						if (m_readBackBufferWidth && m_readBackBufferHeight)
						{
							if (m_readBackBufferWidth > targetW ||
								m_readBackBufferHeight > targetH)
							{
								m_readBackBuffer.resize(targetW, targetH);
								target.writeColorBuffer( m_readBackBuffer, 0, 0 );
							}
							else
							{
								target.writeColorBuffer( m_readBackBuffer, 
									(short)(targetW/2 - m_readBackBufferWidth/2),
									(short)(targetH/2 - m_readBackBufferHeight/2));
							}
							status = MStatus::kSuccess;
						}
					}

					// Blit image back to a software raster
					else
					{
						// To ADD
						status = MStatus::kFailure;
					}
				}
				else
					status = MStatus::kFailure;				
			}

			// Do nothing here. Direct rendering to D3D target
			// should be handled in renderToTarget().
			else 
			{
				status = MStatus::kSuccess;
			}
		}
		else
			status = MStatus::kFailure;
	}
	else
	{
		MGlobal::displayWarning("DX11 renderer : No valid render camera to use. Nothing rendered\n");
		status = MStatus::kFailure;
	}
#else
		status = MStatus::kSuccess;
#endif
	return status;
}

/* virtual */ 
bool	
DX12ViewportRenderer::nativelySupports( MViewportRenderer::RenderingAPI api, 
									   float version )
{
	// Do API and version check
	return ((api == m_API) && (version == m_Version) );
}

/* virtual */ bool	
DX12ViewportRenderer::override( MViewportRenderer::RenderingOverride override )
{
	// Check override
	return (override == fRenderingOverride);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Rendering methods
////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(DX11_SUPPORTED)

bool					
DX12ViewportRenderer::translateCamera( const MRenderingInfo &renderInfo )
//
// Description:
//		Translate Maya's camera 
//
{
	const MDagPath &cameraPath = renderInfo.cameraPath();
	if (cameraPath.isValid())
		return m_resourceManager.translateCamera( cameraPath );
	else
		return false;
}

void					
DX12ViewportRenderer::clearResources(bool onlyInvalidItems, bool clearShaders)
{
	m_resourceManager.clearResources( onlyInvalidItems, clearShaders );
}

MObject findShader( MObject& setNode )
//
//  Description:
//      Find the shading node for the given shading group set node.
//
{
	MFnDependencyNode fnNode(setNode);
	MPlug shaderPlug = fnNode.findPlug("surfaceShader");
			
	if (!shaderPlug.isNull()) {			
		MPlugArray connectedPlugs;
		bool asSrc = false;
		bool asDst = true;
		shaderPlug.connectedTo( connectedPlugs, asDst, asSrc );

		if (connectedPlugs.length() != 1)
			MGlobal::displayError("Error getting shader");
		else 
			return connectedPlugs[0].node();
	}			
	
	return MObject::kNullObj;
}

bool DX12ViewportRenderer::drawSurface( const MDagPath &dagPath, bool active, bool templated)
{
	bool drewSurface = false;

	if ( !dagPath.hasFn( MFn::kMesh ))
	{
		MMatrix  matrix = dagPath.inclusiveMatrix();
		MFnDagNode dagNode(dagPath);
		MBoundingBox box = dagNode.boundingBox();
		float color[3] = {0.6f, 0.3f, 0.0f};
		if (active)
		{
			color[0] = 1.0f;
			color[1] = 1.0f;
			color[2] = 1.0f;
		}
		else if (templated)
		{
			color[0] = 1.0f;
			color[1] = 0.686f;
			color[2] = 0.686f;
		}
		drawBounds( matrix, box, color);
		return true;
	}

	if ( dagPath.hasFn( MFn::kMesh ))
	{
		MMatrix  matrix = dagPath.inclusiveMatrix();
		MFnDagNode dagNode(dagPath);

		// Look for any hardware shaders which can draw D3D first.
		//
		bool drewWithHwShader = false;
		{
			MFnMesh fnMesh(dagPath);
			MObjectArray sets;
			MObjectArray comps;
			unsigned int instanceNum = dagPath.instanceNumber();
			if (!fnMesh.getConnectedSetsAndMembers(instanceNum, sets, comps, true))
				MGlobal::displayError("ERROR : MFnMesh::getConnectedSetsAndMembers");
			for ( unsigned i=0; i<sets.length(); i++ ) 
			{
				MObject set = sets[i];
				MObject comp = comps[i];

				MStatus status;
				MFnSet fnSet( set, &status );
				if (status == MS::kFailure) {
					MGlobal::displayError("ERROR: MFnSet::MFnSet");
					continue;
				}

				MObject shaderNode = findShader(set);
				if (shaderNode != MObject::kNullObj)
				{
					MPxHardwareShader * hwShader = 
						MPxHardwareShader::getHardwareShaderPtr( shaderNode );

					if (hwShader)
					{
						const MRenderProfile & profile = hwShader->profile();
						if (profile.hasRenderer( MRenderProfile::kMayaD3D))
						{
							// Render a Maya D3D hw shader here....
							//printf("Found a D3D hw shader\n");
							//drewWithHwShader = true;
						}
					}
				}
			}
		}

		// Get the geometry buffers for this bad boy and render them
		D3DGeometry* Geometry = m_resourceManager.getGeometry( dagPath, m_pD3DDevice);
		if( Geometry)
		{
			// Transform from object to world space
			//
			XMMATRIX objectToWorld = XMMATRIX
				(
				(float)matrix.matrix[0][0], (float)matrix.matrix[0][1], (float)matrix.matrix[0][2], (float)matrix.matrix[0][3],
				(float)matrix.matrix[1][0], (float)matrix.matrix[1][1], (float)matrix.matrix[1][2], (float)matrix.matrix[1][3],
				(float)matrix.matrix[2][0], (float)matrix.matrix[2][1], (float)matrix.matrix[2][2], (float)matrix.matrix[2][3],
				(float)matrix.matrix[3][0], (float)matrix.matrix[3][1], (float)matrix.matrix[3][2], (float)matrix.matrix[3][3]
			);

			FixedFunctionConstants cb;

			if (!drewWithHwShader)
			{
				// Get material properties for shader associated with mesh
				//
				// 1. Try to draw with the sample internal programmable shader
				bool drewGeometryWithShader = false;

				// 2. Draw with fixed function shader
				if (!drewGeometryWithShader)
				{
					// Set up a default material, just in case there is none.
					//
					float diffuse[3];
					if (active)
					{
						if (templated)
						{
							m_pD3DDeviceCtx->RSSetState( m_pWireframeRS );
							diffuse[0] = 1.0f; diffuse[1] = 0.686f; diffuse[2] = 0.686f;
						}
						else
						{
							m_pD3DDeviceCtx->RSSetState( m_pNormalRS );
							diffuse[0] = 0.6f; diffuse[1] = 0.6f; diffuse[2] = 0.6f;
						}
					}
					else
					{
						if (templated)
						{
							m_pD3DDeviceCtx->RSSetState( m_pWireframeRS );
							diffuse[0] = 1.0f; diffuse[1] = 0.686f; diffuse[2] = 0.686f;
						}
						else
						{
							m_pD3DDeviceCtx->RSSetState( m_pNormalRS );
							diffuse[0] = 0.5f; diffuse[1] = 0.5f; diffuse[2] = 0.5f;
						}
					}

					// Set constant buffer
					XMVECTOR det;
					cb.wvIT = XMMatrixInverse( &det, objectToWorld * m_currentViewMatrix );
					cb.wvp = XMMatrixTranspose( objectToWorld * m_currentViewMatrix * m_currentProjectionMatrix );
					cb.wv = XMMatrixTranspose( objectToWorld * m_currentViewMatrix );
					cb.lightDir = XMFLOAT4( 0.0f, 0.0f, 1.0f, 0.0f );
					cb.lightColor = XMFLOAT4( 1.0f, 1.0f, 1.0f, 0.0f );
					cb.ambientLight = XMFLOAT4( 0.2f, 0.2f, 0.2f, 0.0f );
					cb.diffuseMaterial = XMFLOAT4( diffuse[0], diffuse[1], diffuse[2], 0.0f );
					cb.specularColor = XMFLOAT4( 0.2f, 0.2f, 0.2f, 0.0f );
					cb.diffuseCoeff = 1.0f;
					cb.shininess = 16.0f;
					cb.transparency = 1.0f;
					m_pD3DDeviceCtx->UpdateSubresource( m_pFixedFunctionConstantBuffer, 0, NULL, &cb, 0, 0 );

					// get shader
					SurfaceEffectItemList::const_iterator it = m_resourceManager.getSurfaceEffectItemList().find( "Maya_fixedFunction" );
					if ( it == m_resourceManager.getSurfaceEffectItemList().end() )
						return false;
					const SurfaceEffectItem* sei = it->second;

					// bind shaders
					m_pD3DDeviceCtx->VSSetShader( sei->fVertexShader, NULL, 0 );
					m_pD3DDeviceCtx->VSSetConstantBuffers( 0, 1, &m_pFixedFunctionConstantBuffer );
					m_pD3DDeviceCtx->IASetInputLayout( sei->fInputLayout );
					m_pD3DDeviceCtx->PSSetShader( sei->fPixelShader, NULL, 0 );
					m_pD3DDeviceCtx->PSSetConstantBuffers( 0, 1, &m_pFixedFunctionConstantBuffer );

					Geometry->Render( m_pD3DDeviceCtx );

					drewSurface = true;
				}
			}

			// Draw wireframe on top
			//

			if ( drewSurface && active )
			{
				bool drawActiveWithBounds = false;
				if (drawActiveWithBounds)
				{
					MBoundingBox box = dagNode.boundingBox();
					float color[3] = {1.0f, 1.0f, 1.0f};
					drawBounds( matrix, box, color );
				}
				else
				{
					cb.lightColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
					cb.ambientLight = XMFLOAT4( 1.0f, 1.0f, 1.0f, 0.0f );
					cb.diffuseMaterial = XMFLOAT4( 1.0f, 1.0f, 1.0f, 0.0f );
					cb.specularColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
					m_pD3DDeviceCtx->UpdateSubresource( m_pFixedFunctionConstantBuffer, 0, NULL, &cb, 0, 0 );

					m_pD3DDeviceCtx->RSSetState( m_pWireframeRS );

					Geometry->Render( m_pD3DDeviceCtx );				
				}
			}
		} // If Geometry
	}
	return drewSurface;
}

bool DX12ViewportRenderer::drawScene(const MRenderingInfo &renderInfo)
//
// Description:
//		Draw the Maya scene, using a custom traverser.
//
{
	bool useDrawTraversal = true;
	float groundPlaneColor[3] = { 0.8f, 0.8f, 0.8f };

	if (useDrawTraversal)
	{
		const MDagPath &cameraPath = renderInfo.cameraPath();
		if (cameraPath.isValid())
		{
			// You can actually keep the traverser classes around
			// if desired. Here we just create temporary traversers
			// on the fly.
			//
			MDrawTraversal *trav = new MDrawTraversal;
			if (!trav)
			{
				MGlobal::displayWarning("DX11 renderer : failed to create a traversal class !\n");
				return true;
			}

			trav->enableFiltering( false );

			const MRenderTarget &renderTarget = renderInfo.renderTarget();
			trav->setFrustum( cameraPath, renderTarget.width(), 
							  renderTarget.height() );

			if (!trav->frustumValid())
			{
				MGlobal::displayWarning("DX11 renderer : Frustum is invalid !\n");
				return true;
			}

			trav->traverse();

			unsigned int numItems = trav->numberOfItems();
			unsigned int i;
			for (i=0; i<numItems; i++)
			{
				MDagPath path;
				trav->itemPath(i, path);

				if (path.isValid())
				{
					bool drawIt = false;

					// Default traverer may have view manips showing up.
					// This is currently a known Maya bug.
					if ( path.hasFn( MFn::kViewManip ))
						continue;

					//
					// Draw surfaces (polys, nurbs, subdivs)
					//
					bool active = false;
					bool templated = false;
					if ( path.hasFn( MFn::kMesh) || 
						 path.hasFn( MFn::kNurbsSurface) || 
						 path.hasFn( MFn::kSubdiv) )
					{
						drawIt = true;
						if (trav->itemHasStatus( i, MDrawTraversal::kActiveItem ))
						{
							active = true;
						}
						if (trav->itemHasStatus( i, MDrawTraversal::kTemplateItem ))
						{
							templated = true;
						}
					}

					//
					// Draw the ground plane
					//
					else if (path.hasFn( MFn::kSketchPlane ) ||
							 path.hasFn( MFn::kGroundPlane ))
					{
						MMatrix  matrix = path.inclusiveMatrix();
						MFnDagNode dagNode(path);
						MBoundingBox box = dagNode.boundingBox();
						drawBounds( matrix, box, groundPlaneColor );
					}

					if (drawIt)
					{
						drawSurface( path, active, templated );
					}
				}
			}

			if (trav)
				delete trav;

			// Cleanup any unused resource items
			bool onlyInvalidItems = true;
			clearResources( onlyInvalidItems, false );
		}
	}
	else
	{
		// Draw some poly bounding boxes 
		//
		MItDag::TraversalType traversalType = MItDag::kDepthFirst;
		MFn::Type filter = MFn::kMesh;
		MStatus status;

		MItDag dagIterator( traversalType, filter, &status);

		for ( ; !dagIterator.isDone(); dagIterator.next() ) 
		{

			MDagPath dagPath;

			status = dagIterator.getPath(dagPath);
			if ( !status ) {
				status.perror("MItDag::getPath");
				continue;
			}

			MFnDagNode dagNode(dagPath, &status);
			if ( !status ) {
				status.perror("MFnDagNode constructor");
				continue;
			}

			MMatrix  matrix = dagPath.inclusiveMatrix();
			MBoundingBox box = dagNode.boundingBox();
			drawBounds( matrix, box, groundPlaneColor );
		}
	}
	return true;
}

bool DX12ViewportRenderer::drawBounds( const MMatrix &matrix, const MBoundingBox &box, float color[3] )
{
	// Transform from object to world space
	//
	XMMATRIX mat = XMMATRIX
		(
		(float)matrix.matrix[0][0], (float)matrix.matrix[0][1], (float)matrix.matrix[0][2], (float)matrix.matrix[0][3],
		(float)matrix.matrix[1][0], (float)matrix.matrix[1][1], (float)matrix.matrix[1][2], (float)matrix.matrix[1][3],
		(float)matrix.matrix[2][0], (float)matrix.matrix[2][1], (float)matrix.matrix[2][2], (float)matrix.matrix[2][3],
		(float)matrix.matrix[3][0], (float)matrix.matrix[3][1], (float)matrix.matrix[3][2], (float)matrix.matrix[3][3]
		);

	// Adjust the unit cube to the bounds
	//
	MPoint	minPt = box.min();
	MPoint	maxPt = box.max();
	float minVal[3] = { (float)minPt.x, (float)minPt.y, (float)minPt.z };
	float maxVal[3] = { (float)maxPt.x, (float)maxPt.y, (float)maxPt.z };
	XMMATRIX bounds( 0.5f*(maxVal[0]-minVal[0]), 0.0f,						 0.0f,							0.0f,
					 0.0f,						 0.5f*(maxVal[1]-minVal[1]), 0.0f,							0.0f,
					 0.0f,						 0.0f,						 0.5f*(maxVal[2]-minVal[2]),	0.0f,
					 0.5f*(maxVal[0]+minVal[0]), 0.5f*(maxVal[1]+minVal[1]), 0.5f*(maxVal[2]+minVal[2]),	1.0f );

    // Set vertex buffer
    UINT stride = sizeof( BoundsVertex );
    UINT offset = 0;
    m_pD3DDeviceCtx->IASetVertexBuffers( 0, 1, &m_pBoundsVertexBuffer, &stride, &offset );

	// Set index buffer
    m_pD3DDeviceCtx->IASetIndexBuffer( m_pBoundsIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

	// Set constant buffer
    BoundsConstants cb;
	cb.fWVP = XMMatrixTranspose( bounds * mat * m_currentViewMatrix * m_currentProjectionMatrix );
	cb.fDiffuseMaterial = XMFLOAT3( color[0], color[1], color[2] );
	m_pD3DDeviceCtx->UpdateSubresource( m_pBoundsConstantBuffer, 0, NULL, &cb, 0, 0 );

    // Set primitive topology
    m_pD3DDeviceCtx->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

	// get shader
	SurfaceEffectItemList::const_iterator it = m_resourceManager.getSurfaceEffectItemList().find( "Maya_unlit" );
	if ( it == m_resourceManager.getSurfaceEffectItemList().end() )
		return false;
	const SurfaceEffectItem* sei = it->second;

	// bind shaders
    m_pD3DDeviceCtx->VSSetShader( sei->fVertexShader, NULL, 0 );
	m_pD3DDeviceCtx->VSSetConstantBuffers( 0, 1, &m_pBoundsConstantBuffer );
    m_pD3DDeviceCtx->IASetInputLayout( sei->fInputLayout );
    m_pD3DDeviceCtx->PSSetShader( sei->fPixelShader, NULL, 0 );
	m_pD3DDeviceCtx->PSSetConstantBuffers( 0, 1, &m_pBoundsConstantBuffer );

	// draw
    m_pD3DDeviceCtx->DrawIndexed( 24, 0, 0 );

	return true;
}

bool DX12ViewportRenderer::renderToTarget( const MRenderingInfo &renderInfo )
//
// Description:
//		Rener to off-screen render target and read back into system memory 
//		output buffer.
//
//
{
	// Direct rendering to a D3D surface
	//
	if (renderInfo.renderingAPI() == MViewportRenderer::kDirect3D)
	{
		// Maya does not support D3D currently. Would need
		// to have access to the device, and surface here
		// from an MRenderTarget. API doesn't exist, so
		// do nothing.
		return false;
	}

	//
	// Offscreen rendering
	//
	if (!m_pD3DDevice || !m_pD3DDeviceCtx || !m_pTextureOutput || !m_pTextureInterm)
		return false;

	// START RENDER

	// Set colour and depth surfaces.
	//
	ID3D11RenderTargetView* targets[] = { m_pTextureIntermView };
	m_pD3DDeviceCtx->OMSetRenderTargets( 1, targets, m_pDepthStencilView );

	{
		// Setup projection and view matrices
		setupMatrices( renderInfo );

		// set rasterizer state
		m_pD3DDeviceCtx->RSSetState( m_pNormalRS );

		// Clear the entire buffer (RGB, Depth). Leave stencil for now.
		//
		float clearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; // RGBA
		m_pD3DDeviceCtx->ClearRenderTargetView( m_pTextureIntermView, clearColor );
		if ( m_pDepthStencilView )
			m_pD3DDeviceCtx->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

		// Render the scene
		drawScene(renderInfo);
	}
	// END SCENE RENDER

	return true;
}

bool DX12ViewportRenderer::setupMatrices( const MRenderingInfo &info )
//
// Description:
//
//		Set up camera matrices. Mechanism to check for changes in camera
//		parameters should be done before matrix setup.
//
//		Note that we *must* use a "right-handed" system (RH method 
//		versions) for computations to match what is coming from Maya.
//
{
	if (!m_pD3DDevice || !m_pD3DDeviceCtx)
		return false;

	// set up the viewport
	D3D11_VIEWPORT vp;
    vp.Width = (float)m_renderWidth;
    vp.Height = (float)m_renderHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pD3DDeviceCtx->RSSetViewports( 1, &vp );

	const MMatrix & view = info.viewMatrix(); 
	const MMatrix & projection = info.projectionMatrix();

	// Double to float conversion
	m_currentViewMatrix = XMMATRIX( (float)view.matrix[0][0], (float)view.matrix[0][1], (float)view.matrix[0][2], (float)view.matrix[0][3], 
		(float)view.matrix[1][0], (float)view.matrix[1][1], (float)view.matrix[1][2], (float)view.matrix[1][3], 
		(float)view.matrix[2][0], (float)view.matrix[2][1], (float)view.matrix[2][2], (float)view.matrix[2][3], 
		(float)view.matrix[3][0], (float)view.matrix[3][1], (float)view.matrix[3][2], (float)view.matrix[3][3]);

	m_currentProjectionMatrix = XMMATRIX( (float)projection.matrix[0][0], (float)projection.matrix[0][1], (float)projection.matrix[0][2], (float)projection.matrix[0][3], 
		(float)projection.matrix[1][0], (float)projection.matrix[1][1], (float)projection.matrix[1][2], (float)projection.matrix[1][3], 
		(float)projection.matrix[2][0], (float)projection.matrix[2][1], (float)projection.matrix[2][2], (float)projection.matrix[2][3], 
		(float)projection.matrix[3][0], (float)projection.matrix[3][1], (float)projection.matrix[3][2], (float)projection.matrix[3][3]);

	return true;
}

bool DX12ViewportRenderer::readFromTargetToSystemMemory()
//
// Description:
//		Read back render target memory into system memory to 
//		transfer back to calling code.
// 
{
	if (!m_pD3DDevice || !m_pTextureOutput || m_renderWidth==0 || m_renderHeight == 0 ||
		!m_pTextureReadBack)
		return false;

	bool readBuffer = false;

	HRESULT hr;

	// Dump to file option for debugging purposes.
	//
#if defined(_DUMP_SURFACE_READBACK_CONTENTS_)
	bool dumpToFile= false;
	if (dumpToFile)
	{
		const char fileName[] = "c:\\temp\\d3dDump.jpg";
		HRESULT hres = D3DXSaveSurfaceToFile( fileName, D3DXIFF_JPG,
							m_pTextureOutputSurface, NULL /*palette*/, NULL /*rect*/ );
		if (hres != D3D_OK)
		{
			MGlobal::displayWarning("DX11 renderer : Failed to dump surface contents to file !\n");
		}
	}
#endif

	m_pD3DDeviceCtx->CopyResource( m_pTextureReadBack, m_pTextureOutput );

	D3D11_MAPPED_SUBRESOURCE resource;
	hr = m_pD3DDeviceCtx->Map( m_pTextureReadBack, 0, D3D11_MAP_READ, 0, &resource );
	if ( FAILED(hr) )
	{
		MGlobal::displayError("DX11 Renderer : Could not map resource for readback\n");
		return false;
	}

			INT pitch = resource.RowPitch;
			BYTE *data = (BYTE *)resource.pData;

			// ** Magic number warning ***
			// We use D3DFMT_A8R8G8B8 as the buffer format for now as we
			// assume 32 bits per pixel = 4 bytes per pixel. Will need to
			// change when buffer format changes possibly be float.
			//
			const unsigned int bytesPerPixel = 4;

			// Reallocate buffer block as required.
			unsigned int m_readBackBufferWidth = 0;
			unsigned int m_readBackBufferHeight = 0;
			m_readBackBuffer.getSize(m_readBackBufferWidth, m_readBackBufferHeight);

			BYTE *m_readBackBufferPtr = NULL;
			bool replaceReadBackBuffer = false;

			if (!m_readBackBufferWidth || !m_readBackBufferHeight ||
				m_readBackBufferWidth != m_renderWidth ||
				m_readBackBufferHeight != m_renderHeight)
			{
				// This crashes Maya. Need to figure out why ?????
				m_readBackBuffer.resize(m_renderWidth, m_renderHeight, false);
				m_readBackBuffer.getSize(m_readBackBufferWidth, m_readBackBufferHeight);
				if (m_readBackBufferWidth != m_renderWidth ||
					m_readBackBufferHeight != m_renderHeight)
				{
					MGlobal::displayError("D3D Renderer : Could not resize MImage buffer for readback !\n");
					return false;
				}
				m_readBackBufferPtr = (BYTE *)(m_readBackBuffer.pixels());
			}
			else
				m_readBackBufferPtr = (BYTE *)(m_readBackBuffer.pixels());

			if (m_readBackBufferPtr)
			{
				// Copy a row at a time.
				// The jump by pitch, may differ if pitch is not the same as width.
				//
				unsigned int myLineSize = m_renderWidth * bytesPerPixel;
				unsigned int offsetMyData = (m_renderHeight-1) * myLineSize;
				unsigned int offsetData = 0;

				unsigned int i;
				for ( i=0 ; i < m_renderHeight; i++ )
				{
					memcpy( m_readBackBufferPtr + offsetMyData, 
						data + offsetData, 
						myLineSize );
					offsetMyData -= myLineSize;
					offsetData += pitch;
				}

				readBuffer = true;
			}

			if (replaceReadBackBuffer)
			{
				m_readBackBuffer.setPixels( m_readBackBufferPtr, m_renderWidth,
					m_renderHeight );
				delete[] m_readBackBufferPtr;					
			}
			m_readBackBufferPtr = 0;

	m_pD3DDeviceCtx->Unmap( m_pTextureReadBack, 0 );

	return readBuffer;
}

#endif



