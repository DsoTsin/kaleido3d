#ifndef DX11ResourceManager_h_
#define DX11ResourceManager_h_

#include <maya/MViewportRenderer.h>
#include <maya/MDagPath.h>
#include <maya/MObjectHandle.h>
#include <maya/MMessage.h> // For monitoring geometry list nodes
#include <maya/MStringArray.h>
#include <string>
#include <map>

// Resources
#include "DX11GeometryItem.h"
#include "DX11TextureItem.h"

#define DX11_SUPPORTED
#if defined(DX11_SUPPORTED)
#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>
	// for VS 2012, Win8 SDK includes DX sdk with some headers removed
	#if _MSC_VER >= 1700
		#include <dxgi.h>
	#else
		#include <d3dx11.h>
	#endif

#include <d3dcompiler.h>
#include <xnamath.h>

struct CameraItem {
	// View matrix parameters
	XMFLOAT3				m_vEyePt;		// Eye position
    XMFLOAT3				m_vLookatPt;	// Look at position
    XMFLOAT3				m_vUpVec;		// Up vector

	// Projection matrix parameters
	float					m_FieldOfView;	// Field of view (in degrees)
	float					m_nearClip;		// Near clip plane
	float					m_farClip;		// Far clip plane
	bool					m_isOrtho;		// Is orthographic
};

// Surface effect item
struct SurfaceEffectItem
{
	ID3D11VertexShader*	fVertexShader;
	ID3D11PixelShader*	fPixelShader;
	ID3D11InputLayout*	fInputLayout;
};
typedef std::map<std::string, SurfaceEffectItem*> SurfaceEffectItemList;

class MDagPath;
class MObject;
class MRenderingInfo;

//
// Manager for D3D resources
//
class DX11ResourceManager 
{
public:
	DX11ResourceManager();
	virtual ~DX11ResourceManager();

	D3DGeometry*			getGeometry( const MDagPath& dagPath, ID3D11Device* D3D);
	D3DTexture*				getTexture( MObject& mayaTexture);

	void					clearResources(bool onlyInvalidItems, bool clearShaders);

	bool					translateCamera( const MDagPath &cameraPath);
	CameraItem*				getCamera() { return &m_camera; }

	bool					initializeDefaultSurfaceEffect( const MString &effectsLocation, ID3D11Device* D3D,
															const MString &effectName,
															const MString& vsName, const MString& psName,
															const D3D11_INPUT_ELEMENT_DESC* layout, int numLayoutElements );
	const SurfaceEffectItemList & getSurfaceEffectItemList() const
	{
		return m_SurfaceEffectItemList;
	}

protected:
	void					initializeDefaultCamera();

	// List of D3D geometry corresponding to Maya dag paths
	//
    GeometryItemList		m_geometryItemList;

	// List of D3D textures corresponding to Maya texture nodes
	//
    TextureItemList			m_textureItemList;

	CameraItem				m_camera;

	// List of surface effect items (by name)
	SurfaceEffectItemList	m_SurfaceEffectItemList;
};

#endif

#endif

