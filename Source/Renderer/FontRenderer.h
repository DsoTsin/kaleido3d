#pragma once
#include <KTL/DynArray.hpp>
#include <KTL/String.hpp>
#include <Interface/IRHI.h>

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

	class FontTexture
	{
	public:
		FontTexture(rhi::DeviceRef device, TextQuad const& quad);
		~FontTexture();
		rhi::TextureRef GetTexture() const { return m_Texture; }
	private:
		rhi::TextureRef m_Texture;
	};

	class FontRenderer
	{
	public:
		FontRenderer();
		~FontRenderer();

		void DrawText2D(rhi::CommandContextRef const& cmd, const ::k3d::String & text, float x, float y);
	
	private:
		


	};
}