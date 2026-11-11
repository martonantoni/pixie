#pragma once

template<class SpriteT>
    requires (std::same_as<SpriteT, cSpriteBase> || std::same_as<SpriteT, cSprite>)
class tMultiSpriteBase : public cSpriteBase
{
public:
	using cSprites = std::vector<std::unique_ptr<SpriteT>>;
protected:
	cSprites mSprites;
	virtual void ArrangeSprites() {}
	virtual void PropertiesChanged(unsigned int Properties) override;
	virtual ~tMultiSpriteBase() = default;
public:
	tMultiSpriteBase() = default;
	tMultiSpriteBase(cSprites&& sprites) : mSprites(std::move(sprites)) {}

	size_t GetNumberOfSubSprites() const { return mSprites.size(); }
	auto size() const { return mSprites.size(); }
	SpriteT* GetSubSprite(size_t Index) const { return mSprites[Index].get(); }
	auto subSprite(size_t Index) const { return mSprites[Index].get(); }
	std::vector<std::unique_ptr<SpriteT>> ReleaseSubSprites() { return std::move(mSprites); }


	enum class ePositioning { TopLeft, TopRight, BottomLeft, BottomRight }; // which point of the sprite is specified
	static void PositionSprite(cSpriteBase &Sprite, cPoint Position, ePositioning Positioning);
};

using cMultiSpriteBase = tMultiSpriteBase<cSpriteBase>;
using cMultiSprite = tMultiSpriteBase<cSprite>;


USE_DROP_INSTEAD_DELETE_PARENT(cMultiSpriteBase, cSpriteBase);

class cSimpleMultiSprite: public cMultiSpriteBase
{
	cPoint mBasePos { 0,0 };
	virtual void ArrangeSprites() override;
protected:
	virtual ~cSimpleMultiSprite()=default;
public:
    enum class eBasePosition { Zero, AdjustedToBoundingBox };
	cSimpleMultiSprite(std::vector<std::unique_ptr<cSpriteBase>> &&Sprites, eBasePosition basePosition = eBasePosition::AdjustedToBoundingBox);
    cSimpleMultiSprite() = default; // used by Clone, must be public because of make_unique
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

class cRectBorderMultiSprite : public cMultiSpriteBase
{
	virtual void ArrangeSprites() override;
	int mBorderWidth;
	struct
	{
		static constexpr int top = 0;
		static constexpr int right = 1;
		static constexpr int bottom = 2;
		static constexpr int left = 3;
	} spriteIndexes;
public:
	cRectBorderMultiSprite(int borderWidth);
};

USE_DROP_INSTEAD_DELETE_PARENT(cRectBorderMultiSprite, cMultiSpriteBase);
