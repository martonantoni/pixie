#pragma once

class cSpriteRenderer;
class cPixieWindow;
class cPixieObjectAnimator;
class cPixieWindow;
enum class ePixieObjectAnimationDoneReason;

class cSpriteBase : public cPixieObject
{
	bool mIsOwned = true; // later cSpriteBase might be refcounted
	int mDebugID = 0;
	static int mDebugIDCounter;
protected:
	cPixieWindow* mWindow = nullptr;
	struct cProperties
	{
		int mZOrder = 100;
		bool mVisible = false;
		cColor mColor;
		float mRotation = 0; // in degrees (because that's friendlier to work with)
		std::shared_ptr<cPixelShader> mShader;
		float mShaderParameters[4] = { 0, 0, 0, 0 };
	} mProperties;
	bool mIsColorSet = false;
	virtual ~cSpriteBase(); // use Drop() or Destroy()
	void CopyProperties(const cSpriteBase& source); // used by Clone()

	cRect mRect;
	cPoint mPositionOffset;
public:
    cSpriteBase();
	void Drop();
	void Destroy();
	std::string GetDebugID() const { return std::format("sprite#{}", mDebugID); }

	virtual std::unique_ptr<cSpriteBase> Clone() const = 0;

	virtual bool GetProperty(unsigned int PropertyFlags, OUT cPropertyValues& Value) const override;
	virtual bool SetProperty(unsigned int PropertyFlags, const cPropertyValues& Value) override;
	virtual bool SetStringProperty(unsigned int PropertyFlags, const std::string& Value) override;
	virtual bool GetFloatProperty(unsigned int PropertyFlags, OUT float& Value) const override;
	virtual bool SetFloatProperty(unsigned int PropertyFlags, float Value) override;

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


	int GetZOrder() const { return mProperties.mZOrder; }
	float GetRotation() const { return mProperties.mRotation; }

	const cColor& GetColor() const { return mProperties.mColor; }

	auto GetAlpha() const { return mProperties.mColor.GetAlpha(); }

	void SetRotation(float Rotation);
	void SetZOrder(int ZOrder);
	void SetRGBColor(cColor Color);
	void SetARGBColor(uint32_t Color);
	void SetAlpha(DWORD Alpha); // 0 - 255, 0: solid, 255: transparent

	void setShader(std::shared_ptr<cPixelShader> Shader);
	void setShader(const std::string& shaderId);
	std::shared_ptr<cPixelShader> getShader() const { return mProperties.mShader; }
	void setShaderParam(int index, float value);
	float getShaderParam(int index) const;
	int shaderParamIndex(std::string_view name) const; // throws if not found

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
	virtual void updateTextures() {}

};

USE_DROP_INSTEAD_DELETE(cSpriteBase);

tIntrusivePtr<cPixieObjectAnimator> blendShaderParam(
	cSpriteBase& sprite, int paramOffset, float targetValue, int blendTime);

tIntrusivePtr<cPixieObjectAnimator> blendShaderParam(
	cSpriteBase& sprite, std::string_view paramID, float targetValue, int blendTime);


