#include <Kaleido3D.h>
#include <Core/App.h>
#include <Core/File.h>
#include <Core/AssetManager.h>
#include <Core/LogUtil.h>
#include <Core/Message.h>
#include <RHI/IRHI.h>
#include <Renderer/RenderContext.h>
#include <Renderer/Render.h>
#include <Math/kMath.hpp>

using namespace k3d;
using namespace render;
using namespace kMath;

class TriangleMesh;

struct ConstantBuffer {
	Mat4f projectionMatrix;
	Mat4f modelMatrix;
	Mat4f viewMatrix;
};

class VkTriangleUnitTest : public App
{
public:
	explicit VkTriangleUnitTest(kString const & appName)
		: App(appName, 1920, 1080)
	{}

	bool OnInit() override;
	void OnDestroy() override;
	void OnProcess(Message & msg) override;

	void OnUpdate() override;

protected:
	IShaderCompilerOutput * compile(const char *shaderPath, rhi::EShaderType const& type);

	void PrepareResource();
	void PreparePipeline();
	void PrepareCommandBuffer();

private:

	std::unique_ptr<TriangleMesh>		m_TriMesh;

	rhi::IGpuResource*					m_ConstBuffer;
	ConstantBuffer						m_HostBuffer;

	RenderContext						m_RenderContext;
	rhi::IRenderViewport *				m_Viewport;
	rhi::IPipelineStateObject*			m_pPso;
	rhi::IPipelineLayout*				m_pl;
	std::vector<rhi::ICommandContext*>	m_Cmds;
	rhi::ISyncFence*					m_pFence;
	std::vector<rhi::ICommandContext*>	m_PostCmd;
};

K3D_APP_MAIN(VkTriangleUnitTest);

class TriangleMesh : public render::Mesh
{
public:
	struct Vertex {
		float pos[3];
		float col[3];
	};
	typedef std::vector<Vertex> VertexList;
	typedef std::vector<uint32> IndiceList;

	explicit TriangleMesh(rhi::IDevice * device) : m_pDevice (device), vbuf(nullptr), ibuf(nullptr)
	{
		m_szVBuf = sizeof(TriangleMesh::Vertex)*m_VertexBuffer.size();
		m_szIBuf = sizeof(uint32)*m_IndexBuffer.size();
		m_VertDecs[0] = { rhi::EVF_Float3x32, sizeof(Vertex), 0,0,0 };					/* Position */
		m_VertDecs[1] = { rhi::EVF_Float3x32, sizeof(Vertex), 1,sizeof(float)*3,0 };	/* Color */
	}

	~TriangleMesh()
	{
		if (vbuf)
			delete vbuf;
		if (ibuf)
			delete ibuf;
	}

	const rhi::VertexDeclaration * GetVertDec() const { return m_VertDecs; }

	void	Upload();
	void*	VertexBuffer() override { return nullptr; }

	void SetLoc(uint64 ibo, uint64 vbo)
	{
		iboLoc = ibo;
		vboLoc = vbo; 
	}

	const rhi::VertexBufferView VBO() const { return rhi::VertexBufferView{ vboLoc, 0, 0 }; }

	const rhi::IndexBufferView IBO() const { return rhi::IndexBufferView{ iboLoc, 0 }; }

private:

	rhi::VertexDeclaration m_VertDecs[2];

	uint64 m_szVBuf;
	uint64 m_szIBuf;

	VertexList m_VertexBuffer = {
		{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
	};
	IndiceList m_IndexBuffer = { 0, 1, 2 };

	uint64 iboLoc;
	uint64 vboLoc;

	rhi::IDevice * m_pDevice;
	rhi::IGpuResource*		vbuf, *ibuf;
};

void TriangleMesh::Upload() 
{
	// create stage buffers
	rhi::ResourceDesc stageDesc;
	stageDesc.ViewType = rhi::EGpuMemViewType::EGVT_Undefined;
	stageDesc.CreationFlag = rhi::EGpuResourceCreationFlag::EGRCF_TransferSrc;
	stageDesc.Flag = (rhi::EGpuResourceAccessFlag) (rhi::EGpuResourceAccessFlag::EGRAF_HostCoherent | rhi::EGpuResourceAccessFlag::EGRAF_HostVisible);
	stageDesc.Size = m_szVBuf;
	rhi::IGpuResource* vStageBuf = m_pDevice->NewGpuResource(stageDesc);
	void * ptr = vStageBuf->Map(0, m_szVBuf);
	memcpy(ptr, &m_VertexBuffer[0], m_szVBuf);
	vStageBuf->UnMap();

	stageDesc.Size = m_szIBuf;
	rhi::IGpuResource* iStageBuf = m_pDevice->NewGpuResource(stageDesc);
	ptr = iStageBuf->Map(0, m_szIBuf);
	memcpy(ptr, &m_IndexBuffer[0], m_szIBuf);
	iStageBuf->UnMap();

	rhi::ResourceDesc bufferDesc;
	bufferDesc.ViewType = rhi::EGpuMemViewType::EGVT_VBV;
	bufferDesc.Size = m_szVBuf;
	bufferDesc.CreationFlag = rhi::EGpuResourceCreationFlag::EGRCF_TransferDst;
	bufferDesc.Flag = rhi::EGpuResourceAccessFlag::EGRAF_DeviceVisible;
	vbuf = m_pDevice->NewGpuResource(bufferDesc);
	bufferDesc.ViewType = rhi::EGpuMemViewType::EGVT_IBV;
	bufferDesc.Size = m_szIBuf;
	ibuf = m_pDevice->NewGpuResource(bufferDesc);

	auto cmd = m_pDevice->NewCommandContext(rhi::ECMD_Graphics);
	rhi::BufferRegion region = {0,0, m_szVBuf};
	DynArray<rhi::BufferRegion> regions;
	regions.Append(region);
	cmd->Begin();
	cmd->CopyBuffer(*vbuf, *vStageBuf, regions);
	regions[0].CopySize = m_szIBuf;
	cmd->CopyBuffer(*ibuf, *iStageBuf, regions);
	cmd->End();
	cmd->Execute(true);
//	m_pDevice->WaitIdle();
	uint64 vboLoc = vbuf->GetResourceLocation();
	uint64 iboLoc = ibuf->GetResourceLocation();
	SetLoc(iboLoc, vboLoc);
	KLOG(Info, TriangleMesh, "finish buffer upload..");
}


bool VkTriangleUnitTest::OnInit()
{
	bool inited = App::OnInit();
	if(!inited)
		return inited;
	m_RenderContext.Init(RHIType::ERTVulkan);
	m_RenderContext.Attach(HostWindow());
	m_Viewport = m_RenderContext.GetViewport();

	KLOG(Info, Test, __K3D_FUNC__);
	PrepareResource();
	PreparePipeline();
	PrepareCommandBuffer();

	return true;
}

IShaderCompilerOutput * 
VkTriangleUnitTest::compile(const char * shaderPath, rhi::EShaderType const & type)
{
	auto pDevice = m_RenderContext.GetDevice();
	ShaderCompilerOption vertOpt = { type, "", "main" };
	IShaderCompiler * pCompiler = pDevice->NewShaderCompiler();
	
	IAsset * shaderFile = AssetManager::Open(shaderPath);
	if (!shaderFile)
	{
		Log::Out(LogLevel::Fatal, "VkTriangle", "Error opening %s.", shaderPath);
		return nullptr;
	}

	return pCompiler->Compile(vertOpt, (const char*)shaderFile->GetBuffer());;
}

void VkTriangleUnitTest::PrepareResource()
{
	KLOG(Info, Test, __K3D_FUNC__);
	m_TriMesh = std::make_unique<TriangleMesh>(m_RenderContext.GetDevice());
	m_TriMesh->Upload();

	auto pDevice = m_RenderContext.GetDevice();
	rhi::ResourceDesc desc;
	desc.Flag = rhi::EGpuResourceAccessFlag::EGRAF_HostVisible;
	desc.ViewType = rhi::EGpuMemViewType::EGVT_CBV;
	desc.Size = sizeof(ConstantBuffer);
	m_ConstBuffer = pDevice->NewGpuResource(desc);
	OnUpdate();
//	pDevice->WaitIdle(); TOFIX: this may cause crash on Android N
}

void VkTriangleUnitTest::PreparePipeline()
{
	auto pDevice = m_RenderContext.GetDevice();
	IShaderCompilerOutput * vertSh = compile("asset://Test/triangle.vert", rhi::ES_Vertex);
	IShaderCompilerOutput * fragSh = compile("asset://Test/triangle.frag", rhi::ES_Fragment);
	rhi::PipelineLayoutDesc ppldesc = vertSh->GetBindingTable();
	m_pl = pDevice->NewPipelineLayout(ppldesc);
	rhi::IDescriptor * descriptor = m_pl->GetDescriptorSet();
	descriptor->Update(0, m_ConstBuffer);
	auto attrib = vertSh->GetAttributes();
	rhi::PipelineDesc desc;
	desc.Shaders[rhi::ES_Vertex] = vertSh->GetByteCode();
	desc.Shaders[rhi::ES_Fragment] = fragSh->GetByteCode();
	desc.VertexLayout.Append(m_TriMesh->GetVertDec()[0]).Append(m_TriMesh->GetVertDec()[1]);
	m_pPso = pDevice->NewPipelineState(desc, m_pl, rhi::EPSO_Graphics);

}

void VkTriangleUnitTest::PrepareCommandBuffer()
{
	auto pDevice = m_RenderContext.GetDevice();
	m_pFence = pDevice->NewFence();
	for (uint32 i = 0; i < m_Viewport->GetSwapChainCount(); i++)
	{
		auto pRT = m_Viewport->GetRenderTarget(i);
		auto gfxCmd = pDevice->NewCommandContext(rhi::ECMD_Graphics);
		gfxCmd->Begin();
		gfxCmd->SetPipelineLayout(m_pl);
		rhi::Rect rect{ 0,0, (long)m_Viewport->GetWidth(), (long)m_Viewport->GetHeight() };
		gfxCmd->SetRenderTarget(pRT);
		gfxCmd->SetScissorRects(1, &rect);
		gfxCmd->SetViewport(rhi::ViewportDesc(m_Viewport->GetWidth(), m_Viewport->GetHeight()));
		gfxCmd->SetPipelineState(0, m_pPso);
		gfxCmd->SetIndexBuffer(m_TriMesh->IBO());
		gfxCmd->SetVertexBuffer(0, m_TriMesh->VBO());
		gfxCmd->DrawIndexedInstanced(rhi::DrawIndexedInstancedParam(3, 1));
		gfxCmd->EndRendering();
		gfxCmd->TransitionResourceBarrier(pRT->GetBackBuffer(), rhi::ERS_RenderTarget, rhi::ERS_Present);
		gfxCmd->End();
		m_Cmds.push_back(gfxCmd);
		m_PostCmd.push_back(pDevice->NewCommandContext(rhi::ECMD_Graphics));
		m_PostCmd[i]->Begin();
		m_PostCmd[i]->TransitionResourceBarrier(pRT->GetBackBuffer(), rhi::ERS_Unknown, rhi::ERS_RenderTarget);
		m_PostCmd[i]->End();
	}

}

void VkTriangleUnitTest::OnDestroy()
{
	m_TriMesh->~TriangleMesh();
	if (m_pPso) {
		delete m_pPso;
	}
	m_RenderContext.Destroy();
}

void VkTriangleUnitTest::OnProcess(Message& msg)
{
	KLOG(Info, VkTriangleUnitTest, __K3D_FUNC__);
	m_Viewport->PrepareNextFrame();
	m_PostCmd[m_Viewport->GetSwapChainIndex()]->Execute(true);
	m_Cmds[m_Viewport->GetSwapChainIndex()]->PresentInViewport(m_Viewport);
}

void VkTriangleUnitTest::OnUpdate()
{
	m_HostBuffer.projectionMatrix = Perspective(60.0f, (float)1920 / (float)1080, 0.1f, 256.0f);
	m_HostBuffer.viewMatrix = Translate(Vec3f(0.0f, 0.0f, -2.5f), MakeIdentityMatrix<float>());
	m_HostBuffer.modelMatrix = MakeIdentityMatrix<float>();
	m_HostBuffer.modelMatrix = Rotate(Vec3f(1.0f, 0.0f, 0.0f), 0.f, m_HostBuffer.modelMatrix);
	m_HostBuffer.modelMatrix = Rotate(Vec3f(0.0f, 1.0f, 0.0f), 0.f, m_HostBuffer.modelMatrix);
	m_HostBuffer.modelMatrix = Rotate(Vec3f(0.0f, 0.0f, 1.0f), 0.f, m_HostBuffer.modelMatrix);
	void * ptr = m_ConstBuffer->Map(0, sizeof(ConstantBuffer));
	memcpy(ptr, &m_HostBuffer, sizeof(ConstantBuffer));
	m_ConstBuffer->UnMap();
}
