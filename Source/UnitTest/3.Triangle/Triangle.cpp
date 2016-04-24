#include <Kaleido3D.h>
#include <Core/App.h>
#include <Core/File.h>
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
	rhi::ResourceDesc desc;
	desc.ViewType = rhi::EGpuMemViewType::EGVT_VBV;
	desc.Size = m_szVBuf;
	vbuf = m_pDevice->NewGpuResource(desc);
	void * ptr = vbuf->Map(0, m_szVBuf);
	memcpy(ptr, &m_VertexBuffer[0], m_szVBuf);
	vbuf->UnMap();

	uint64 vboLoc = vbuf->GetResourceLocation();

	desc.ViewType = rhi::EGpuMemViewType::EGVT_IBV;
	desc.Size = m_szIBuf;
	ibuf = m_pDevice->NewGpuResource(desc);
	ptr = ibuf->Map(0, m_szIBuf);
	memcpy(ptr, &m_IndexBuffer[0], m_szIBuf);
	ibuf->UnMap();

	uint64 iboLoc = ibuf->GetResourceLocation();

	SetLoc(iboLoc, vboLoc);

	KLOG(Info, TriangleMesh, "finish buffer upload..");
}


bool VkTriangleUnitTest::OnInit()
{
	while(!App::OnInit());
	m_RenderContext.Init(RHIType::ERTVulkan);
	m_RenderContext.Attach(HostWindow());
	m_Viewport = m_RenderContext.GetViewport();

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
	File file;
	if (!file.Open(shaderPath, IOFlag::IORead))
	{
		Log::Out(LogLevel::Fatal, "VkTriangle", "Error opening %s.", shaderPath);
		return nullptr;
	}
	char * shSrc = new char[file.GetSize() + 1];
	shSrc[file.GetSize()] = 0;
	file.Read(shSrc, file.GetSize());

	IShaderCompilerOutput * shader = pCompiler->Compile(vertOpt, shSrc);
	file.Close();
	delete shSrc;
	return shader;
}

void VkTriangleUnitTest::PrepareResource()
{
	m_TriMesh = std::make_unique<TriangleMesh>(m_RenderContext.GetDevice());
	m_TriMesh->Upload();

	auto pDevice = m_RenderContext.GetDevice();
	rhi::ResourceDesc desc;
	desc.ViewType = rhi::EGpuMemViewType::EGVT_CBV;
	desc.Size = sizeof(ConstantBuffer);
	m_HostBuffer.projectionMatrix = Perspective(60.0f, (float)1920 / (float)1080, 0.1f, 256.0f);
	m_HostBuffer.viewMatrix = Translate(Vec3f(0.0f, 0.0f, -1.f), Mat4f());
	m_HostBuffer.modelMatrix = Mat4f();
	m_HostBuffer.modelMatrix = Rotate(Vec3f(1.0f, 0.0f, 0.0f), 0.f, m_HostBuffer.modelMatrix);
	m_HostBuffer.modelMatrix = Rotate(Vec3f(0.0f, 1.0f, 0.0f), 0.f, m_HostBuffer.modelMatrix);
	m_HostBuffer.modelMatrix = Rotate(Vec3f(0.0f, 0.0f, 1.0f), 0.f, m_HostBuffer.modelMatrix);
	m_ConstBuffer = pDevice->NewGpuResource(desc);
	void * ptr = m_ConstBuffer->Map(0, sizeof(ConstantBuffer));
	memcpy(ptr, &m_HostBuffer, sizeof(ConstantBuffer));
	m_ConstBuffer->UnMap();
}

void VkTriangleUnitTest::PreparePipeline()
{
	auto pDevice = m_RenderContext.GetDevice();
	IShaderCompilerOutput * vertSh = compile("../../Data/Test/triangle.vert", rhi::ES_Vertex);
	IShaderCompilerOutput * fragSh = compile("../../Data/Test/triangle.frag", rhi::ES_Fragment);
	rhi::PipelineLayoutDesc ppldesc = vertSh->GetBindingTable();
	m_pl = pDevice->NewPipelineLayout(ppldesc);
	
	/*File _file;
	_file.Open("../../Data/Test/triangle.vert.spv", IOWrite);
	_file.Write(vertSh->GetShaderBytes(), vertSh->GetByteCount());
	_file.Close();
	File _file2;
	_file2.Open("../../Data/Test/triangle.frag.spv", IOWrite);
	_file2.Write(fragSh->GetShaderBytes(), fragSh->GetByteCount());
	_file2.Close();*/
	auto attrib = vertSh->GetAttributes();

	rhi::PipelineDesc desc;
	desc.Shaders[rhi::ES_Vertex] = vertSh->GetByteCode();
	desc.Shaders[rhi::ES_Fragment] = fragSh->GetByteCode();
	desc.VertexLayout.Append(m_TriMesh->GetVertDec()[0]).Append(m_TriMesh->GetVertDec()[1]);
	m_pPso = pDevice->NewPipelineState(desc, m_pl, rhi::EPSO_Graphics);

	rhi::IDescriptor * descriptor = m_pl->GetDescriptorSet();
	descriptor->Update(0, m_ConstBuffer);
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
		gfxCmd->SetPipelineState(0, m_pPso);
		//Vec4f clearColor = {0.0f,0.0f,0.0f,1.0f};
		//gfxCmd->ClearColorBuffer(pRT->GetBackBuffer(), clearColor);
		rhi::Rect rect{ 0,0,1920,1080 };
		gfxCmd->SetRenderTarget(pRT);
		gfxCmd->SetScissorRects(1, &rect);
		gfxCmd->SetViewport(rhi::ViewportDesc(1920, 1080));
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
	m_PostCmd[m_Viewport->GetSwapChainIndex()]->Execute(false);
	m_Cmds[m_Viewport->GetSwapChainIndex()]->PresentInViewport(m_Viewport);
}