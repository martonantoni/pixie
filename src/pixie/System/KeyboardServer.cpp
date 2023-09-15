#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

tDataHolder<uint32_t> cKeyboardServer::mEventDataHolder;


const cEventDispatchers::cDispatcherRangeInfo cKeyboardServer::mDispatcherRangeInfo=
{
	PixieEvents::Keyboard_First, { "keydown", 
//            0 / 8         1 / 9         2 / a         3 / b         4 / c         5 / d         6 / e         7 / f
/* 00 - 08 */ "character",  "",           "",           "",           "",           "",           "",           "", 
/* 08 - 10 */ "keydown_bs", "keydown_tab","",           "",           "",           "keydown_enter",           "",           "", 
/* 10 - 18 */ "keydown_shift","",         "",           "",           "",           "",           "",           "", 
/* 18 - 20 */ "",           "",           "",           "keydown_esc","",           "",           "",           "", 
/* 20 - 28 */ "",           "keydown_page_up","keydown_page_down","keydown_end","keydown_home","keydown_left","keydown_up","keydown_right", 
/* 28 - 30 */ "keydown_down","",           "",           "",           "",           "",           "keydown_delete","", 
/* 30 - 38 */ "keydown_0",  "keydown_1",  "keydown_2",  "keydown_3",  "keydown_4",  "keydown_5",  "keydown_6",  "keydown_7", 
/* 38 - 40 */ "keydown_8",  "keydown_9",  "",           "",           "",           "",           "",           "", 
/* 40 - 48 */ "",           "keydown_a",  "keydown_b",  "keydown_c",  "keydown_d",  "keydown_e",  "keydown_f",  "keydown_g", 
/* 48 - 50 */ "keydown_h",  "keydown_i",  "keydown_j",  "keydown_k",  "keydown_l",  "keydown_m",  "keydown_n",  "keydown_o", 
/* 50 - 58 */ "keydown_p",  "keydown_q",  "keydown_r",  "keydown_s",  "keydown_t",  "keydown_u",  "keydown_v",  "keydown_w", 
/* 58 - 60 */ "keydown_x",  "keydown_y",  "keydown_z",  "",           "",           "",           "",           "",
/* 60 - 68 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* 68 - 70 */ "",           "",           "keydown_num*","keydown_num+","",         "",           "",           "keydown_num/", 
/* 70 - 78 */ "keydown_f1", "keydown_f2", "keydown_f3", "keydown_f4", "keydown_f5", "keydown_f6", "keydown_f7", "keydown_f8", 
/* 78 - 80 */ "keydown_f9", "keydown_f10","keydown_f11","keydown_f12",           "",           "",           "",           "", 
/* 80 - 88 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* 88 - 90 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* 90 - 98 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* 98 - a0 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* a0 - a8 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* a8 - b0 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* b0 - b8 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* b8 - c0 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* c0 - c8 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* c8 - d0 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* d0 - d8 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* d8 - e0 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* e0 - e8 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* e8 - f0 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* f0 - f8 */ "",           "",           "",           "",           "",           "",           "",           "", 
/* f8 - ff */ "",           "",           "",           "",           "",           "",           "",
}
};

int cKeyboardServer::ConfigStringToEventID(const std::string &ConfigString)
{
	auto i=std::find_if(mDispatcherRangeInfo.mEventNames, [NameToFind="keydown_"s+ConfigString](auto &Name) { return Name==NameToFind; });
	return i==mDispatcherRangeInfo.mEventNames.end()?0:i-mDispatcherRangeInfo.mEventNames.begin()-1; // -1 because of Keyboard_First is invalid
}

cKeyboardServer::cKeyboardServer()
{
	cPrimaryWindow &PrimaryWindow=cPrimaryWindow::Get();
	mListenerIDs.push_back(PrimaryWindow.AddMessageHandler(WM_KEYDOWN, [this](auto wp, auto lp) { return OnKeyDown(wp, lp); }));
    mListenerIDs.push_back(PrimaryWindow.AddMessageHandler(WM_CHAR, [this](auto wp, auto lp) { return OnCharacter(wp, lp); }));

	mEventDispatchers.Init(cEventDispatcher::GetGlobalDispatcher("pixie.keyboard",  cEventDispatcher::CanCreate));
	mEventDispatchers.AddEvents(mDispatcherRangeInfo);
	cEventDispatcher::cEventListenerRequest ListenerRequest;
	ListenerRequest.mOrder=0;
}

uint32_t cKeyboardServer::keyUpDownEventData(WPARAM wParam)
{
    uint32_t eventData = wParam & KeyCodeMask;
	if (GetKeyState(VK_SHIFT) & 128)
		eventData |= ShiftFlag;
    if (GetKeyState(VK_CONTROL) & 0x80)
        eventData |= CtrlFlag;
    if (GetKeyState(VK_MENU) & 128)
        eventData |= AltFlag;
	return eventData;
}

cWindowsMessageResult cKeyboardServer::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	auto eventData = keyUpDownEventData(wParam);
	mEventDispatchers.PostEvent(Keyboard_KeyDown_Any, cEvent(mEventDataHolder.StoreData(eventData)));
	if(wParam<=255&&!mDispatcherRangeInfo.mEventNames[wParam+1].empty())
		mEventDispatchers.PostEvent(Keyboard_KeyDown_First+wParam, cEvent(mEventDataHolder.StoreData(eventData)));
//    mEventDispatchers.PostEvent(Keyboard_Character_Any, cEvent(mEventDataHolder.StoreData(eventData)));
	return cWindowsMessageResult();
}

cWindowsMessageResult cKeyboardServer::OnCharacter(WPARAM wParam, LPARAM lParam)
{
    printf("WM_CHAR wParam: %d\n", wParam);
	auto packedKeyData = wParam;
    if (GetKeyState(VK_SHIFT) & 128)
		packedKeyData |= ShiftFlag;
    if (GetKeyState(VK_CONTROL) & 0x80)
		packedKeyData |= CtrlFlag;
    if (GetKeyState(VK_MENU) & 128)
		packedKeyData |= AltFlag;
    mEventDispatchers.PostEvent(Keyboard_Character_Any, cEvent(mEventDataHolder.StoreData(packedKeyData)));
    return cWindowsMessageResult();
}

bool cKeyboardServer::shiftState(const cEvent& keyboardEvent)
{
	return ShiftFlag & *mEventDataHolder.GetData(keyboardEvent.mEventDataID);
}

bool cKeyboardServer::altState(const cEvent& keyboardEvent)
{
    return AltFlag & *mEventDataHolder.GetData(keyboardEvent.mEventDataID);
}

bool cKeyboardServer::ctrlState(const cEvent& keyboardEvent)
{
    return CtrlFlag & *mEventDataHolder.GetData(keyboardEvent.mEventDataID);
}

char cKeyboardServer::displayableCharacter(const cEvent& keyboardEvent)
{
	uint32_t eventValue = *mEventDataHolder.GetData(keyboardEvent.mEventDataID);
	printf("displayableCharacter: %d\n", eventValue & KeyCodeMask);
	return eventValue & KeyCodeMask;
// 	bool isShiftDown = ShiftFlag & eventValue;
// 	uint32_t KeyCode = eventValue & KeyCodeMask;
// 	if(KeyCode>='A'&&KeyCode<='Z') return KeyCode+(isShiftDown?0:'a'-'A');
// 	if(KeyCode>='0'&&KeyCode<='9') return KeyCode;
// //	static const char *NonAlphaNum=" !$&*()-+?.,;:\"'/\|=_@";
// 	if(KeyCode==' ') 
//         return ' ';
// 	return 0;
}
