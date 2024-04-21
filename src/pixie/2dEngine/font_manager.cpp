#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_MODULE_H

#pragma warning(disable:4018) // '<': signed/unsigned mismatch

#include "3rd_party/BinPacker.h"

void InitFreeType()
{
	theFontManager.Init();
}

#define FT_CHECKED_CALL(x) \
	if(ASSERTFALSE(x)) \
		return nullptr;

std::shared_ptr<const cFont> cFontManager::makeFont(const std::string& fileName, int height)
{
	FT_Library  library;
	FT_CHECKED_CALL(FT_Init_FreeType(&library));
	FT_Face     face;      /* handle to face object */
	if(FT_New_Face(library, fileName.c_str(), 0, &face))
    {
        printf("FontManager: unable to load font: \"%s\"\n", fileName.c_str());
        return nullptr;
    }
	FT_CHECKED_CALL(FT_Set_Pixel_Sizes(face, 0, height));
	auto font = std::make_shared<cFont>(fileName);
	auto& Font = *font;
	Font.mFontHeight=face->size->metrics.height>>6;
	Font.mAscender=face->size->metrics.ascender>>6;
	Font.mDescender=face->size->metrics.descender>>6;

	static std::string Letters="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'()*+-'.,/:; <=>?@[\\]^_`{|}~";
	std::vector<FT_Glyph> Glyphs(Letters.size() + mExtraLetters.size() + 1);
	struct cLetterData
	{
		cPoint mPos;
	};
	std::vector<cLetterData> LetterData;
	std::vector<int> Sizes;
	LetterData.resize(Letters.size() + mExtraLetters.size());
	int Index=-1;
	int HeightMax=0;
	int YMaxMax=0;

	FT_Select_Charmap(face, ft_encoding_unicode);

	auto ForEachLetter=[&Font, this](const auto &Function)
	{
		for(auto Letter: Letters)
		{
			Function(Letter, Font.mNormalLetters[(unsigned char)Letter]);
		}
		for(auto Letter: mExtraLetters)
		{
			Function(Letter, Font.mExtraLetters[Letter]);
		}
	};

	ForEachLetter([&](auto Letter, auto &FontLetterData)
	{
		++Index;
		auto glyph_index = FT_Get_Char_Index(face, Letter);
		/* load glyph image into the slot without rendering */
		auto Error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		if(ASSERTFALSE(Error))
			return;  /* ignore errors, jump to next glyph */
					 /* extract glyph image and store it in our table */
		FT_GlyphSlot  slot = face->glyph;
		FontLetterData.mAdvance=slot->advance.x;
		Error = FT_Get_Glyph(face->glyph, &Glyphs[Index]);
		if(ASSERTFALSE(Error))
			return;  /* ignore errors, jump to next glyph */
		FT_BBox  glyph_bbox;
		FT_Glyph_Get_CBox(Glyphs[Index], ft_glyph_bbox_pixels, &glyph_bbox);

		int width  = glyph_bbox.xMax - glyph_bbox.xMin;
		int height = glyph_bbox.yMax - glyph_bbox.yMin;
		Sizes.push_back(width+1);
		Sizes.push_back(height+1);
								
		if(height>HeightMax)
			HeightMax=height;
		if(glyph_bbox.yMax>YMaxMax)
			YMaxMax=glyph_bbox.yMax;
	});

	Font.mAscender=YMaxMax;      // well, same deal
	int textureSize = 256;
	for(;; textureSize *= 2)
	{
		BinPacker Packer;
		std::vector<std::vector<int>> Packed;
		Packer.Pack(Sizes, Packed, textureSize, false);
		if(Packed.size()>=2)
			continue;
		Font.mAtlasTexture=cTexture::CreateWriteable({ textureSize, textureSize });
		std::vector<int> &FirstPacked=Packed.front();
		int Index=-1;
		ForEachLetter([&](auto Letter, auto &FontLetterData)
		{
			++Index;
			for(int i=0; i<FirstPacked.size(); i+=4)
			{
				if(FirstPacked[i]==Index)
				{
					LetterData[Index].mPos= { FirstPacked[i+1],FirstPacked[i+2] };
					break;
				}
			}
		});
		break;
	}
	cTexture::cLockInfo LockInfo=Font.mAtlasTexture->LockSurface(cTexture::IsReadOnly::no);
	for (int y = 0; y < Font.mAtlasTexture->GetSurfaceHeight(); ++y)
	{
		memset(LockInfo.mBytes + y * LockInfo.mPitch, 0xff, Font.mAtlasTexture->GetSurfaceWidth() * 4);
		for (int x = 0; x < Font.mAtlasTexture->GetSurfaceWidth(); ++x)
		{
			*(LockInfo.mBytes + y * LockInfo.mPitch + x*4 + 3) = 0;
		}
	}

	Index=-1;

	ForEachLetter([&](auto Letter, auto &FontLetterData)
	{
		++Index;

		auto Error = FT_Glyph_To_Bitmap(&Glyphs[Index], FT_RENDER_MODE_NORMAL, NULL, 1);
		if(ASSERTFALSE(Error))
			return;
		FT_BitmapGlyph  bitmap = (FT_BitmapGlyph)Glyphs[Index];

		cLetterData &ThisLetterData=LetterData[Index];

		for(int row=0; row<bitmap->bitmap.rows; ++row)
		{
			char *dest=LockInfo.mBytes+ThisLetterData.mPos.x*4+(ThisLetterData.mPos.y+row)*LockInfo.mPitch;
			unsigned char *source=bitmap->bitmap.buffer+row*bitmap->bitmap.pitch;
			for(int x=0; x<bitmap->bitmap.width; ++x)
			{
				*(unsigned int*)dest = (((unsigned int)*source) << 24) | 0xff'ff'ff;
				dest+=4;
				++source;
			}
		}
		FontLetterData.mXOffset=bitmap->left;
		FontLetterData.mYOffset=YMaxMax-bitmap->top;
		ASSERT(FontLetterData.mYOffset>=0);
		FontLetterData.mTexture=Font.mAtlasTexture->CreateSubTexture(
			cRect(ThisLetterData.mPos, { (int)bitmap->bitmap.width, (int)bitmap->bitmap.rows }));

		FT_Done_Glyph(Glyphs[Index]);
	});
	Font.mAtlasTexture->UnlockSurface();
	FT_Done_Face(face);
	FT_Done_Library(library);

	return font;
}

#undef FT_CHECKED_CALL
#define FT_CHECKED_CALL(x) \
	if(ASSERTFALSE(x)) \
		return;

std::shared_ptr<const cFont> cFontManager::GetFont(const std::string &Name)
{
	auto i=std::ranges::find_if(mFonts, [&Name](auto &Font) { return Font.mName==Name; });
	if(ASSERTFALSE(i==mFonts.end()))
		return nullptr;
	auto &FontData=*i;
	if(!FontData.mAliasOf.empty())
		return GetFont(FontData.mAliasOf);
	return FontData.mFont;
}

std::shared_ptr<const cFont> createFont(const std::string& name, int size)
{
	return {};
}

void cFontManager::Init()
{
	auto fontsConfig = theGlobalConfig->createSubConfig("fonts");
	auto ExtraLettersUTF8 = fontsConfig->get<std::string>("extra_letters", std::string());
	mExtraLetters = UTF8::Decode(ExtraLettersUTF8);
	mExtraLetters.pop_back(); // the terminating zero

	fontsConfig->forEachSubConfig(
		[this](const std::string& name, const cConfig& config)
		{
			cFontData FontData(name);
            auto AliasOf=config.get<std::string>("alias_of", std::string());
            if(!AliasOf.empty())
            {
                FontData.mAliasOf=std::move(AliasOf);
                mFonts.emplace_back(std::move(FontData));
            }
            else
            {
				FontData.mFont = std::make_shared<cFont>(name);
				auto fileName = config.get<std::string>("file");
				auto height = config.get<int>("height");
				if(auto font = makeFont(fileName, height))
                {
					FontData.mFont = std::move(font);
                    mFonts.emplace_back(std::move(FontData));
                }
                else
                {
                    MainLog->Log("FontManager: Unable to initialize font: \"%s\"", name.c_str());
                }
            }
		});

	int totalTextureSize = 0;
	for(auto &Font: mFonts)
    {
        if(Font.mAliasOf.empty())
            totalTextureSize += Font.mFont->atlasTexture()->GetSurfaceWidth() * Font.mFont->atlasTexture()->GetSurfaceHeight();
    }
	MainLog->Log("FontManager: %d fonts initialized. Total texture size: %d kb", mFonts.size(), totalTextureSize / 1024);
}

cFontManager theFontManager;