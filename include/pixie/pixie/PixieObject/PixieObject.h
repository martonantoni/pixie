#pragma once

class cPixieObjectAnimator;
enum class ePixieObjectAnimationDoneReason;

class cPixieObject
{
public:
	static constexpr unsigned int PropertyType_Text  = 0x80000000;
	// all property flags are listed here, even those that are not available for all PixieObjects
	// this is so that there is no accidental collision
	static constexpr unsigned int Property_X           =  0x0001;
	static constexpr unsigned int Property_Y           =  0x0002;
	static constexpr unsigned int Property_W           =  0x0004;
	static constexpr unsigned int Property_H           =  0x0008;
	static constexpr unsigned int Property_XOffset     =  0x0010;
	static constexpr unsigned int Property_YOffset     =  0x0020;
	static constexpr unsigned int Property_ZOrder      =  0x0040;
	static constexpr unsigned int Property_Color       =  0x0080;
	static constexpr unsigned int Property_Alpha       =  0x0100;
	static constexpr unsigned int Property_Texture     =  0x0200|PropertyType_Text;
	static constexpr unsigned int Property_Text        =  0x0400|PropertyType_Text;
	static constexpr unsigned int Property_Window      =  0x0800;
	static constexpr unsigned int Property_Visibility  =  0x1000;
	static constexpr unsigned int Property_Rotation    =  0x2000;
	static constexpr unsigned int Property_CenterAndHSize = 0x4000|Property_X|Property_Y|Property_W|Property_H;
	static constexpr unsigned int Property_Center      =  0x8000|Property_X|Property_Y;
	static constexpr unsigned int Property_ValidRect   =  0x10000;
	static constexpr unsigned int Property_ScreenX     =  0x20000;
	static constexpr unsigned int Property_ScreenY     =  0x40000;
	static constexpr unsigned int Property_TextureSize =  0x80000;  // cSprite & getter only
	static constexpr unsigned int Property_ClippingMode= 0x100000;
    static constexpr unsigned int Property_P1          = 0x200000;
    static constexpr unsigned int Property_P2          = 0x40'0000;
    static constexpr unsigned int Property_P3          = 0x80'0000;
	// combinations:
	static constexpr unsigned int Property_Position       = Property_X|Property_Y;
	static constexpr unsigned int Property_ScreenPosition = Property_ScreenX|Property_ScreenY;
	static constexpr unsigned int Property_Size           = Property_W|Property_H;
	static constexpr unsigned int Property_Rect           = Property_Position|Property_Size;
	static constexpr unsigned int Property_PositionOffset = Property_XOffset|Property_YOffset;
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

