#include "StdAfx.h"

cFont::cFont(const cConfig &Config)
{
	int Height=Config.GetInt("height", 0);
	int Width=Config.GetInt("width", 0);
	int Weight=Config.GetInt("weight", 0);
	int IsItalic=Config.GetBool("is_italic", false);
	std::string FaceName=Config.GetString("face_name", "Arial"s);
	mDefaultColor=Config.GetInt("default_color", D3DCOLOR_ARGB(0xff, 0, 0, 0));

	D3DXFONT_DESC FontDesc =
	{
		Height,
		(UINT)Width,
		(UINT)Weight,
		0,  // mip levels
		IsItalic,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, //OUT_TT_PRECIS,
		PROOF_QUALITY, //CLEARTYPE_QUALITY,
		DEFAULT_PITCH,
		""
	};
	strncpy(FontDesc.FaceName, FaceName.c_str(), LF_FACESIZE);

	D3DXCreateFontIndirect(theDevice->GetD3DObject(), &FontDesc, &mFont);

	mFont->GetTextMetrics(&mMetrics);
}

void cFont::Render(const std::string &Text, const RECT &Rect, DWORD Format, D3DCOLOR Color) const
{
	mFont->DrawTextA(NULL,Text.c_str(),(int)Text.length(),&const_cast<RECT &>(Rect),Format,Color);
}

void cFont::RenderUTF8(const std::string &Text, const RECT &Rect, DWORD Format, D3DCOLOR Color) const
{
	auto Decoded=UTF8::Decode(Text);
	mFont->DrawTextW(NULL,Decoded.data(),INT(Decoded.size()-1),&const_cast<RECT &>(Rect),Format,Color);
}
