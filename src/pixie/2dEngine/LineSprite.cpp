#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

void SpriteBetweenPoints(cSprite &Sprite, cPoint a, cPoint b, int width)
{
	auto dx=a.x-b.x;
	auto dy=a.y-b.y;
	auto Length=sqrt(dx*dx+dy*dy);
	auto Angle=dx?90.0+atan((double)dy/(double)dx)*180.0/3.141592653589793238462643383279:0.0;
	cPoint Center=cPoint(a.x+b.x, a.y+b.y)/2;
	cPoint StartPoint=Center-cPoint(0, Length/2);
	Sprite.SetPlacement(cRect(StartPoint, { width, (int)Length }));
	Sprite.SetRotation(Angle);
}

std::unique_ptr<cSprite> CreateLineSprite(cPoint a, cPoint b, int width)
{
	auto LineSprite=std::make_unique<cSprite>();
	LineSprite->SetTexture(theTextureManager.GetTexture("1pix"));
	SpriteBetweenPoints(*LineSprite, a, b, width);
	return LineSprite;
}
