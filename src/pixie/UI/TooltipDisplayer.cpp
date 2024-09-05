#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/ui/TooltipDisplayer.h"

cRegisteredID cTooltipDisplayer::DisplayTooltip(const cTooltipRequest &Request)
{
	auto Sprites=cTooltipDisplayer::CreateTooltipSprites(Request);
	Sprites->SetAlpha(255);
	Sprites->Show();
	BlendObjectAlpha(*Sprites, { 0 }, 250);

	auto SpritesPtr=Sprites.get();
	return CreateWrappedRegisteredID(mTooltips.Register(std::move(Sprites)), 
		[SpritesPtr]()
		{
			BlendObjectAlpha(*SpritesPtr, { 255 }, 200);
		});
}

std::unique_ptr<cSimpleMultiSprite> cTooltipDisplayer::CreateTooltipSprites(const cTooltipRequest &Request)
{
    return CreateTooltipSprites(Request, std::make_unique<cSimpleMultiSprite>(Request.mTextRenderer.render(Request.mText), cSimpleMultiSprite::eBasePosition::Zero));
}

std::unique_ptr<cSimpleMultiSprite> cTooltipDisplayer::CreateTooltipSprites(const cBaseRequest &Request, std::unique_ptr<cSimpleMultiSprite> TextSprites)
{
	TextSprites->SetWindow(&thePixieDesktop.GetTopLayer());
	TextSprites->SetZOrder(Request.mZOrder);
	TextSprites->Show();
	cPoint Size=TextSprites->GetSize();
	if(Request.mWidth!=-1)
		Size.x=Request.mWidth;

	static const int ToolTipBorder=15;
	Size+=cPoint(ToolTipBorder*2, ToolTipBorder*2);
	cPoint Position(0, 0);

	cRect OuterRestriction=thePixieDesktop.GetClientRect();
	auto CalcRight=[&OuterRestriction, BaseRect=Request.mBaseRegion, Size]()
	{
		if(BaseRect.right()+Size.x<OuterRestriction.right())
			return BaseRect.right();
		else
			return BaseRect.left()-Size.x;
	};
	auto CalcVertCenter=[&OuterRestriction, BaseRect=Request.mBaseRegion, Size]()
	{
		return std::max(BaseRect.center().y-Size.y/2, 0);
	};
	switch(Request.mPrefferedDirection)
	{
	case cTooltipRequest::eDirection::UpAndRight:
		{
			Position.x=CalcRight();
			Position.y=std::max(Request.mBaseRegion.top()-Size.y, 0);
			break;
		}
	case cTooltipRequest::eDirection::Right:
		{
			Position.x=CalcRight();
			Position.y=CalcVertCenter();
			break;
		}
	case cTooltipRequest::eDirection::Left:
		{
			if(Request.mBaseRegion.left()-Size.x>=0)
				Position.x=Request.mBaseRegion.left()-Size.x;
			else
				Position.x=Request.mBaseRegion.right();
			Position.y=CalcVertCenter();
			break;
		}
	case cTooltipRequest::eDirection::Above:
	case cTooltipRequest::eDirection::Bellow:
	default:
		ASSERT(false);
	}

	TextSprites->SetPositionOffset(Position+cPoint { ToolTipBorder, ToolTipBorder });
	TextSprites->SetAlpha(255);

	auto BaseSprite=std::make_unique<cMultiSprite_Simple9Patch>();
	BaseSprite->SetCenterSpriteMode(cMultiSprite_Simple9Patch::eCenterSpriteMode::BetweenSides);
	BaseSprite->Init(&thePixieDesktop.GetTopLayer(), "thin_border");//  "simple_window");
	BaseSprite->SetRect({ Position, Size });
	BaseSprite->SetZOrder(Request.mZOrder-1);
	BaseSprite->SetAlpha(255);
	BaseSprite->Show();

	std::vector<std::unique_ptr<cSpriteBase>> Sprites;
	Sprites.emplace_back(std::move(TextSprites));
	Sprites.emplace_back(std::move(BaseSprite));

	return std::make_unique<cSimpleMultiSprite>(std::move(Sprites));
}
