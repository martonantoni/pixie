#include "StdAfx.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_MODULE_H

#ifdef _DEBUG
#pragma comment(lib,"../pixie/pixie/freetype-2.6/obj/x64/debug/freetype26d.lib")
#else
#pragma comment(lib,"../pixie/pixie/freetype-2.6/obj/x64/release/freetype26.lib")
#endif

#pragma warning(disable:4018) // '<': signed/unsigned mismatch

#include "BinPacker.h"

void InitFreeType()
{
	theFontManager.Init();
}

#define FT_CHECKED_CALL(x) \
	if(ASSERTFALSE(x)) \
		return false;

bool cFontManager2::InitFont(cFont2 &Font, tIntrusivePtr<cConfig> Config)
{
	if(ASSERTFALSE(!Config))
		return false;
	FT_Library  library;
	FT_CHECKED_CALL(FT_Init_FreeType(&library));
	FT_Face     face;      /* handle to face object */
	FT_CHECKED_CALL(FT_New_Face(library, Config->GetString("file").c_str(), 0, &face));
//	Font.mFontHeight=Config->GetInt("height");
	FT_CHECKED_CALL(FT_Set_Pixel_Sizes(face, 0, Config->GetInt("height")));
	Font.mFontHeight=face->size->metrics.height>>6;
	Font.mAscender=face->size->metrics.ascender>>6;
	Font.mDescender=face->size->metrics.descender>>6;

	static std::string Letters="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'()*+-'.,/:; <=>?@[\\]^_`{|}~";
	auto ExtraLettersUTF8=Config->GetString("extra_letters", std::string());
	auto ExtraLetters=UTF8::Decode(ExtraLettersUTF8);
	ExtraLetters.pop_back(); // the terminating zero
	std::vector<FT_Glyph> Glyphs(Letters.size()+ExtraLetters.size()+1);
	struct cLetterData
	{
		cPoint mPos;
	};
	std::vector<cLetterData> LetterData;
	std::vector<int> Sizes;
	LetterData.resize(Letters.size()+ExtraLetters.size());
	int Index=-1;
	int HeightMax=0;
	int YMaxMax=0;

	FT_Select_Charmap(face, ft_encoding_unicode);

	auto ForEachLetter=[&Font, &ExtraLetters](const auto &Function)
	{
		for(auto Letter: Letters)
		{
			Function(Letter, Font.mNormalLetters[(unsigned char)Letter]);
		}
		for(auto Letter: ExtraLetters)
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
		Sizes.push_back(width+1);//+2);
		Sizes.push_back(height+1);//+2);
								  //		LetterData[Index].mSize= { width,height };
		if(height>HeightMax)
			HeightMax=height;
		if(glyph_bbox.yMax>YMaxMax)
			YMaxMax=glyph_bbox.yMax;
	});


//	Font.mFontHeight=HeightMax;  //<<----             should not be needed
	Font.mAscender=YMaxMax;      // well, same deal
	for(int s=256;; s*=2)
	{
		BinPacker Packer;
		std::vector<std::vector<int>> Packed;
		Packer.Pack(Sizes, Packed, s, false);
		if(Packed.size()>=2)
			continue;
		Font.mAtlasTexture=cTexture::CreateWriteable({ s,s });
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
	cTexture::cLockInfo LockInfo=Font.mAtlasTexture->LockSurface();
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
				*(unsigned int *)dest=(((unsigned int)*source) << 24) | 0xffffff;
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
	return true;
}

#undef FT_CHECKED_CALL
#define FT_CHECKED_CALL(x) \
	if(ASSERTFALSE(x)) \
		return;

cFont2 *cFontManager2::GetFont(const std::string &Name)
{
	auto i=std::find_if(mFonts, [&Name](auto &Font) { return Font.mName==Name; });
	if(ASSERTFALSE(i==mFonts.end()))
		return nullptr;
	auto &FontData=*i;
	if(!FontData.mAliasOf.empty())
		return GetFont(FontData.mAliasOf);
	return FontData.mFont.get();
}

void cFontManager2::Init()
{
	auto FontsConfig=theMainConfig->GetSubConfig("fonts");
	auto FontNames=FontsConfig->GetSubConfigNames();
	mFonts.reserve(FontNames.size());
	for(size_t i=0; i<FontNames.size(); ++i)
	{
		cFontData FontData(FontNames[i]);
		auto FontConfig=FontsConfig->GetSubConfig(FontNames[i]);
		if(ASSERTFALSE(!FontConfig))
			continue;
		std::string AliasOf=FontConfig->GetString("alias_of",std::string());
		if(!AliasOf.empty())
		{
			FontData.mAliasOf=std::move(AliasOf);
			mFonts.emplace_back(std::move(FontData));
		}
		else
		{
			FontData.mFont=std::make_unique<cFont2>(FontNames[i]);
			if(InitFont(*FontData.mFont, FontConfig))
			{
				mFonts.emplace_back(std::move(FontData));
			}
			else
			{
				MainLog->Log("Unable to initialize font: \"%s\"", FontNames[i].c_str());
			}
		}
	}
}
