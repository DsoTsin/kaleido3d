#include "Kaleido3D.h"

#include <Core>

#include <Renderer/DirectX/RenderMesh.h>
#include <Renderer/DirectX/Device.h>
#include <Renderer/DirectX/Renderer.h>

#include <Renderer/DirectX/Wrapper.h>
#include <Renderer/DirectX/Infrastructure/Shader.h>
#include <Renderer/DirectX/Infrastructure/VertexBuffer.h>
#include <Renderer/DirectX/RootSignature.h>
#include <Renderer/DirectX/PipeLineState.h>

using namespace k3d;
using namespace k3d::d3d12;

UploadBufferWrapper mWorldMatrix;
UploadBufferWrapper mViewMatrix;
UploadBufferWrapper mProjMatrix;
DescriptorHeapWrapper mCBDescriptorHeap; 

d3d12::Shader g_VS;
d3d12::Shader g_PS;
RootSignature g_RootSig;
PipelineStateObject g_PSO;
VertexBuffer g_VB; 
DescriptorHeapWrapper mSamplerHeap;
PtrResource mTexture2D;

void Init();
void Render();

int main(int argc, const char ** argv)
{
	Log::InitLogFile("UTDXRenderer.html");
	AssetManager::Get().Init();
	Window window("UTDXRenderer", 1700, 700);
	window.Show();

	d3d12::Device::Get().Init(&window);
	
	Concurrency::Thread thread;
	thread.Bind([]() {});

	DirectXRenderer & renderer = DirectXRenderer::Get();

	Init();

	Message msg;
	while (window.IsOpen())
	{
		bool isQuit = false;
		while (window.PollMessage(msg))
		{
			if (msg.type == Message::MouseButtonPressed)
			{
				Debug::Out("Main", "leftbutton");
			}
			else if (msg.type == Message::Closed)
			{
				isQuit = true;
				break;
			}
		}

		Render();
		Device::Get().Present();

		if (isQuit)
			break;
	}

	Log::CloseLog();
	return 0;
}


void Init() {
	UINT cbSize = sizeof(DirectX::XMMATRIX);
	HRESULT hr = mWorldMatrix.Create(Device::Get().GetD3DDevice(), cbSize, D3D12_HEAP_TYPE_UPLOAD);
	hr = mViewMatrix.Create(Device::Get().GetD3DDevice(), cbSize, D3D12_HEAP_TYPE_UPLOAD);
	hr = mProjMatrix.Create(Device::Get().GetD3DDevice(), cbSize, D3D12_HEAP_TYPE_UPLOAD);
	mCBDescriptorHeap.Create(Device::Get().GetD3DDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3, true);
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc;
	cbDesc.BufferLocation = mViewMatrix.pBuf->GetGPUVirtualAddress();
	cbDesc.SizeInBytes = 256;
	Device::Get().GetD3DDevice()->CreateConstantBufferView(&cbDesc, mCBDescriptorHeap.hCPU(0));
	cbDesc.BufferLocation = mProjMatrix.pBuf->GetGPUVirtualAddress();
	cbDesc.SizeInBytes = 256;
	Device::Get().GetD3DDevice()->CreateConstantBufferView(&cbDesc, mCBDescriptorHeap.hCPU(1));
}


void Render() {
	using namespace DirectX;
	static float angle = 0.0f;
	angle += XM_PI / 180.0f;
	if ((angle > XM_PI * 0.5f) && (angle < XM_PI * 1.5f)) angle = XM_PI * 1.5f;
	if (angle > XM_2PI) angle = 0.0f;

}