#pragma once

class cKeyboardServer: public tSingleton<cKeyboardServer>
{
public:
	enum eKeyboardEvent
	{
		Keyboard_KeyDown_Any   = PixieEvents::Keyboard_First,
		// about the Keyboard_KeyDown_Any event:
		//    the reg_id in the event is for tDataHolder<uint32_t>, which holds the keycode for the event.
		//    Use the KeyBoardServer's function for dealing with the KeyCode, it is not usable in its own.
		Keyboard_KeyDown_First,
        Keyboard_Character_Any = Keyboard_KeyDown_First + 1,
		Keyboard_KeyDown_Tab= Keyboard_KeyDown_First+9,
		Keyboard_KeyDown_Enter= Keyboard_KeyDown_First+0xd,
		Keyboard_KeyDown_Shift= Keyboard_KeyDown_First+0x10,
		Keyboard_KeyDown_Esc= Keyboard_KeyDown_First+27,
		Keyboard_KeyDown_PageUp= Keyboard_KeyDown_First+0x21, Keyboard_KeyDown_PageDown, Keyboard_KeyDown_End, Keyboard_KeyDown_Home,
		Keyboard_KeyDown_Left= Keyboard_KeyDown_First+0x25, Keyboard_KeyDown_Up, Keyboard_KeyDown_Right, Keyboard_KeyDown_Down,
		Keyboard_KeyDown_Delete= Keyboard_KeyDown_First+0x2e,
		Keyboard_KeyDown_Backspace=Keyboard_KeyDown_First+8,
		Keyboard_KeyDown_0 = Keyboard_KeyDown_First+'0',
		Keyboard_KeyDown_1, Keyboard_KeyDown_2, Keyboard_KeyDown_3, Keyboard_KeyDown_4, Keyboard_KeyDown_5, 
		Keyboard_KeyDown_6, Keyboard_KeyDown_7, Keyboard_KeyDown_8, Keyboard_KeyDown_9,
		Keyboard_KeyDown_A = Keyboard_KeyDown_First+'A',
		Keyboard_KeyDown_B, Keyboard_KeyDown_C, Keyboard_KeyDown_D, Keyboard_KeyDown_E, Keyboard_KeyDown_F,
		Keyboard_KeyDown_G, Keyboard_KeyDown_H, Keyboard_KeyDown_I, Keyboard_KeyDown_J, Keyboard_KeyDown_K,
		Keyboard_KeyDown_L, Keyboard_KeyDown_M, Keyboard_KeyDown_N, Keyboard_KeyDown_O, Keyboard_KeyDown_P,
		Keyboard_KeyDown_Q, Keyboard_KeyDown_R, Keyboard_KeyDown_S, Keyboard_KeyDown_T, Keyboard_KeyDown_U,
		Keyboard_KeyDown_V, Keyboard_KeyDown_W, Keyboard_KeyDown_X, Keyboard_KeyDown_Y, Keyboard_KeyDown_Z,
		Keyboard_KeyDown_F1=Keyboard_KeyDown_First+0x70,
		Keyboard_KeyDown_F2, Keyboard_KeyDown_F3, Keyboard_KeyDown_F4, Keyboard_KeyDown_F5, Keyboard_KeyDown_F6, Keyboard_KeyDown_F7,
		Keyboard_KeyDown_F8, Keyboard_KeyDown_F9, Keyboard_KeyDown_F10, Keyboard_KeyDown_F11, Keyboard_KeyDown_F12,
		Keyboard_KeyDown_Last=Keyboard_KeyDown_First+255,
	};
private:
	static const cEventDispatchers::cDispatcherRangeInfo mDispatcherRangeInfo;
	cEventDispatchers mEventDispatchers;
	static tDataHolder<uint32_t> mEventDataHolder;
    static constexpr uint32_t ShiftFlag = 0x8000'0000u;
    static constexpr uint32_t AltFlag = 0x4000'0000u;
    static constexpr uint32_t CtrlFlag = 0x2000'0000u;
    static constexpr uint32_t KeyCodeMask = 0x0000'ffffu;
	enum { DefaultOrder = 100 };
	cRegisteredIDList mListenerIDs;
	cWindowsMessageResult OnKeyDown(WPARAM wParam,LPARAM lParam);
    cWindowsMessageResult OnCharacter(WPARAM wParam, LPARAM lParam);
	static uint32_t keyUpDownEventData(WPARAM wParam);
public:
	cKeyboardServer();
	tIntrusivePtr<cEventDispatcher> GetDispatcher(size_t EventIndex) const { return mEventDispatchers[EventIndex]; }
	static bool shiftState(const cEvent& keyboardEvent);
	static bool altState(const cEvent& keyboardEvent);
	static bool ctrlState(const cEvent& keyboardEvent);
	static char displayableCharacter(const cEvent& keyboardEvent);
	static int ConfigStringToEventID(const std::string &ConfigString);
};