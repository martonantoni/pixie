#pragma once

class cSpriteBase : public c2DRenderable
{
protected:
	cRect mRect;
	cPoint mPositionOffset;
public:
	virtual std::unique_ptr<cSpriteBase> Clone() const = 0;

	virtual bool GetProperty(unsigned int PropertyFlags, OUT cPropertyValues& Value) const override;
	virtual bool SetProperty(unsigned int PropertyFlags, const cPropertyValues& Value) override;

	virtual cPoint GetPrefferedSize() const { return GetSize(); }
	int GetX() const { return mRect.left(); }
	int GetY() const { return mRect.top(); }
	cPoint GetPosition() const { return mRect.position(); }
	cPoint GetScreenPosition() const;
	cPoint center() const;
	int GetWidth() const { return mRect.width(); }
	int GetHeight() const { return mRect.height(); }
	cPoint GetSize() const { return mRect.size(); }
	const cRect& GetRect() const { return mRect; }
	cRect GetCenterAndHSize() const;
	void SetRect(const cRect& Rect);
	void SetPlacement(const cRect& Rect) { SetRect(Rect); } // just an alias
	void SetCenterAndHSize(const cRect& Rect);
	void SetCenter(cPoint Point);
	void SetPosition(int x, int y) { SetPosition(cPoint(x, y)); }
	void SetPosition(cPoint Position);
	void SetScreenPosition(cPoint Position);
	void SetPositionOffset(int XOffset, int YOffset) { SetPositionOffset(cPoint(XOffset, YOffset)); }
	void SetPositionOffset(const cPoint& PositionOffset);
	cPoint GetPositionOffset() const { return mPositionOffset; }
	void SetSize(int Width, int Height) { SetSize(cPoint(Width, Height)); }
	void SetSize(cPoint Size);
	cRect GetRectForRendering() const; // this one includes X and Y offset
};

USE_DROP_INSTEAD_DELETE_PARENT(cSpriteBase, c2DRenderable);


