#pragma once
#include <KTL/DynArray.hpp>
#include <KTL/String.hpp>
#include <Interface/IRHI.h>

#include <unordered_map>

namespace render
{
	class TextQuad
	{
	public:
		int X;
		int Y;
		int W;
		int H;
		int HSpace;
		unsigned int* Pixels;
	};

	typedef ::k3d::DynArray<TextQuad> TextQuads;

	class K3D_API FontManager
	{
	public:
		FontManager();
		~FontManager();

		void LoadLib(const char * fontPath);
		void ChangeFontSize(int height);
		void SetPaintColor(int color);

		TextQuads AcquireText(const ::k3d::String & text);

	private:
		int		m_PixelSize;
		int		m_Color;
		void*	m_pFontLib;
		void*	m_pFontFace;
		// Cache
	};

	class CharTexture
	{
	public:
		CharTexture(rhi::DeviceRef device, TextQuad const& quad);
		~CharTexture();
		rhi::TextureRef GetTexture() const { return m_Texture; }
	private:
		rhi::TextureRef m_Texture;
	};

	class CharRenderer
	{
	public:
		CharRenderer();
		~CharRenderer();

		void InitVertexBuffers(rhi::DeviceRef const& device);

	private:
		static short s_Indices[];
		static float s_Vertices[];
		static float s_CharTexCoords[];

	private:
		rhi::GpuResourceRef m_VertexBuffer;
		rhi::GpuResourceRef m_IndexBuffer;
	};

	class FontRenderer
	{
	public:
		explicit FontRenderer(rhi::DeviceRef const & device);
		~FontRenderer();
		void InitPSO();
		void DrawText2D(rhi::CommandContextRef const& cmd, const ::k3d::String & text, float x, float y);
	
	private:
		rhi::DeviceRef							m_Device;
		rhi::PipelineStateObjectRef				m_TextRenderPSO;
		FontManager								m_FontManager;
		std::unordered_map<char, CharTexture>	m_TexCache;
	};
}