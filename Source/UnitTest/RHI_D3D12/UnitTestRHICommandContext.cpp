#include "UnitTestRHICommandContext.h"
#include "RHI/D3D12/Public/D3D12RHI.h"

using namespace rhi;

const char * source = K3D_STRINGIFY(
struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
	PSInput result;

	result.position = position;
	result.color = color;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return input.color;
}
);

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

bool UnitTestRHICommandContext::OnInit()
{
	if(!App::OnInit())
		return false; 
	if (!InitDevice())
		return false;

	InitCommandContext();
	InitSwapChain();
	InitPipeLineState();
	InitRenderResource();
	return true;
}

void UnitTestRHICommandContext::OnDestroy()
{
	if (m_TestDevice)
	{
		delete m_TestDevice;
		m_TestDevice = nullptr;
	}
	if (m_TestCommandContext) 
	{
		delete m_TestCommandContext;
		m_TestCommandContext = nullptr;
	}
	if (m_TestPipelineState)
	{
		delete m_TestPipelineState;
		m_TestPipelineState = nullptr;
	}
	App::OnDestroy();
}

void UnitTestRHICommandContext::OnProcess(Message & msg)
{
	if (msg.type == Message::Resized)
	{
		OnSizeChanged(msg.size.width, msg.size.height);
	}
}

void UnitTestRHICommandContext::OnUpdate()
{
	RenderFrame();
	m_pViewport->Present();
}

void UnitTestRHICommandContext::OnSizeChanged(int width, int height)
{
	Log::Out("SizeChanged", "%d, %d", width, height);
}

bool UnitTestRHICommandContext::InitDevice()
{
	IDeviceAdapter ** list = nullptr;
	uint32 adapterNum = 0;
	k3d::d3d12::EnumAllDeviceAdapter(list, &adapterNum);
	if (list == nullptr)
	{
		return false;
	}

	m_TestDevice = new Device;
	IDevice::Result Result = m_TestDevice->Create(list[0], true);
	if (Result != IDevice::DeviceFound)
	{
		return false;
	}
	return true;
}

void UnitTestRHICommandContext::InitCommandContext()
{
	// test [Device -> NewCommandContext] interface
	m_TestCommandContext = m_TestDevice->NewCommandContext(rhi::ECommandType::ECMD_Graphics);
}

void UnitTestRHICommandContext::InitSwapChain()
{
	m_pViewport = new D3D12Viewport(
		Device::Ptr(static_cast<Device*>(m_TestDevice)), 
		(HWND)HostWindow()->GetHandle(), 
		HostWindow()->Width(), HostWindow()->Height());
	m_pViewport->Init();
}

void UnitTestRHICommandContext::InitPipeLineState()
{
	ShaderCompiler vsCompiler;
	IShaderBytes* sbVert = vsCompiler.CompileFromSource(rhi::IShaderCompiler::HLSL_5_0, rhi::ES_Vertex, source, "VSMain");

	ShaderCompiler fsCompiler;
	IShaderBytes* sbPixel = fsCompiler.CompileFromSource(rhi::IShaderCompiler::HLSL_5_0, rhi::ES_Fragment, source, "PSMain");
	// test [Device -> NewpipelineState] interface
	m_TestPipelineState = m_TestDevice->NewPipelineState(rhi::EPipelineType::EPSO_Graphics);
	rhi::ShaderParamLayout shaderLayout;
	m_TestPipelineLayout = rhi::CreatePipelineLayout(shaderLayout);
	// test PipelineState's interfaces
	m_TestPipelineLayout->Finalize(m_TestDevice);
	m_TestPipelineState->SetLayout(m_TestPipelineLayout);
	m_TestPipelineState->SetShader(rhi::ES_Vertex, sbVert);
	m_TestPipelineState->SetShader(rhi::ES_Fragment, sbPixel);
	IGraphicsPipelineState * gPS = dynamic_cast<IGraphicsPipelineState *>(m_TestPipelineState);
	gPS->SetBlendState(rhi::BlendState());
	gPS->SetDepthStencilState(rhi::DepthStencilState());
	gPS->SetRasterizerState(rhi::RasterizerState());
	gPS->SetRenderTargetFormat(rhi::RenderTargetFormat());
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	//gPS->SetVertexInputLayout(new VertexInputLayout(inputElementDescs, 2));
	gPS->SetPrimitiveTopology(rhi::EPT_Triangles);
	m_TestPipelineState->Finalize();
}

void UnitTestRHICommandContext::InitRenderResource()
{
	m_FrameIndex = 0;
	Device * device = static_cast<Device*>(m_TestDevice);
	DescriptorHeapAllocator & dha = device->GetViewDescriptorAllocator<D3D12_RENDER_TARGET_VIEW_DESC>();
	SIZE_T outIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE handle = dha.AllocateHeapSlot(outIndex);
	for (int n = 0; n < frame_count; n++)
	{
		ThrowIfFailed(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n])));
		device->Get()->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, handle);
		handle = dha.AllocateHeapSlot(outIndex);
	}

	m_AspectRatio = 1920.f / 1080.f;

	// test [Device -> NewGpuResource] interface
	IGpuResource * GpuRes = m_TestDevice->NewGpuResource(rhi::EGT_Buffer);
	// Create the vertex buffer.
	{
		// Define the geometry for a triangle.
		Vertex triangleVertices[] =
		{
			{ { 0.0f, 0.25f * m_AspectRatio, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * m_AspectRatio, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * m_AspectRatio, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const UINT vertexBufferSize = sizeof(triangleVertices);
		/*
		GpuBuffer *resource = static_cast<GpuBuffer*>(GpuRes);
		resource->Create(KT("Triangle Vertex Buffer"), 3, sizeof(Vertex), triangleVertices);
		m_TestVertexBufferView = resource->AsVertexBufferView(0, 3 * sizeof(Vertex), sizeof(Vertex));*/
	}
}

void UnitTestRHICommandContext::RenderFrame()
{
	rhi::IGraphicsCommand * GfxContext = dynamic_cast<rhi::IGraphicsCommand*>(m_TestCommandContext);
	GfxContext->SetViewport(m_Viewport);
	rhi::Rect rect = { 0, 0, 1920, 1080 };
	GfxContext->SetScissorRects(1, &rect);
	GfxContext->SetPipelineState(0, m_TestPipelineState);
	GfxContext->SetPrimitiveType(rhi::EPT_Triangles);
	GfxContext->SetVertexBuffer(0, m_TestVertexBufferView);
	GfxContext->SetPipelineLayout(m_TestPipelineLayout);
	//GfxContext->SetRenderTargets();
	GfxContext->DrawInstanced(rhi::DrawInstanceParam(3, 1));
	m_TestCommandContext->Execute(false);
	m_TestCommandContext->Reset();
}
