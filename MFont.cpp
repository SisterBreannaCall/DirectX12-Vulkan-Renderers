#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBTT_STATIC
#define _CRT_SECURE_NO_WARNINGS

#include "MFont.h"

namespace MScreen
{
	void MFont::loadFont()
	{
		std::ifstream file(fileName.c_str(), std::ios_base::binary);

		if (!file.is_open())
		{
			std::cout << "SkyNet :: could not open font file" << std::endl;
		}

		std::vector<unsigned char> fontData;
		
		std::copy(std::istreambuf_iterator<char>(file),
			std::istreambuf_iterator<char>(),
			std::back_inserter(fontData));

		file.close();

		pixels.resize(bitmapWidth * bitmapHeight);

		stbtt_pack_context packContext;

		stbtt_PackBegin(
			&packContext,
			pixels.data(),
			bitmapWidth,
			bitmapHeight,
			0,
			2,
			nullptr);

		std::vector<int32_t> codePoints;
		codePoints.resize(sCharacters.size());

		for (int i = 0; i < codePoints.size(); i++)
		{
			codePoints[i] = (int32_t)sCharacters[i];
		}

		std::vector<stbtt_pack_range> packRanges;
		packRanges.resize(1);
		packRanges[0].font_size = fontSize;
		packRanges[0].first_unicode_codepoint_in_range = 0;
		packRanges[0].array_of_unicode_codepoints = codePoints.data();
		packRanges[0].num_chars = (int)codePoints.size();
		packRanges[0].chardata_for_range = new stbtt_packedchar[codePoints.size()];

		stbtt_PackFontRanges(
			&packContext,
			fontData.data(),
			0,
			packRanges.data(),
			(int)packRanges.size());

		stbtt_PackEnd(&packContext);

		stbi_write_bmp("fonts.bmp", bitmapWidth, bitmapHeight, 1, pixels.data());

		for (const auto& range : packRanges)
		{
			for (int32_t i = 0; i < range.num_chars; i++)
			{
				const auto& ginfo = range.chardata_for_range[i];

				glyph indexGlyph;

				indexGlyph.codePoint = range.array_of_unicode_codepoints[i];
				indexGlyph.uv0 = {
					float(ginfo.x0) / bitmapWidth,
					float(ginfo.y1) / bitmapHeight
				};

				indexGlyph.uv1 = {
					float(ginfo.x1) / bitmapWidth,
					float(ginfo.y0) / bitmapHeight
				};

				indexGlyph.normAdvance = ginfo.xadvance / fontSize;

				indexGlyph.normOffset = {
					float(ginfo.xoff) / fontSize,
					float(ginfo.yoff) / fontSize
				};

				indexGlyph.normSize = {
					float(ginfo.x1 - ginfo.x0) / fontSize,
					float(ginfo.y1 - ginfo.y0) / fontSize
				};

				indexGlyphs.push_back(indexGlyph);
			}

			delete[] range.chardata_for_range;
		}
	}
}