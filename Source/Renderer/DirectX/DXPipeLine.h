#pragma once
#include <KTL/NonCopyable.hpp>
#include "DXCommon.h"
#include "DirectXRenderer.h"

namespace k3d
{
	class DXPipeLine : public NonCopyable
	{
	public:
		explicit DXPipeLine(DXDevice & device);
		~DXPipeLine();

		//void						DrawIndexPrimitive(deferredcontext=nullptr);
		//void						DrawPrimitive(deferredcontext = nullptr);
		


//-------------------------------------------------------------------
		/**
		 * CommandList Support For MultiThread Rendering
		 */
		DxRef::DevContextPtr	ObtainDeferredContext();
		DxRef::CmdListPtr		ObtainCommandList(DxRef::DevContextPtr const & deferredContext);
		
		void				    ExecuteCommandList(DxRef::CmdListPtr const & commandList);

	private:

		Ref<ID3D11Device>			m_pDevice;
		Ref<ID3D11DeviceContext>	m_pImmediateContext;
	};
}