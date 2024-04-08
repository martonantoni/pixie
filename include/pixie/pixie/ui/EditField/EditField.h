#pragma once

class cEditField: public cDialogItem
{
public:
	enum eEventType: size_t
	{
		Event_Enter = PixieEvents::EditField_First,
	};
private:
	static const cEventDispatchers::cDispatcherRangeInfo mDispatcherRangeInfo;
public:
	struct cInitData: public tPixieSimpleInitData<cMouseTarget>
	{
		std::string mTitle;
		std::string mTextStyle { "edit_field" };
		int mMaxTextLength=std::numeric_limits<int>::max();
		cInitData()=default;
		void setConfig(std::shared_ptr<cConfig> config);
	};
	class cVisualizer: public tVisualizerBase<cEditField>
	{
	public:
		virtual void StateChanged()=0;
	};
protected:
	cRegisteredIDList mKeyListeningIDs;
	tDataHolder<uint32_t> &mKeyCodeHolder=tDataHolder<uint32_t>::Get();
	HAS_VISUALIZER();
	int mCursorPosition=0; // before which character
	int mMaxTextLength=0;
	void OnLeft();
	void OnRight();
	void OnHome();
	void OnEnd();
	void OnDelete();
	void OnBackspace();
	void OnEnter();
	void OnKey(const cEvent& event);
	virtual void DialogItemStateChanged() override;
	virtual void OnLeftButtonDown(cPoint ScreenCoords, bool IsInside) override;
	virtual void onAcquiredFocus() override;
	virtual void onLostFocus() override;
	virtual void PropertiesChanged(unsigned int Properties) override;
public:
	virtual ~cEditField()=default;
	void Init(const cInitData &InitData);
	int GetCursorPosition() const { return mCursorPosition; }
};