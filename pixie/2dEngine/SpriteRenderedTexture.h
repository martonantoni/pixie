#pragma once

class cSpriteRenderedTexture: public cTexture
{
	std::unique_ptr<cPixieWindow> mSpriteContainerWindow;
	std::unique_ptr<cSpriteRenderer> mSpriteRenderer;
	typedef std::vector<std::unique_ptr<cSpriteBase>> cOwnedSprites;
	cOwnedSprites mOwnedSprites;
	cSpriteRenderedTexture(cPoint Size);
	void Cleanup();
public:
	virtual ~cSpriteRenderedTexture();
	void AddSprite(std::unique_ptr<cSpriteBase> Sprite);
	static tIntrusivePtr<cSpriteRenderedTexture> Create(const cPoint &Size);
	virtual void Update() override;
};