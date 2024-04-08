#pragma once

class cPushButton: public cDialogItem
{
public:
	enum eEventType: size_t
	{
		Event_Activated = PixieEvents::PushButton_First
	};
private:
	static const cEventDispatchers::cDispatcherRangeInfo mDispatcherRangeInfo;
public:
	struct cInitData: public tPixieSimpleInitData<cMouseTarget>
	{
		int mActivationKey=0;
		std::string mTextureBaseName;
		std::string mTitle;
		std::string mTextStyle { "push_button" };
		std::string mTooltipText;
//todo		std::unique_ptr<cSpriteBase> mTextSprite;
		cInitData()=default;
		void setConfig(std::shared_ptr<cConfig> Config);
	};
	class cVisualizer: public tVisualizerBase<cPushButton>
	{
	public:
		virtual void StateChanged()=0;
	};
protected:
	bool mIsMouseInside=false;
	bool mIsMouseButtonDown=false;
	cRegisteredID mKeyboardListeningID;
	HAS_VISUALIZER();
	virtual void OnMouseMove(cPoint ScreenCoords, bool IsInside) override;
	virtual void OnLeftButtonDown(cPoint ScreenCoords, bool IsInside) override;
	virtual void OnLeftButtonUp(cPoint ScreenCoords, bool IsInside) override;
	virtual void DialogItemStateChanged() override;
public:
	virtual ~cPushButton()=default;
	void Init(const cInitData &InitData);


	bool IsMouseInside() const { return mIsMouseInside&&!mIsMouseButtonDown; }
	bool IsPushed() const { return IsMouseTrackingActive()&&mIsMouseInside; }
};