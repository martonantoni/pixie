#pragma once

class cFont
{
	LPD3DXFONT mFont;
	D3DCOLOR mDefaultColor;
	TEXTMETRIC mMetrics;
public:
	cFont(const cConfig &Config);
	void Render(const std::string &Text, const RECT &Rect, DWORD Format, D3DCOLOR Color) const;
	void RenderUTF8(const std::string &Text, const RECT &Rect, DWORD Format, D3DCOLOR Color) const;
	D3DCOLOR GetDefaultColor() const { return mDefaultColor; }

	int GetHeight() const { return mMetrics.tmHeight; }
	int GetDisctanceBetweenRows() const { return mMetrics.tmExternalLeading; }
};