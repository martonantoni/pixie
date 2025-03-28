#pragma once

class cPixieObjectAnimator;
enum class ePixieObjectAnimationDoneReason;

class cPixieObject
{
public:
	static const unsigned int PropertyType_Text = 0x80000000;
	enum // all property flags are listed here, even those that are not available for all PixieObjects
	{    // this is so that there is no accidental collision
		Property_X           =  0x0001,
		Property_Y           =  0x0002,
		Property_W           =  0x0004,
		Property_H           =  0x0008,
		Property_XOffset     =  0x0010,
		Property_YOffset     =  0x0020,
		Property_ZOrder      =  0x0040,
		Property_Color       =  0x0080,
		Property_Alpha       =  0x0100,
		Property_Texture     =  0x0200|PropertyType_Text,
		Property_Text        =  0x0400|PropertyType_Text,
		Property_Window      =  0x0800,
		Property_Visibility  =  0x1000,
		Property_Rotation    =  0x2000,
		Property_CenterAndHSize = 0x4000|Property_X|Property_Y|Property_W|Property_H,
		Property_Center      =  0x8000|Property_X|Property_Y,
		Property_ValidRect   =  0x10000,
		Property_ScreenX     =  0x20000,
		Property_ScreenY     =  0x40000,
		Property_TextureSize =  0x80000,  // cSprite & getter only
		Property_ClippingMode= 0x100000,
        Property_P1          = 0x200000,
        Property_P2          = 0x400000,
        Property_P3          = 0x800000,
		// combinations:
		Property_Position       = Property_X|Property_Y,
		Property_ScreenPosition = Property_ScreenX|Property_ScreenY,
		Property_Size           = Property_W|Property_H,
		Property_Rect           = Property_Position|Property_Size,
		Property_PositionOffset = Property_XOffset|Property_YOffset,
	};
	enum class eClippingMode 
	{ 
		None,          // no clipping
		Parent,        // validRect is relative to parent
		ParentParent,  // validRect is relative to parent's parent (commonly used for scrollable window areas)
		Screen,        // validRect's coordinates are screen coordinates
	}; // meaning that validRect is relative to parent or screen
	struct cPropertyValues;
	class cPropertyBinder;
	template<class T> class tPropertyBinder;
protected:
	cRegisteredIDList mKeyListeningIDs;
	tSafeObjects<tIntrusivePtr<cPixieObjectAnimator>> mAnimators2;
	cRegisteredID mAnimatorID;
	cPixieObjectAnimator *mAnimatorBeingExecuted=nullptr;
	typedef std::vector<std::unique_ptr<cPropertyBinder>> cPropertyBinders;
	cPropertyBinders mPropertyBinders;
	unsigned int mLockedProperties=0;
	void PropertiesSet(unsigned int Properties);
	bool CheckIfChangableProperty(unsigned int PropertiesBeingChanged) const;
	virtual void PropertiesChanged(unsigned int Properties) {}
public:
	virtual ~cPixieObject();
	virtual bool GetProperty(unsigned int PropertyFlags, OUT cPropertyValues &Value) const { return false; }
	virtual bool SetProperty(unsigned int PropertyFlags, const cPropertyValues &Value) { return false;  }
	virtual bool SetStringProperty(unsigned int PropertyFlags, const std::string &Value) { return false; }
	virtual bool GetFloatProperty(unsigned int PropertyFlags, OUT float &Value) const { return false; }
	virtual bool SetFloatProperty(unsigned int PropertyFlags, float Value) { return false; }
	void BindProperty(unsigned int PropertyFlags, const cResourceLocation &SharedVariableLocation);
	void AddAnimator(tIntrusivePtr<cPixieObjectAnimator> Animator);
	void RunAnimators();
	void KillAnimators();
};

