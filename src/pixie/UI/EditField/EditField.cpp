#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
INIT_VISUALIZER_IMP(cEditField);

const cEventDispatchers::cDispatcherRangeInfo cEditField::mDispatcherRangeInfo=
{
	PixieEvents::EditField_First, { "enter" }
};


cEditField::cInitData::cInitData(const tIntrusivePtr<cConfig> &Config): tPixieSimpleInitData<cMouseTarget>(Config)
{
	mTitle=Config->GetString("title", std::string());
	mTextStyle=Config->GetString("text_style", mTextStyle);
	mMaxTextLength=Config->GetInt("max_length", mMaxTextLength);
}

void cEditField::Init(const cInitData &InitData)
{
	mMaxTextLength=InitData.mMaxTextLength;
	cDialogItem::Init(InitData);
	mEventDispatchers.AddEvents(mDispatcherRangeInfo);
	InitVisualizer(InitData, InitData.mVisualizer);
	if(!InitData.mTitle.empty())
	{
		SetText(InitData.mTitle);
	}
	mCursorPosition=(int)GetText().size();
}

void cEditField::DialogItemStateChanged()
{
	mVisualizer->StateChanged();
}

void cEditField::OnLeft()
{
	if(mCursorPosition>0)
	{
		--mCursorPosition;
		DialogItemStateChanged();
	}
}

void cEditField::OnRight()
{
	if(mCursorPosition<GetText().length())
	{
		++mCursorPosition;
		DialogItemStateChanged();
	}
}

void cEditField::OnHome()
{
	mCursorPosition=0;
	DialogItemStateChanged();
}

void cEditField::OnEnd()
{
	mCursorPosition=(int)GetText().length();
	DialogItemStateChanged();
}

void cEditField::OnDelete()
{
	auto Text=GetText();
	if(mCursorPosition!=Text.length())
	{
		Text.erase(Text.begin()+mCursorPosition);
		SetText(Text);
	}
}

void cEditField::OnBackspace()
{
	if(mCursorPosition!=0)
	{
		--mCursorPosition;
		auto Text=GetText();
		Text.erase(Text.begin()+mCursorPosition);
		SetText(Text);
	}
}

void cEditField::OnKey(uint32_t KeyCode)
{
	auto DisplayableCharacter=cKeyboardServer::GetDisplayableCharacter(KeyCode);
	if(!DisplayableCharacter)
		return;
	auto Text=GetText();
	if(Text.length()>=mMaxTextLength)
		return;
	Text.insert(Text.begin()+mCursorPosition, DisplayableCharacter);
	++mCursorPosition;
	SetText(Text);
}

void cEditField::OnFocused()
{
	if(ASSERTFALSE(!mKeyListeningIDs.empty()))
		OnLostFocus();
	auto &KeyboardServer=cKeyboardServer::Get();
	mKeyListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_Enter)->RegisterListener([this](auto &Event) { OnEnter(); }));
	mKeyListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_Left)->RegisterListener([this](auto &Event) { OnLeft(); }));
	mKeyListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_Right)->RegisterListener([this](auto &Event) { OnRight(); }));
	mKeyListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_Home)->RegisterListener([this](auto &Event) { OnHome(); }));
	mKeyListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_End)->RegisterListener([this](auto &Event) { OnEnd(); }));
	mKeyListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_Delete)->RegisterListener([this](auto &Event) { OnDelete(); }));
	mKeyListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_Backspace)->RegisterListener([this](auto &Event) { OnBackspace(); }));
	mKeyListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_Any)->RegisterListener([this](const cEvent &Event)
	{
		OnKey(*mKeyCodeHolder.GetData(Event.mEventDataID));
	}));
}

void cEditField::OnLostFocus()
{
	mKeyListeningIDs.clear();
}

void cEditField::OnLeftButtonDown(cPoint ScreenCoords, bool IsInside)
{
	if(mKeyListeningIDs.empty())
		SetFocus();
}

void cEditField::OnEnter()
{
	mEventDispatchers.PostEvent(Event_Enter);
}

void cEditField::PropertiesChanged(unsigned int Properties)
{
	if(Properties&Property_Text)
	{
		mCursorPosition=std::min(mCursorPosition, (int)GetText().length());
	}
}
