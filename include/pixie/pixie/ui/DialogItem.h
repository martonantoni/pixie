#pragma once

struct cDialogItemConfig;

class cDialogItem: public cMouseTarget, public cFocusable
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
	std::vector<cAutoEventListener> mKeyboardListeningIDs;
	void Init(const cPixieInitData &InitData);
	virtual void DialogItemStateChanged() {}
	virtual void onAcquiredFocus() override;
	virtual void onLostFocus() override;
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
	void AddPushButton(cPixieWindow &Window, tIntrusivePtr<cConfig> config);
	void AddTextField(cPixieWindow &Window, tIntrusivePtr<cConfig> config);
	void AddEditField(cPixieWindow &Window, tIntrusivePtr<cConfig> config);
public:
	void Init(cPixieWindow &Window, tIntrusivePtr<cConfig> Config);
	cDialogItem& item(int index) { return *mItems[index]; }
};
