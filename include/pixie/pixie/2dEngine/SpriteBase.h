#pragma once

class cSpriteRenderer;
class cPixieWindow;
class cPixieObjectAnimator;
class cPixieWindow;
enum class ePixieObjectAnimationDoneReason;

class c2DRenderable: public cPixieObject
{
public:
//	struct cRenderInfo;
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
		float mRotation = 0; // in degrees (because that's friendlier to work with)
		cRect mValidRect; 
		eClippingMode mClippingMode = eClippingMode::None;
	} mProperties;
	bool mIsColorSet=false;
	virtual ~c2DRenderable(); // use Drop() or Destroy()
    void CopyProperties(const c2DRenderable& source); // used by Clone()
public:
	c2DRenderable();
	void Drop();
	void Destroy(); 
	std::string GetDebugID() const { return fmt::sprintf("sprite#%d",mDebugID); }

	virtual bool GetProperty(unsigned int PropertyFlags, OUT cPropertyValues &Value) const override;
	virtual bool SetProperty(unsigned int PropertyFlags, const cPropertyValues &Value) override;
	virtual bool SetStringProperty(unsigned int PropertyFlags, const std::string &Value) override;
	virtual bool GetFloatProperty(unsigned int PropertyFlags, OUT float &Value) const override;
	virtual bool SetFloatProperty(unsigned int PropertyFlags, float Value) override;

	int GetX() const { return mProperties.mRect.left(); }
	int GetY() const { return mProperties.mRect.top(); }
	cPoint GetPosition() const { return mProperties.mRect.position(); }
	cPoint GetScreenPosition() const;
	cPoint center() const;
	int GetWidth() const { return mProperties.mRect.width(); }
	int GetHeight() const { return mProperties.mRect.height(); }
	cPoint GetSize() const { return mProperties.mRect.size(); }
	const cRect &GetRect() const { return mProperties.mRect; }
	cRect GetCenterAndHSize() const;
	cRect GetRectForRendering() const; // this one includes X and Y offset
	int GetZOrder() const { return mProperties.mZOrder; }
	float GetRotation() const { return mProperties.mRotation; }
	virtual cPoint GetPrefferedSize() const { return GetSize(); }

	const cColor &GetColor() const { return mProperties.mColor; }
	const cSpriteColor &GetSpriteColor() const { return mProperties.mColor;	}

	DWORD GetAlpha() const { return mProperties.mColor.GetAlpha(); }

	void SetRect(const cRect &Rect);
	void SetPlacement(const cRect& Rect) { SetRect(Rect); } // just an alias
	void SetCenterAndHSize(const cRect &Rect);
	void SetCenter(cPoint Point);
	void SetPosition(int x,int y) { SetPosition(cPoint(x,y)); }
	void SetPosition(cPoint Position);
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

	void setClippingMode(eClippingMode ClippingMode);
	eClippingMode getClippingMode() const { return mProperties.mClippingMode; }
	void SetValidRect(const cRect &ValidRect); // for clipping
	cRect GetValidRect() const { return mProperties.mValidRect; }
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

USE_DROP_INSTEAD_DELETE(c2DRenderable)

class cSpriteBase : public c2DRenderable
{
public:
	virtual std::unique_ptr<cSpriteBase> Clone() const = 0;
};

USE_DROP_INSTEAD_DELETE_PARENT(cSpriteBase, c2DRenderable);


