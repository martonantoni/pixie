#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/ui/EditField/EditField_Visualizer_Standard.h"

void cStandardEditFieldVisualizer::Init(const cEditField::cInitData &InitData)
{
	mTextStyle=InitData.mTextStyle;
	RedrawNeeded();

}

void cStandardEditFieldVisualizer::StateChanged()
{
	RedrawNeeded();
}

void cStandardEditFieldVisualizer::Redraw()
{
	cRect ValidRect=GetVisualizable().GetValidRect();
	bool UseClipping=GetVisualizable().GetUseClipping();
	cRect Rect=GetPlacement();
	cTextRenderer TextRenderer(mTextStyle);
	TextRenderer.setMaxWidth(Rect.width());
	TextRenderer.setDefaultColor("editfield");
	mTextSprite=std::make_unique<cSimpleMultiSprite>(TextRenderer.render(GetVisualizable().GetText()), cSimpleMultiSprite::eBasePosition::Zero);
	mTextSprite->SetWindow(GetWindow());
	mTextSprite->SetPosition(Rect.position());
	mTextSprite->SetZOrder(mBaseZ);
	mTextSprite->Show();

	if(!mCursorSprite)
	{
		auto Texture=theTextureManager.GetTexture("1pix");
		mCursorSprite=std::make_unique<cSprite>();
		mCursorSprite->SetTexture(Texture);
		mCursorSprite->SetSize({ 2, theFontManager.font(mTextStyle)->GetHeight() });
		mCursorSprite->SetRGBColor("editfield_cursor");
		auto TimedAnimators=make_intrusive_ptr<cTimedAnimators>();
		TimedAnimators->AddStandardBlender({ { 0, 400 }, cSprite::Property_Alpha, { 255 } });
		TimedAnimators->AddStandardBlender({ { 400, 800 }, cSprite::Property_Alpha, { 128 } });
		TimedAnimators->SetLooping(800, 0);
		mCursorSprite->AddAnimator(std::move(TimedAnimators));
		mCursorSprite->SetWindow(GetWindow());
	}
	int CursorPosition=GetVisualizable().GetCursorPosition();
	mCursorSprite->SetPosition({ CursorPosition==0?Rect.left():mTextSprite->GetSubSprite(CursorPosition-1)->GetRect().right(), Rect.top() });

	mCursorSprite->SetVisible(GetVisualizable().IsInFocus());
}

REGISTER_VISUALIZER(cEditField, "standard", cStandardEditFieldVisualizer);

