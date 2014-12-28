#pragma once
#include <Core/Image.h>

namespace k3d {

	class DDSHelper : public Image {
	public:
		DDSHelper();
		~DDSHelper();

		bool Load(uint8 *dataPtr, uint32 length) override;
		bool IsCompressed() const override;

		void Clear();
		static bool upperLeftOrigin;


	private:

		void flipSurface(uint8 *surf, int32 width, int32 height, int32 depth);
		static void flipBlocksDxtc1(uint8 *ptr, uint32 numBlocks);
		static void flipBlocksDxtc3(uint8 *ptr, uint32 numBlocks);
		static void flipBlocksDxtc5(uint8 *ptr, uint32 numBlocks);
		static void flipBlocksBc4(uint8 *ptr, uint32 numBlocks);
		static void flipBlocksBc5(uint8 *ptr, uint32 numBlocks);

	};
}