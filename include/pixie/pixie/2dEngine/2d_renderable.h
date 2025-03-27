#pragma once

class cSpriteRenderer;
class cPixieWindow;
class cPixieObjectAnimator;
class cPixieWindow;
enum class ePixieObjectAnimationDoneReason;

class c2DRenderable : public cPixieObject
{
	bool mIsOwned = true; // later c2DRenderable might be refcounted
	int mDebugID = 0;
	static int mDebugIDCounter;
protected:
	cPixieWindow* mWindow = nullptr;
	struct cProperties
	{
		int mZOrder = 100;
		bool mVisible = false;
		cSpriteColor mColor;
		float mRotation = 0; // in degrees (because that's friendlier to work with)
		cRect mValidRect;
		eClippingMode mClippingMode = eClippingMode::None;
	} mProperties;
	bool mIsColorSet = false;
	virtual ~c2DRenderable(); // use Drop() or Destroy()
	void CopyProperties(const c2DRenderable& source); // used by Clone()
public:
	c2DRenderable();
	void Drop();
	void Destroy();
	std::string GetDebugID() const { return fmt::sprintf("sprite#%d", mDebugID); }

	virtual bool GetProperty(unsigned int PropertyFlags, OUT cPropertyValues& Value) const override;
	virtual bool SetProperty(unsigned int PropertyFlags, const cPropertyValues& Value) override;
	virtual bool SetStringProperty(unsigned int PropertyFlags, const std::string& Value) override;
	virtual bool GetFloatProperty(unsigned int PropertyFlags, OUT float& Value) const override;
	virtual bool SetFloatProperty(unsigned int PropertyFlags, float Value) override;

	int GetZOrder() const { return mProperties.mZOrder; }
	float GetRotation() const { return mProperties.mRotation; }

	const cColor& GetColor() const { return mProperties.mColor; }
	const cSpriteColor& GetSpriteColor() const { return mProperties.mColor; }

	DWORD GetAlpha() const { return mProperties.mColor.GetAlpha(); }

	void SetRotation(float Rotation);
	void SetZOrder(int ZOrder);
	void SetRGBColor(cColor Color);
	void SetARGBColor(D3DCOLOR Color);
	void SetARGBColor_ByPosition(unsigned int PositionFlags, D3DCOLOR Color);
	void SetAlpha(DWORD Alpha); // 0 - 255, 0: solid, 255: transparent

	void setClippingMode(eClippingMode ClippingMode);
	eClippingMode getClippingMode() const { return mProperties.mClippingMode; }
	void SetValidRect(const cRect& ValidRect); // for clipping
	cRect GetValidRect() const { return mProperties.mValidRect; }
	void DisableClipping();

	void Show();
	void Hide();
	bool IsVisible() const { return mProperties.mVisible; }
	void SetVisible(bool IsVisible);
	void SetWindow(cPixieWindow* Window);
	cPixieWindow* GetWindow() const { return mWindow; }

	//-----------------------------------
	enum eDestroyZombieResult { StillAlive, Destroyed };
	eDestroyZombieResult DestroyZombie(); // only cPixieWindow is allowed to call this
	bool IsDestroyable() const;

	virtual cSpriteRenderInfo GetRenderInfo() const { return cSpriteRenderInfo(); }
};

USE_DROP_INSTEAD_DELETE(c2DRenderable)