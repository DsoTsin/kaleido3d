#include "Kaleido3D.h"
#include "FontRenderer.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace render
{
	FontManager::FontManager()
		: m_PixelSize(0)
		, m_Color(0xffffffff)
		, m_pFontLib(nullptr)
		, m_pFontFace(nullptr)
	{
		FT_Init_FreeType((FT_Library*)&m_pFontLib);
	}

	FontManager::~FontManager()
	{
		if (m_pFontFace)
		{
			FT_Done_Face((FT_Face)m_pFontFace);
			m_pFontFace = nullptr;
		}
		if (m_pFontLib)
		{
			FT_Done_FreeType((FT_Library)m_pFontLib);
			m_pFontLib = nullptr;
		}
	}

	void FontManager::LoadLib(const char * fontPath)
	{
		if (m_pFontFace)
		{
			FT_Done_Face((FT_Face)m_pFontFace);
			m_pFontFace = nullptr;
		}
		FT_New_Face((FT_Library)m_pFontLib, fontPath, 0, (FT_Face*)&m_pFontFace);
	}

	void FontManager::ChangeFontSize(int height)
	{
		FT_Set_Pixel_Sizes((FT_Face)m_pFontFace, 0, height);
		m_PixelSize = height;
	}

	void FontManager::SetPaintColor(int color)
	{
		m_Color = color;
	}

	unsigned int* GlyphTexture(const FT_Bitmap& bitmap, const unsigned int& color)
	{
		unsigned int* buffer = new unsigned int[bitmap.width * bitmap.rows * 4];
		for (int y = 0; y< bitmap.rows; y++)
		{
			for (int x = 0; x < bitmap.width; x++)
			{
				unsigned int pixel = (color & 0xffffff00);
				unsigned int alpha = bitmap.buffer[(y * bitmap.pitch) + x];
				pixel |= alpha;
				buffer[(y*bitmap.width) + x] = pixel;
			}
		}
		return buffer;
	}

	TextQuads FontManager::AcquireText(const::k3d::String & text)
	{
		if(text.Length()==0 || !m_pFontFace)
			return TextQuads();
		FT_Face face = (FT_Face)m_pFontFace;
		TextQuads quadlist;
		for (unsigned int i = 0; i < text.Length(); i++) 
		{
			FT_Load_Char(face, text[i], FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
			unsigned int *bytes = GlyphTexture(face->glyph->bitmap, m_Color);
			int width = face->glyph->bitmap.width;
			int height = face->glyph->bitmap.rows; 
			int x = face->glyph->metrics.horiBearingX / m_PixelSize;
			int y =	face->glyph->metrics.horiBearingY / m_PixelSize;
			int next = face->glyph->metrics.horiAdvance / m_PixelSize;
			quadlist.Append({ x,y, width, height, next, bytes});
		}
		return quadlist;
	}

	FontTexture::FontTexture(rhi::DeviceRef device, TextQuad const & quad)
	{
		rhi::ResourceDesc texDesc;
		texDesc.Type = rhi::EGT_Texture2D;
		texDesc.ViewType = rhi::EGpuMemViewType::EGVT_SRV;
		texDesc.Flag = rhi::EGpuResourceAccessFlag::EGRAF_HostVisible;
		texDesc.TextureDesc.Format = rhi::EPF_RGBA8Unorm; // TODO font color fmt inconsistent
		texDesc.TextureDesc.Width = quad.W;
		texDesc.TextureDesc.Height = quad.H;
		texDesc.TextureDesc.Layers = 1;
		texDesc.TextureDesc.MipLevels = 1;
		texDesc.TextureDesc.Depth = 1;
		m_Texture = ::k3d::DynamicPointerCast<rhi::ITexture>(device->NewGpuResource(texDesc));

		uint64 sz = m_Texture->GetResourceSize();
		void * pData = m_Texture->Map(0, sz);
		rhi::SubResourceLayout layout = {};
		rhi::TextureResourceSpec spec = { rhi::ETAF_COLOR,0,0 };
		device->QueryTextureSubResourceLayout(m_Texture, spec, &layout);
		if (quad.W * 4 == layout.RowPitch)
		{
			memcpy(pData, quad.Pixels, sz);
		}
		else
		{
			for (int y = 0; y < quad.H; y++)
			{
				uint32_t *row = (uint32_t *)((char *)pData + layout.RowPitch * y);
				for (int x = 0; x < quad.W; x++)
				{
					row[x] = quad.Pixels[x + y * quad.W];
				}
			}
		}
		m_Texture->UnMap();

		auto cmd = device->NewCommandContext(rhi::ECMD_Graphics);
		cmd->Begin();
		cmd->TransitionResourceBarrier(m_Texture, rhi::ERS_ShaderResource);
		cmd->End();
		cmd->Execute(false);
	}
	
	FontTexture::~FontTexture()
	{
	}

	FontRenderer::FontRenderer()
	{
	}
	
	FontRenderer::~FontRenderer()
	{
	}
	
	void FontRenderer::DrawText2D(rhi::CommandContextRef const & cmd, const::k3d::String & text, float x, float y)
	{
	}
}