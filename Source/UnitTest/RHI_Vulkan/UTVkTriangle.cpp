#include <Kaleido3D.h>
#include <Core/App.h>
#include <Core/File.h>
#include <Core/Message.h>
#include <RHI/IRHI.h>
#include <Renderer/RenderContext.h>
#include <Renderer/Render.h>
#include "RHI/Vulkan/VkCommon.h"
#include "RHI/Vulkan/Public/VkRHI.h"

using namespace k3d;
using namespace render;

class TriangleMesh;

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

	void PrepareMesh();
	void PreparePipeline();
	void PrepareCommandBuffer();

private:

	std::unique_ptr<TriangleMesh> m_TriMesh;
	RenderContext			m_RenderContext;
	rhi::IRenderViewport *	m_Viewport;
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

	explicit TriangleMesh(rhi::IDevice * device) : m_pDevice (device) 
	{
		m_szVBuf = sizeof(TriangleMesh::Vertex)*m_VertexBuffer.size();
		m_szIBuf = sizeof(uint32)*m_IndexBuffer.size();
		m_VertDecs[0] = { rhi::EVF_Float3x32, sizeof(Vertex), 0,0,0 };					/* Position */
		m_VertDecs[1] = { rhi::EVF_Float3x32, sizeof(Vertex), 1,sizeof(float)*3,0 };	/* Color */
	}

	const rhi::VertexDeclaration * GetVertDec() const { return m_VertDecs; }

	void	Upload();
	void*	VertexBuffer() override { return nullptr; }

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
	rhi::IDevice * m_pDevice;
};

void TriangleMesh::Upload() 
{
	rhi::ResourceDesc desc;
	rhi::IGpuResource * vbuf = m_pDevice->NewGpuResource(desc, m_szVBuf);
	void * ptr = vbuf->Map(0, m_szVBuf);
	memcpy(ptr, &m_VertexBuffer[0], m_szVBuf);
	vbuf->UnMap();

	rhi::IGpuResource * ibuf = m_pDevice->NewGpuResource(desc, m_szIBuf);
	ptr = ibuf->Map(0, m_szIBuf);
	memcpy(ptr, &m_IndexBuffer[0], m_szIBuf);
	ibuf->UnMap();

	Log::Out(LogLevel::Info, "TriangleMesh", "finish buffer upload..");
}


bool VkTriangleUnitTest::OnInit()
{
	App::OnInit();
	m_RenderContext.Init(RHIType::ERTVulkan);
	m_RenderContext.Attach(HostWindow());

	PrepareMesh();
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

void VkTriangleUnitTest::PrepareMesh()
{
	m_TriMesh = std::make_unique<TriangleMesh>(m_RenderContext.GetDevice());
	m_TriMesh->Upload();
}

void VkTriangleUnitTest::PreparePipeline()
{
	auto pDevice = m_RenderContext.GetDevice();
	IShaderCompilerOutput * vertSh = compile("../../Data/Test/triangle.vert", rhi::ES_Vertex);
	IShaderCompilerOutput * fragSh = compile("../../Data/Test/triangle.frag", rhi::ES_Fragment);

	rhi::PipelineDesc desc;
	desc.Shaders[rhi::ES_Vertex] = vertSh->GetByteCode();
	desc.Shaders[rhi::ES_Fragment] = vertSh->GetByteCode();
	desc.VertexLayout.Append(m_TriMesh->GetVertDec()[0])
		.Append(m_TriMesh->GetVertDec()[1]);

	static_cast<vk::Device*>(pDevice)->CreatePipelineStateObject(desc);

	auto pPipelineState = pDevice->NewPipelineState(rhi::EPSO_Graphics);
	pPipelineState->SetShader(rhi::EShaderType::ES_Vertex, vertSh);
	pPipelineState->SetShader(rhi::EShaderType::ES_Fragment, fragSh);
	pPipelineState->SetVertexInputLayout(m_TriMesh->GetVertDec(), 2);
	pPipelineState->SetBlendState(rhi::BlendState());
	pPipelineState->SetDepthStencilState(rhi::DepthStencilState());
	pPipelineState->SetRasterizerState(rhi::RasterizerState());
	pPipelineState->SetPrimitiveTopology(rhi::EPrimitiveType::EPT_Triangles);
	pPipelineState->Finalize();
}

void VkTriangleUnitTest::PrepareCommandBuffer()
{

}

void VkTriangleUnitTest::OnDestroy()
{
	m_RenderContext.Destroy();
}

void VkTriangleUnitTest::OnProcess(Message& msg)
{
/*
	cmd = CommandContext::Begin(pDevice, pQueue)
	cmd.BindGraphicsPipeline()
	cmd.SetIndexBuffer(ibo)
	cmd.SetVertexBuffer(vbo)
	cmd.DrawIndex() / ExecuteCmdBuf()
	cmd.FlushAndWait()

	swapChain.Present(pQueue, pImage, pWindow[, semaphore])
*/
}