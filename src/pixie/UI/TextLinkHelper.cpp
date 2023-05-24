#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cTextLinkHelper::~cTextLinkHelper()
{
}

cTextLinkHelper::cTextLinkHelper(const cInitData &InitData)
{
	auto &RenderInfo=InitData.mRenderInfo;
	auto &TextSprite=InitData.mTextSprite;
	auto &LinkHandlers=InitData.mLinkHandlers;
	mLinks.resize(RenderInfo.mGroups.size());
	ASSERT(LinkHandlers.size()==RenderInfo.mGroups.size()); // we survive if this is not true, but it is probably not what you intended.

	for(size_t i=0, iend=mLinks.size(); i!=iend;++i)
	{
		auto &LinkData=mLinks[i];

		if(LinkHandlers.size()>i)  // this is what the assert before the loop checked
			LinkData.mLinkHandler=LinkHandlers[i];

		LinkData.mSprites.reserve(RenderInfo.mGroups[i].mLastSpriteIndex-RenderInfo.mGroups[i].mFirstSpriteIndex+1);
		LinkData.mSprites.emplace_back(TextSprite.GetSubSprite(RenderInfo.mGroups[i].mFirstSpriteIndex));
		cRect BoundingRect=TextSprite.GetSubSprite(RenderInfo.mGroups[i].mFirstSpriteIndex)->GetRect();
		for(size_t sprite_index=RenderInfo.mGroups[i].mFirstSpriteIndex+1; sprite_index<=RenderInfo.mGroups[i].mLastSpriteIndex; ++sprite_index)
		{
			BoundingRect.GrowToBound(TextSprite.GetSubSprite(sprite_index)->GetRect());
			LinkData.mSprites.emplace_back(TextSprite.GetSubSprite(sprite_index));
		}
		LinkData.mBoundingRect=BoundingRect;
		LinkData.mOriginalSpriteColors.reserve(LinkData.mSprites.size());
		for(auto &Sprite: LinkData.mSprites)
		{
			LinkData.mOriginalSpriteColors.emplace_back(Sprite->GetColor());
		}

		LinkData.mTarget=std::make_unique<cMouseEventEmitterTarget>();
		cMouseEventEmitterTarget::cInitData TargetInitData;
		TargetInitData.mZOrder=200;
		TargetInitData.mParentWindow=InitData.mWindow;
		TargetInitData.mPlacement=BoundingRect;
		TargetInitData.mPlacement.Move(InitData.mTextPosition);
		LinkData.mTarget->Init(TargetInitData);

		LinkData.mListeners.emplace_back(LinkData.mTarget->GetDispatcher(cMouseEventEmitterTarget::Event_MoveInside)->RegisterListener([i, this](auto &)
			{ OnMoveInsideTarget(i); }));
		LinkData.mListeners.emplace_back(LinkData.mTarget->GetDispatcher(cMouseEventEmitterTarget::Event_LeftButtonDown)->RegisterListener([i, this](auto &)
			{ OnClickTarget(i); }));
		LinkData.mListeners.emplace_back(LinkData.mTarget->GetDispatcher(cMouseEventEmitterTarget::Event_MoveOutside)->RegisterListener([i, this](auto &)
			{ OnMoveOutsideTarget(i); }));
	}
}

void cTextLinkHelper::SetLinkColor(size_t LinkIndex, const std::string &ColorName)
{
	auto Color=theColorServer.GetColor(ColorName);
	for(auto &Sprite: mLinks[LinkIndex].mSprites)
	{
		::BlendObjectRGBColor_NoKeepAlive(*Sprite, Color, 300);
	}
}

void cTextLinkHelper::NotifyHandler(size_t LinkIndex, eAction Action)
{
	auto &Link=mLinks[LinkIndex];
	if(!Link.mLinkHandler)
		return;
	auto LinkRect=Link.mTarget->GetPlacement();
	auto LinkTargetWindow=Link.mTarget->GetWindow();
	if(ASSERTTRUE(LinkTargetWindow))
		LinkRect.Move(LinkTargetWindow->GetScreenRect().GetPosition());
	Link.mLinkHandler(Action, LinkRect);
}

void cTextLinkHelper::OnMoveInsideTarget(size_t LinkIndex)
{
	SetLinkColor(LinkIndex, "link_active"s);
	NotifyHandler(LinkIndex, eAction::Hover_Enter);
}

void cTextLinkHelper::OnMoveOutsideTarget(size_t LinkIndex)
{
	for(size_t i=0,iend=mLinks[LinkIndex].mSprites.size();i!=iend;++i)
		::BlendObjectRGBColor_NoKeepAlive(*mLinks[LinkIndex].mSprites[i], mLinks[LinkIndex].mOriginalSpriteColors[i], 300);
	NotifyHandler(LinkIndex, eAction::Hover_Leave);
}

void cTextLinkHelper::OnClickTarget(size_t LinkIndex)
{
	NotifyHandler(LinkIndex, eAction::Click);
}

void cTextLinkHelper::SetWindow(cPixieWindow *Window)
{
	for(auto &Link: mLinks)
		Link.mTarget->SetWindow(Window);
}

void cTextLinkHelper::SetTextPosition(cPoint Position)
{
	for(auto &Link: mLinks)
	{
		Link.mTarget->SetPosition(Position+Link.mBoundingRect.GetPosition());
	}
}
