#ifndef MFONT_H
#define MFONT_H

#include <STB/stb_truetype.h>
#include <STB/stb_image_write.h>

#include <glm/glm.hpp>

#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <array>

namespace MScreen
{
	class MFont
	{
		struct glyph
		{
			int32_t codePoint;
			glm::vec2 uv0, uv1;

			float normAdvance;
			glm::vec2 normOffset;
			glm::vec2 normSize;
		};

	private:
		const float fontSize = 64.0f;

		std::string fileName = "data/MScreen/font.ttf";
		std::string sCharacters = "1234567890ABCDEFGHIJKLMNOPQRSUTVWXYZabcd";
		std::vector<glyph> indexGlyphs;

	public:
		const int32_t bitmapWidth = 1024;
		const int32_t bitmapHeight = 1024;
		std::vector<unsigned char> pixels;

		void loadFont();
	};
}
#endif