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
        std::shared_ptr<cPixelShader> mShader;
        float mShaderParameters[4] = { 0, 0, 0, 0 };
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

	auto GetAlpha() const { return mProperties.mColor.GetAlpha(); }

	void SetRotation(float Rotation);
	void SetZOrder(int ZOrder);
	void SetRGBColor(cColor Color);
	void SetARGBColor(uint32_t Color);
	void SetARGBColor_ByPosition(unsigned int PositionFlags, uint32_t Color);
	void SetAlpha(DWORD Alpha); // 0 - 255, 0: solid, 255: transparent
    void setShader(std::shared_ptr<cPixelShader> Shader);
	void setShaderParam(int index, float value);
	float getShaderParam(int index) const;
    int shaderParamIndex(std::string_view name) const; // throws if not found

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

tIntrusivePtr<cPixieObjectAnimator> blendShaderParam(
	c2DRenderable& sprite, int paramOffset, float targetValue, int blendTime);

tIntrusivePtr<cPixieObjectAnimator> blendShaderParam(
	c2DRenderable& sprite, std::string_view paramID, float targetValue,	int blendTime);
