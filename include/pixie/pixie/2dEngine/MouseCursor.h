#pragma once

class cMouseCursor: public tSingleton<cMouseCursor>
{
	std::unique_ptr<cSprite> mSprite;
	tIntrusivePtr<cTexture> mDefaultCursorTexture;
	bool mIsHidden=false;
	tDataHolder<cPoint> &mPointHolder=tDataHolder<cPoint>::Get();
	cAutoEventListener mMoveListener;
	void OnMouseMove(const cEvent &Event);
	cWindowsMessageResult OnSetCursor(WPARAM wParam,LPARAM lParam);
public:
	cMouseCursor();
	~cMouseCursor();
	void SetTexture(tIntrusivePtr<cTexture> Texture); // NULL to set it back to default

	void Show();
	void Hide();
};