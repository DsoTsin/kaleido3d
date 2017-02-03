#include <Kaleido3D.h>
#include <Core/App.h>
#include <Core/AssetManager.h>
#include <Core/LogUtil.h>
#include <Core/Message.h>
#include <Renderer/Render.h>
#include <Math/kMath.hpp>
#include "TextureObject.h"

using namespace k3d;
using namespace render;
using namespace kMath;

class CubeMesh;

struct ConstantBuffer {
	Mat4f projectionMatrix;
	Mat4f modelMatrix;
	Mat4f viewMatrix;
};

class TCubeUnitTest : public App
{
public:
	TCubeUnitTest(kString const & appName, uint32 width, uint32 height)
	: App(appName, width, height), m_Width(width), m_Height(height)
	{}

	explicit TCubeUnitTest(kString const & appName)
		: App(appName, 1920, 1080)
	{}
	
	~TCubeUnitTest()
	{
		KLOG(Info, CubeTest, "Destroying..");
	}
	bool OnInit() override;
	void OnDestroy() override;
	void OnProcess(Message & msg) override;

	void OnUpdate() override;

protected:
	void LoadGlslangCompiler();
	void compile(const char *shaderPath, rhi::EShaderType const& type, rhi::ShaderBundle & bundle);
	rhi::GpuResourceRef CreateStageBuffer(uint64 size);
	void LoadTexture();
	void PrepareResource();
	void PreparePipeline();
	void PrepareCommandBuffer();

private:

	rhi::IShCompiler::Ptr				m_Compiler;
	std::unique_ptr<CubeMesh>			m_TriMesh;

	rhi::GpuResourceRef					m_ConstBuffer;
	TextureObject*						m_Texture;
	ConstantBuffer						m_HostBuffer;

	RenderContext						m_RenderContext;
	rhi::RenderViewportRef				m_Viewport;
	rhi::PipelineStateObjectRef			m_pPso;
	rhi::PipelineLayoutRef				m_pl;
	std::vector<rhi::CommandContextRef>	m_Cmds;
	rhi::SyncFenceRef					m_pFence;
	uint32 m_Width;
	uint32 m_Height;
};

K3D_APP_MAIN(TCubeUnitTest);

class CubeMesh
{
public:
	struct Vertex {
		float pos[3];
		float col[4];
		float uv[2];
	};
	typedef std::vector<Vertex> VertexList;
	typedef std::vector<uint32> IndiceList;

	explicit CubeMesh(rhi::DeviceRef device) : m_pDevice (device), vbuf(nullptr)
	{
		m_szVBuf = sizeof(CubeMesh::Vertex)*m_VertexBuffer.size();
		m_VertDecs[0] = { rhi::EVF_Float3x32, sizeof(Vertex), 0,0,0 };					/* Position */
		m_VertDecs[1] = { rhi::EVF_Float4x32, sizeof(Vertex), 1,sizeof(float)*3,0 };	/* Color */
		m_VertDecs[2] = { rhi::EVF_Float2x32, sizeof(Vertex), 2,sizeof(float)*7,0 };	/* UV */
	}

	~CubeMesh()
	{
	}

	const rhi::VertexDeclaration * GetVertDec() const { return m_VertDecs; }

	void Upload();

	void SetLoc(uint64 vbo)
	{
		vboLoc = vbo; 
	}

	const rhi::VertexBufferView VBO() const { return rhi::VertexBufferView{ vboLoc, 0, 0 }; }
	
private:

	rhi::VertexDeclaration m_VertDecs[3];

	uint64 m_szVBuf;

	VertexList m_VertexBuffer = { 
		//left
		{ -1.0f,-1.0f,-1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f },
		{ -1.0f,-1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f },
		{ -1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f },

		{ -1.0f,-1.0f,-1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f },
		{ -1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f },
		{ -1.0f, 1.0f,-1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f },

		//back
		{ 1.0f, 1.0f,-1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f },
		{ -1.0f,-1.0f,-1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f },
		{ -1.0f, 1.0f,-1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f },

		{ 1.0f, 1.0f,-1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f },
		{ 1.0f,-1.0f,-1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f },
		{ -1.0f,-1.0f,-1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f },

		//top
		{ 1.0f, -1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f },
		{ -1.0f,-1.0f,-1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f },
		{ 1.0f, -1.0f,-1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f },

		{ 1.0f, -1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f },
		{ -1.0f,-1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f },
		{ -1.0f,-1.0f,-1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f },

		//front
		{ -1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f },
		{ -1.0f,-1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f },
		{ 1.0f,-1.0f, 1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f },

		{ 1.0f, 1.0f, 1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f },
		{ 1.0f,-1.0f, 1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f },

		//right
		{ 1.0f, 1.0f, 1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f },
		{ 1.0f,-1.0f,-1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f },
		{ 1.0f, 1.0f,-1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f },

		{ 1.0f,-1.0f,-1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f },
		{ 1.0f,-1.0f, 1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f },

		//bottom
		{ 1.0f, 1.0f, 1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f },
		{ 1.0f, 1.0f,-1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f },
		{ -1.0f, 1.0f,-1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f },

		{ 1.0f, 1.0f, 1.0f,     1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f },
		{ -1.0f, 1.0f,-1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f },
	};

	rhi::GpuResourceRef		vbuf;
	uint64					vboLoc;
	rhi::DeviceRef			m_pDevice;
};

void CubeMesh::Upload()
{
	// create stage buffers
	rhi::ResourceDesc stageDesc;
	stageDesc.ViewType = rhi::EGpuMemViewType::EGVT_Undefined;
	stageDesc.CreationFlag = rhi::EGpuResourceCreationFlag::EGRCF_TransferSrc;
	stageDesc.Flag = (rhi::EGpuResourceAccessFlag) (rhi::EGpuResourceAccessFlag::EGRAF_HostCoherent | rhi::EGpuResourceAccessFlag::EGRAF_HostVisible);
	stageDesc.Size = m_szVBuf;
	auto vStageBuf = m_pDevice->NewGpuResource(stageDesc);
	void * ptr = vStageBuf->Map(0, m_szVBuf);
	memcpy(ptr, &m_VertexBuffer[0], m_szVBuf);
	vStageBuf->UnMap();

	rhi::ResourceDesc bufferDesc;
	bufferDesc.ViewType = rhi::EGpuMemViewType::EGVT_VBV;
	bufferDesc.Size = m_szVBuf;
	bufferDesc.CreationFlag = rhi::EGpuResourceCreationFlag::EGRCF_TransferDst;
	bufferDesc.Flag = rhi::EGpuResourceAccessFlag::EGRAF_DeviceVisible;
	vbuf = m_pDevice->NewGpuResource(bufferDesc);
	
	auto cmd = m_pDevice->NewCommandContext(rhi::ECMD_Graphics);
	rhi::CopyBufferRegion region = {0,0, m_szVBuf};
	cmd->Begin();
	cmd->CopyBuffer(*vbuf, *vStageBuf, region);
	cmd->End();
	cmd->Execute(true);
//	m_pDevice->WaitIdle();
	SetLoc(vbuf->GetResourceLocation());
	KLOG(Info, TCubeMesh, "finish buffer upload..");
}


bool  TCubeUnitTest::OnInit()
{
	bool inited = App::OnInit();
	if(!inited)
		return inited;
	LoadGlslangCompiler();
	m_RenderContext.Init(RHIType::ERTVulkan, m_Width, m_Height);
	m_RenderContext.Attach(HostWindow());
	m_Viewport = m_RenderContext.GetViewport();

	KLOG(Info, Test, __K3D_FUNC__);
	PrepareResource();
	PreparePipeline();
	PrepareCommandBuffer();

	return true;
}

void TCubeUnitTest::LoadGlslangCompiler()
{
	rhi::IShModule* shMod = (rhi::IShModule*)ACQUIRE_PLUGIN(ShaderCompiler);
	if (shMod)
	{
		m_Compiler = shMod->CreateShaderCompiler(rhi::ERHI_Vulkan);
	}
}

void TCubeUnitTest::compile(const char * shaderPath, rhi::EShaderType const & type, rhi::ShaderBundle & bundle)
{
	IAsset * shaderFile = AssetManager::Open(shaderPath);
	if (!shaderFile)
	{
		KLOG(Fatal, "TCube", "Error opening %s.", shaderPath);
		return;
	}
	std::vector<char> buffer;
	uint64 len = shaderFile->GetLength();
	buffer.resize(len+1);
	shaderFile->Read(buffer.data(), shaderFile->GetLength());
	buffer[len] = 0;

	String src(buffer.data());
	rhi::ShaderDesc desc = { rhi::EShFmt_Text, rhi::EShLang_GLSL, rhi::EShProfile_Modern, type, "main" };
	m_Compiler->Compile(src, desc, bundle);
}

rhi::GpuResourceRef TCubeUnitTest::CreateStageBuffer(uint64 size)
{
	rhi::ResourceDesc stageDesc;
	stageDesc.ViewType = rhi::EGpuMemViewType::EGVT_Undefined;
	stageDesc.CreationFlag = rhi::EGpuResourceCreationFlag::EGRCF_TransferSrc;
	stageDesc.Flag = (rhi::EGpuResourceAccessFlag) (rhi::EGpuResourceAccessFlag::EGRAF_HostCoherent | rhi::EGpuResourceAccessFlag::EGRAF_HostVisible);
	stageDesc.Size = size;
	return m_RenderContext.GetDevice()->NewGpuResource(stageDesc);
}

void TCubeUnitTest::LoadTexture()
{
	IAsset * textureFile = AssetManager::Open("asset://Test/bricks.tga");
	if (textureFile)
	{
 		uint64 length = textureFile->GetLength();
		uint8* data = new uint8[length];
		textureFile->Read(data, length);
		m_Texture = new TextureObject(m_RenderContext.GetDevice(), data, false);
		auto texStageBuf = CreateStageBuffer(m_Texture->GetSize());
		m_Texture->MapIntoBuffer(texStageBuf);
		m_Texture->CopyAndInitTexture(texStageBuf);
		rhi::ResourceViewDesc viewDesc;
		auto pDevice = m_RenderContext.GetDevice();
		auto srv = pDevice->NewShaderResourceView(m_Texture->GetResource(), viewDesc);
		auto texure = DynamicPointerCast<rhi::ITexture>(m_Texture->GetResource());
		texure->SetResourceView(srv);
		rhi::SamplerState samplerDesc;
		auto sampler2D = pDevice->NewSampler(samplerDesc);
		texure->BindSampler(sampler2D);
	}
}

void TCubeUnitTest::PrepareResource()
{
	KLOG(Info, Test, __K3D_FUNC__);
	m_TriMesh = std::make_unique<CubeMesh>(m_RenderContext.GetDevice());
	m_TriMesh->Upload();
	LoadTexture();
	auto pDevice = m_RenderContext.GetDevice();
	rhi::ResourceDesc desc;
	desc.Flag = rhi::EGpuResourceAccessFlag::EGRAF_HostVisible;
	desc.ViewType = rhi::EGpuMemViewType::EGVT_CBV;
	desc.Size = sizeof(ConstantBuffer);
	m_ConstBuffer = pDevice->NewGpuResource(desc);
	OnUpdate();
}

void TCubeUnitTest::PreparePipeline()
{
	auto pDevice = m_RenderContext.GetDevice();
	rhi::ShaderBundle vertSh, fragSh;
	compile("asset://Test/cube.vert", rhi::ES_Vertex, vertSh);
	compile("asset://Test/cube.frag", rhi::ES_Fragment, fragSh);
	auto vertBinding = vertSh.BindingTable;
	auto fragBinding = fragSh.BindingTable;
	auto mergedBindings = vertBinding | fragBinding;
	m_pl = pDevice->NewPipelineLayout(mergedBindings);
	auto descriptor = m_pl->GetDescriptorSet();
	descriptor->Update(0, m_ConstBuffer);
	descriptor->Update(1, m_Texture->GetResource());
	//auto attrib = vertSh->GetAttributes();
	rhi::PipelineDesc desc;
	desc.Shaders[rhi::ES_Vertex] = vertSh;
	desc.Shaders[rhi::ES_Fragment] = fragSh;
	desc.VertexLayout.Append(m_TriMesh->GetVertDec()[0]).Append(m_TriMesh->GetVertDec()[1]).Append(m_TriMesh->GetVertDec()[2]);
	m_pPso = pDevice->NewPipelineState(desc, m_pl, rhi::EPSO_Graphics);

}

void TCubeUnitTest::PrepareCommandBuffer()
{
	auto pDevice = m_RenderContext.GetDevice();
	m_pFence = pDevice->NewFence();
	for (uint32 i = 0; i < m_Viewport->GetSwapChainCount(); i++)
	{
		auto pRT = m_Viewport->GetRenderTarget(i);
		auto gfxCmd = pDevice->NewCommandContext(rhi::ECMD_Graphics);
		gfxCmd->Begin();
		gfxCmd->TransitionResourceBarrier(pRT->GetBackBuffer(), rhi::ERS_RenderTarget);
		gfxCmd->SetPipelineLayout(m_pl);
		rhi::Rect rect{ 0,0, (long)m_Viewport->GetWidth(), (long)m_Viewport->GetHeight() };
		gfxCmd->SetRenderTarget(pRT);
		gfxCmd->SetScissorRects(1, &rect);
		gfxCmd->SetViewport(rhi::ViewportDesc(1.f*m_Viewport->GetWidth(), 1.f*m_Viewport->GetHeight()));
		gfxCmd->SetPipelineState(0, m_pPso);
		gfxCmd->SetVertexBuffer(0, m_TriMesh->VBO());
		gfxCmd->DrawInstanced(rhi::DrawInstancedParam(36, 1));
		gfxCmd->EndRendering();
		gfxCmd->TransitionResourceBarrier(pRT->GetBackBuffer(), rhi::ERS_Present);
		gfxCmd->End();
		m_Cmds.push_back(gfxCmd);
	}

}

void TCubeUnitTest::OnDestroy()
{
	App::OnDestroy();
	m_pFence->WaitFor(1000);
	if (m_Texture)
	{
		delete m_Texture;
		m_Texture = nullptr;
	}

	m_TriMesh->~CubeMesh();
	m_RenderContext.Destroy();
}

void TCubeUnitTest::OnProcess(Message& msg)
{
	KLOG(Info, TCubeUnitTest, __K3D_FUNC__);
	m_Viewport->PrepareNextFrame();
	m_Cmds[m_Viewport->GetSwapChainIndex()]->PresentInViewport(m_Viewport);
}

void TCubeUnitTest::OnUpdate()
{
	m_HostBuffer.projectionMatrix = Perspective(60.0f, (float)m_Viewport->GetWidth() / (float)m_Viewport->GetHeight(), 0.1f, 256.0f);
	m_HostBuffer.viewMatrix = Translate(Vec3f(0.0f, 0.0f, -4.5f), MakeIdentityMatrix<float>());
	m_HostBuffer.modelMatrix = MakeIdentityMatrix<float>();
	static auto angle = 60.f;
#if K3DPLATFORM_OS_ANDROID
	angle += 0.1f;
#else
	angle += 0.001f;
#endif
	m_HostBuffer.modelMatrix = Rotate(Vec3f(1.0f, 0.0f, 0.0f), angle, m_HostBuffer.modelMatrix);
	m_HostBuffer.modelMatrix = Rotate(Vec3f(0.0f, 1.0f, 0.0f), angle, m_HostBuffer.modelMatrix);
	m_HostBuffer.modelMatrix = Rotate(Vec3f(0.0f, 0.0f, 1.0f), angle, m_HostBuffer.modelMatrix);
	void * ptr = m_ConstBuffer->Map(0, sizeof(ConstantBuffer));
	memcpy(ptr, &m_HostBuffer, sizeof(ConstantBuffer));
	m_ConstBuffer->UnMap();
}
