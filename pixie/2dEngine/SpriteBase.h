#pragma once

class cSpriteRenderer;
class cPixieWindow;
class cPixieObjectAnimator;
class cPixieWindow;
enum class ePixieObjectAnimationDoneReason;

class cSpriteBase: public cPixieObject
{
public:
	struct cRenderInfo;
private:
	bool mIsOwned=true; // later cSpriteBase might be refcounted
	int mDebugID=0;
	static int mDebugIDCounter;
protected:
	cPixieWindow *mWindow=nullptr;
	struct cProperties
	{
		cRect mRect;
		cPoint mPositionOffset;
		int mZOrder=100;
		bool mVisible=false;
		cSpriteColor mColor;
		float mRotation=0; // in degrees (because that's friendlier to work with)
	} mProperties;
	bool mIsColorSet=false;
	bool mUseClipping=false;
	cRect mValidRect;
	virtual ~cSpriteBase(); // use Drop() or Destroy()
public:
	cSpriteBase();
	void Drop();
	void Destroy(); 
	std::string GetDebugID() const { return fmt::sprintf("sprite#%d",mDebugID); }

	virtual bool GetProperty(unsigned int PropertyFlags, OUT cPropertyValues &Value) const override;
	virtual bool SetProperty(unsigned int PropertyFlags, const cPropertyValues &Value) override;
	virtual bool SetStringProperty(unsigned int PropertyFlags, const std::string &Value) override;
	virtual bool GetFloatProperty(unsigned int PropertyFlags, OUT float &Value) const override;
	virtual bool SetFloatProperty(unsigned int PropertyFlags, float Value) override;

	int GetX() const { return mProperties.mRect.mLeft; }
	int GetY() const { return mProperties.mRect.mTop; }
	cPoint GetPosition() const { return mProperties.mRect.GetPosition(); }
	cPoint GetScreenPosition() const;
	cPoint GetCenter() const;
	int GetWidth() const { return mProperties.mRect.mWidth; }
	int GetHeight() const { return mProperties.mRect.mHeight; }
	cPoint GetSize() const { return mProperties.mRect.GetSize(); }
	const cRect &GetRect() const { return mProperties.mRect; }
	cRect GetCenterAndHSize() const;
	cRect GetRectForRendering() const; // this one includes X and Y offset
	int GetZOrder() const { return mProperties.mZOrder; }
	float GetRotation() const { return mProperties.mRotation; }
	virtual cPoint GetPrefferedSize() const { return GetSize(); }

	const cColor &GetColor() const { return mProperties.mColor; }
	const cSpriteColor &GetSpriteColor() const { return mProperties.mColor;	}

//	D3DCOLOR GetARGBColor() const { return (0xffffff&mProperties.mRGBColor)|((255-mProperties.mAlpha)<<24); }
//	DWORD GetRGBColor() const { return mProperties.mColor.GetRGBColor(); }
	DWORD GetAlpha() const { return mProperties.mColor.GetAlpha(); }

	void SetRect(const cRect &Rect);
	void SetCenterAndHSize(const cRect &Rect);
	void SetCenter(cPoint Point);
	void SetPosition(int x,int y) { SetPosition(cPoint(x,y)); }
	void SetPosition(cPoint Position);
	void SetPlacement(const cRect &Rect) { SetPosition(Rect.GetPosition()); SetSize(Rect.GetSize()); }
	void SetScreenPosition(cPoint Position);
	void SetPositionOffset(int XOffset,int YOffset) { SetPositionOffset(cPoint(XOffset,YOffset)); }
	void SetPositionOffset(const cPoint &PositionOffset);
	cPoint GetPositionOffset() const { return mProperties.mPositionOffset; }
	void SetSize(int Width,int Height) { SetSize(cPoint(Width,Height)); }
	void SetSize(cPoint Size);
	void SetRotation(float Rotation);
	void SetZOrder(int ZOrder);
	void SetRGBColor(cColor Color);
	void SetARGBColor(D3DCOLOR Color);
	void SetARGBColor_ByPosition(unsigned int PositionFlags,D3DCOLOR Color);
	void SetAlpha(DWORD Alpha); // 0 - 255, 0: solid, 255: transparent

	void SetValidRect(const cRect &ValidRect); // for clipping
	cRect GetValidRect() const { return mValidRect; }
	void DisableClipping();

	void Show();
	void Hide();
	bool IsVisible() const { return mProperties.mVisible; }
	void SetVisible(bool IsVisible);
	void SetWindow(cPixieWindow *Window);
	cPixieWindow *GetWindow() const { return mWindow; }
//-----------------------------------
 	enum eDestroyZombieResult { StillAlive, Destroyed };	
	eDestroyZombieResult DestroyZombie(); // only cPixieWindow is allowed to call this
	bool IsDestroyable() const;

	virtual cSpriteRenderInfo GetRenderInfo() const { return cSpriteRenderInfo(); }
};

USE_DROP_INSTEAD_DELETE(cSpriteBase)


