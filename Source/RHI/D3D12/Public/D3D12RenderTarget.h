#pragma once
#ifndef __D3D12RenderTarget_h__
#define __D3D12RenderTarget_h__

NS_K3D_D3D12_BEGIN

class D3D12RenderTarget
{
public:
	D3D12RenderTarget();
	~D3D12RenderTarget();

	void SetClearColor(Vector4F & ClearColor)
	{
		m_ClearColor = ClearColor;
	}

private:
	typedef std::pair<PtrResource, D3D12_CPU_DESCRIPTOR_HANDLE> ColorHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE m_DSV;
	
	Vector4F m_ClearColor;
};

NS_K3D_D3D12_END

#endif