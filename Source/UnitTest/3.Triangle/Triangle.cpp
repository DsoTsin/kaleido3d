#include <Kaleido3D.h>
#include <Core/App.h>
#include <Core/AssetManager.h>
#include <Core/LogUtil.h>
#include <Core/Message.h>
#include <RHI/IRHI.h>
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
const char* vertSource = "#version 430\n"
		"\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"\n"
		"layout (location = 0) in vec3 inPos;\n"
		"layout (location = 1) in vec3 inColor;\n"
		"\n"
		"layout (binding = 0) uniform UBO \n"
		"{\n"
		"\tmat4 projectionMatrix;\n"
		"\tmat4 modelMatrix;\n"
		"\tmat4 viewMatrix;\n"
		"} ubo;\n"
		"\n"
		"layout (location = 0) out vec3 outColor;\n"
		"\n"
		"out gl_PerVertex \n"
		"{\n"
		"    vec4 gl_Position;   \n"
		"};\n"
		"\n"
		"\n"
		"void main() \n"
		"{\n"
		"\toutColor = inColor;\n"
		"\tgl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);\n"
		"}";
const char* fragSource = "#version 430\n"
		"\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"\n"
		"layout (location = 0) in vec3 inColor;\n"
		"\n"
		"layout (location = 0) out vec4 outFragColor;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  outFragColor = vec4(inColor, 1.0);\n"
		"}";
class VkTriangleUnitTest : public App
{
public:
	VkTriangleUnitTest(kString const & appName, uint32 width, uint32 height)
			: App(appName, width, height), m_Width(width), m_Height(height)
	{}
	explicit VkTriangleUnitTest(kString const & appName)
		: App(appName, 1920, 1080), m_Width(1920), m_Height(1080)
	{}

	bool OnInit() override;
	void OnDestroy() override;
	void OnProcess(Message & msg) override;

	void OnUpdate() override;

protected:
	void LoadGlslangCompiler();
	void compile(const char *shaderSource, rhi::EShaderType const& type, rhi::ShaderBundle & shader);

	void PrepareResource();
	void PreparePipeline();
	void PrepareCommandBuffer();

private:
	rhi::IShCompiler::Ptr				m_Compiler;
	std::unique_ptr<TriangleMesh>		m_TriMesh;

	rhi::GpuResourceRef					m_ConstBuffer;
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

K3D_APP_MAIN(VkTriangleUnitTest)

class TriangleMesh
{
public:
	struct Vertex {
		float pos[3];
		float col[3];
	};
	typedef std::vector<Vertex> VertexList;
	typedef std::vector<uint32> IndiceList;

	explicit TriangleMesh(rhi::DeviceRef device) : m_pDevice (device), vbuf(nullptr), ibuf(nullptr)
	{
		m_szVBuf = sizeof(TriangleMesh::Vertex)*m_VertexBuffer.size();
		m_szIBuf = sizeof(uint32)*m_IndexBuffer.size();
		m_VertDecs[0] = { rhi::EVF_Float3x32, sizeof(Vertex), 0,0,0 };					/* Position */
		m_VertDecs[1] = { rhi::EVF_Float3x32, sizeof(Vertex), 1,sizeof(float)*3,0 };	/* Color */
	}

	~TriangleMesh()
	{
	}

	const rhi::VertexDeclaration * GetVertDec() const { return m_VertDecs; }

	void Upload();

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

	rhi::DeviceRef			m_pDevice;
	rhi::GpuResourceRef		vbuf, ibuf;
};

void TriangleMesh::Upload() 
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

	stageDesc.Size = m_szIBuf;
	auto iStageBuf = m_pDevice->NewGpuResource(stageDesc);
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
	rhi::CopyBufferRegion region = {0,0, m_szVBuf};
	cmd->Begin();
	cmd->CopyBuffer(*vbuf, *vStageBuf, region);
	region.CopySize = m_szIBuf;
	cmd->CopyBuffer(*ibuf, *iStageBuf, region);
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
	m_RenderContext.Init(RHIType::ERTVulkan, m_Width, m_Height);
	m_RenderContext.Attach(HostWindow());
	m_Viewport = m_RenderContext.GetViewport();
	LoadGlslangCompiler();
	KLOG(Info, Test, __K3D_FUNC__);
	PrepareResource();
	PreparePipeline();
	PrepareCommandBuffer();

	return true;
}

void VkTriangleUnitTest::LoadGlslangCompiler()
{
	rhi::IShModule* shMod = (rhi::IShModule*)ACQUIRE_PLUGIN(ShaderCompiler);
	if (shMod)
	{
		m_Compiler = shMod->CreateShaderCompiler(rhi::ERHI_Vulkan);
	}
}

void VkTriangleUnitTest::compile(const char * shaderPath, rhi::EShaderType const & type, rhi::ShaderBundle & shader)
{
	auto pDevice = m_RenderContext.GetDevice();
	IAsset * shaderFile = AssetManager::Open(shaderPath);
	if (!shaderFile)
	{
		KLOG(Fatal, "VkTriangle", "Error opening %s.", shaderPath);
		return;
	}
	std::vector<char> buffer;
	uint64 len = shaderFile->GetLength();
	buffer.resize(len+1);
	shaderFile->Read(buffer.data(), shaderFile->GetLength());
	buffer[len] = 0;
	String src(buffer.data());
	rhi::ShaderDesc desc = { rhi::EShFmt_Text, rhi::EShLang_GLSL, rhi::EShProfile_Modern, type, "main" };
	m_Compiler->Compile(src, desc, shader);
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
	rhi::ShaderBundle vertSh, fragSh;
	compile("asset://Test/triangle.vert", rhi::ES_Vertex, vertSh);
	compile("asset://Test/triangle.frag", rhi::ES_Fragment, fragSh);
	rhi::PipelineLayoutDesc ppldesc = vertSh.BindingTable;
	m_pl = pDevice->NewPipelineLayout(ppldesc);
	auto descriptor = m_pl->GetDescriptorSet();
	descriptor->Update(0, m_ConstBuffer);
	auto attrib = vertSh.Attributes;
	rhi::PipelineDesc desc;
	desc.Shaders[rhi::ES_Vertex] = vertSh;
	desc.Shaders[rhi::ES_Fragment] = fragSh;
	desc.VertexLayout.Append(m_TriMesh->GetVertDec()[0]).Append(m_TriMesh->GetVertDec()[1]);
	m_pPso = pDevice->NewPipelineState(desc, m_pl, rhi::EPSO_Graphics);

}

void VkTriangleUnitTest::PrepareCommandBuffer()
{
	auto pDevice = m_RenderContext.GetDevice();
	m_pFence = pDevice->NewFence();
	/*
	for (uint32 i = 0; i < m_Viewport->GetSwapChainCount(); i++)
	{
		auto pRT = m_Viewport->GetRenderTarget(i);
		auto gfxCmd = pDevice->NewCommandContext(rhi::ECMD_Graphics);
		gfxCmd->Begin();
		gfxCmd->TransitionResourceBarrier(pRT->GetBackBuffer(), rhi::EPS_All, rhi::ERS_RenderTarget);
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
		gfxCmd->TransitionResourceBarrier(pRT->GetBackBuffer(), rhi::EPS_All, rhi::ERS_Present);
		gfxCmd->End();
		m_Cmds.push_back(gfxCmd);
	}
	*/
}

void VkTriangleUnitTest::OnDestroy()
{
	m_TriMesh->~TriangleMesh();
	m_RenderContext.Destroy();
}

void VkTriangleUnitTest::OnProcess(Message& msg)
{
	KLOG(Info, VkTriangleUnitTest, __K3D_FUNC__);
	m_Viewport->PrepareNextFrame();
	auto curRt = m_Viewport->GetCurrentBackRenderTarget();
	auto pDevice = m_RenderContext.GetDevice();
	auto renderCmd = pDevice->NewCommandContext(rhi::ECMD_Graphics);
	renderCmd->Begin();
	renderCmd->TransitionResourceBarrier(curRt->GetBackBuffer(), rhi::ERS_RenderTarget);
	renderCmd->SetPipelineLayout(m_pl);
	rhi::Rect rect{ 0,0, (long)m_Viewport->GetWidth(), (long)m_Viewport->GetHeight() };
	renderCmd->SetRenderTarget(curRt);
	renderCmd->SetScissorRects(1, &rect);
	renderCmd->SetViewport(rhi::ViewportDesc(m_Viewport->GetWidth(), m_Viewport->GetHeight()));
	renderCmd->SetPipelineState(0, m_pPso);
	renderCmd->SetIndexBuffer(m_TriMesh->IBO());
	renderCmd->SetVertexBuffer(0, m_TriMesh->VBO());
	renderCmd->DrawIndexedInstanced(rhi::DrawIndexedInstancedParam(3, 1));
	renderCmd->EndRendering();
	renderCmd->TransitionResourceBarrier(curRt->GetBackBuffer(), rhi::ERS_Present);
	renderCmd->End();
	renderCmd->PresentInViewport(m_Viewport);
}

void VkTriangleUnitTest::OnUpdate()
{
	m_HostBuffer.projectionMatrix = Perspective(60.0f, (float)m_Viewport->GetWidth() / (float)m_Viewport->GetHeight(), 0.1f, 256.0f);
	m_HostBuffer.viewMatrix = Translate(Vec3f(0.0f, 0.0f, -2.5f), MakeIdentityMatrix<float>());
	m_HostBuffer.modelMatrix = MakeIdentityMatrix<float>();
	m_HostBuffer.modelMatrix = Rotate(Vec3f(1.0f, 0.0f, 0.0f), 0.f, m_HostBuffer.modelMatrix);
	m_HostBuffer.modelMatrix = Rotate(Vec3f(0.0f, 1.0f, 0.0f), 0.f, m_HostBuffer.modelMatrix);
	m_HostBuffer.modelMatrix = Rotate(Vec3f(0.0f, 0.0f, 1.0f), 0.f, m_HostBuffer.modelMatrix);
	void * ptr = m_ConstBuffer->Map(0, sizeof(ConstantBuffer));
	memcpy(ptr, &m_HostBuffer, sizeof(ConstantBuffer));
	m_ConstBuffer->UnMap();
}
