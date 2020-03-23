#pragma once

class cMultiSpriteBase: public cSpriteBase
{
protected:
	typedef std::vector<std::unique_ptr<cSpriteBase>> cSprites;
	cSprites mSprites;
	virtual void ArrangeSprites() {}
	virtual void PropertiesChanged(unsigned int Properties) override;
	virtual ~cMultiSpriteBase()=default;
public:
	cMultiSpriteBase()=default;

	size_t GetNumberOfSubSprites() const { return mSprites.size(); }
	cSpriteBase *GetSubSprite(size_t Index) const { return mSprites[Index].get(); }

	enum class ePositioning { TopLeft, TopRight, BottomLeft, BottomRight }; // which point of the sprite is specified
	static void PositionSprite(cSpriteBase &Sprite, cPoint Position, ePositioning Positioning);
	enum class eStretchInclusion { IncludeRects, ExcludeRects };
	static void StretchSpriteBetween(cSpriteBase &Sprite, const cRect &RectTopLeft, const cRect &RectBottomRight, eStretchInclusion Inclusion);
	   // StretchSpriteBetween works only between equal size rects & Sprite's texture size
	static void StretchSpriteBetween(cSpriteBase &Sprite, const cSpriteBase &SpriteTopLeft, const cSpriteBase &SpriteBottomRight, eStretchInclusion Inclusion)
	{
		StretchSpriteBetween(Sprite, SpriteTopLeft.GetRect(), SpriteBottomRight.GetRect(), Inclusion);
	}
};

USE_DROP_INSTEAD_DELETE_PARENT(cMultiSpriteBase, cSpriteBase);

class cSimpleMultiSprite: public cMultiSpriteBase
{
	cPoint mBasePos { 0,0 };
	virtual void ArrangeSprites() override;
protected:
	virtual ~cSimpleMultiSprite()=default;
public:
	cSimpleMultiSprite(std::vector<std::unique_ptr<cSpriteBase>> &&Sprites);
};

USE_DROP_INSTEAD_DELETE_PARENT(cSimpleMultiSprite, cMultiSpriteBase);


class cAutoMultiSpriteBase: public cMultiSpriteBase
{
	std::vector<tIntrusivePtr<cTexture>> mTextureOverrides;
	void Cleanup();
protected:
	virtual const char **GetPositionNames() const=0;
	virtual bool FinalizeInit() { return true; }
	virtual ~cAutoMultiSpriteBase()=default;
public:
	void OverrideTexture(size_t Index, tIntrusivePtr<cTexture> Texture); // must be called before Init
	bool Init(cPixieWindow *Window, const std::string &TextureNameBase);
};

USE_DROP_INSTEAD_DELETE_PARENT(cAutoMultiSpriteBase, cMultiSpriteBase);