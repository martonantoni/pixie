#pragma once

struct cDialogItemConfig;

class cDialogItem: public cMouseTarget
{
	cSharedBool mIsEnabled=true;
	cSharedBool mIsVisible=true;
	cSharedString mText=std::string();
	cSharedString mTooltipText=std::string();
	cRegisteredID mFocusID;
	cRegisteredIDList mListeningIDs;
	cRegisteredID mTooltipID;
	void TooltipTextChanged();
protected:
	void Init(const cPixieInitData &InitData);
	virtual void DialogItemStateChanged() {}
	virtual void OnFocused() {}
	virtual void OnLostFocus() {}
	virtual void OnMouseMove(cPoint ScreenCoords, bool IsInside) override;
public:
	virtual bool SetStringProperty(unsigned int PropertyFlags, const std::string &Value) override;

	void SetEnabled(bool IsEnabled);
	void Disable() { SetEnabled(false); }
	void Enable() { SetEnabled(true); }
	bool IsEnabled() const { return mIsEnabled&&mIsVisible; }

	void SetVisible(bool IsVisible);
	void Hide() { SetVisible(false); }
	void Show() { SetVisible(true); }
	bool IsVisible() const { return mIsVisible; }

	void LostFocus();
	bool IsInFocus() const { return mFocusID.IsValid(); }
	void SetFocus();

	void SetTooltip(const std::string &TooltipText)
	{
		mTooltipText=TooltipText;
	}

	void SetText(const std::string &Text);
	std::string GetText() const { return mText; }
};

class cDialogItems
{
	std::vector<std::unique_ptr<cDialogItem>> mItems;
	void AddPushButton(cPixieWindow &Window, const cConfig& Config);
	void AddTextField(cPixieWindow &Window, const cConfig& Config);
	void AddEditField(cPixieWindow &Window, const cConfig& Config);
public:
	void Init(cPixieWindow &Window, const cConfig &Config);
};
