#pragma once 
#include "RHI/IRHI.h"

NS_K3D_D3D12_BEGIN

class GfxRootParameter
{
public:
	GfxRootParameter();

	void Load(const char* rpFile);

private:
	CD3DX12_ROOT_PARAMETER *m_TableSlots;
	CD3DX12_DESCRIPTOR_RANGE *m_DescriptorRanges;
	SIZE_T	m_Count;
};

class TextContext {
public:
	TextContext();

	struct Batch {
		D2D1_RECT_F Rect;
		PtrTextFormat Format;
		PtrSolidColorBrush Brush;
		virtual void Apply(PtrDevice2DContext context) = 0;
	};

	struct BatchW : Batch {
		std::wstring Text;
		void Apply(PtrDevice2DContext context) override
		{
			context->DrawText(Text.c_str(), (UINT32)Text.length(), Format.Get(), &Rect, Brush.Get());
		}
	};


	void Draw(const kchar * text, int x, int y);

	void FlushText() {
		m_DeviceContext->BeginDraw();
		m_DeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
		while (!m_Batches.empty()) {
			Batch * b = m_Batches.front();
			b->Apply(m_DeviceContext);
			m_Batches.pop();
		}
		m_DeviceContext->EndDraw();
	}

protected:
	void DrawIntermediate(Batch* batch);

private:
	std::queue<Batch*> m_Batches;

	PtrDevice11 m_Device11;
	PtrDevice11On12 m_Device11On12;
	PtrDevice2DContext m_DeviceContext;

};


NS_K3D_D3D12_END