#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

void SpriteBetweenPoints(cSprite &Sprite, cPoint a, cPoint b)
{
	auto dx=a.x-b.x;
	auto dy=a.y-b.y;
	auto Length=sqrt(dx*dx+dy*dy);
	auto Angle=dx?90.0+atan((double)dy/(double)dx)*180.0/3.141592653589793238462643383279:0.0;
	cPoint Center=cPoint(a.x+b.x, a.y+b.y)/2;
	cPoint StartPoint=Center-cPoint(0, Length/2);
	Sprite.SetPlacement(cRect(StartPoint, { 1, (int)Length }));
	Sprite.SetRotation(Angle);
}

std::unique_ptr<cSprite> Create1PixLineSprite(cPoint a, cPoint b)
{
	{
		const std::unique_ptr<cPixieWindow> Window=std::make_unique<cPixieWindow>();
		Window->SetPlacement({ 20,20,30,30 });
	}

	auto LineSprite=std::make_unique<cSprite>();
	LineSprite->SetTexture(theTextureManager.GetTexture("1pix"));
	SpriteBetweenPoints(*LineSprite, a, b);
	return LineSprite;
}
