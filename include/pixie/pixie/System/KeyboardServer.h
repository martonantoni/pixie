#pragma once

class cKeyboardServer: public tSingleton<cKeyboardServer>
{
public:
	struct KeyCodes
	{
        static constexpr int Tab = 9;
        static constexpr int Enter = 0xd;
        static constexpr int Shift = 0x10;
        static constexpr int Esc = 27;
        static constexpr int PageUp = 0x21;
        static constexpr int PageDown = 0x22;
        static constexpr int End = 0x23;
        static constexpr int Home = 0x24;
        static constexpr int Left = 0x25;
        static constexpr int Up = 0x26;
        static constexpr int Right = 0x27;
        static constexpr int Down = 0x28;
        static constexpr int Delete = 0x2e;
        static constexpr int Backspace = 8;
        static constexpr int F1 = 0x70;
        static constexpr int F2 = 0x71;
        static constexpr int F3 = 0x72;
        static constexpr int F4 = 0x73;
        static constexpr int F5 = 0x74;
        static constexpr int F6 = 0x75;
	};

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

		Keyboard_KeyUp_First,
		Keyboard_KeyUp_Tab = Keyboard_KeyUp_First + 9,
		Keyboard_KeyUp_Enter = Keyboard_KeyUp_First + 0xd,
		Keyboard_KeyUp_Shift = Keyboard_KeyUp_First + 0x10,
		Keyboard_KeyUp_Esc = Keyboard_KeyUp_First + 27,
		Keyboard_KeyUp_PageUp = Keyboard_KeyUp_First + 0x21, Keyboard_KeyUp_PageDown, Keyboard_KeyUp_End, Keyboard_KeyUp_Home,
		Keyboard_KeyUp_Left = Keyboard_KeyUp_First + 0x25, Keyboard_KeyUp_Up, Keyboard_KeyUp_Right, Keyboard_KeyUp_Down,
		Keyboard_KeyUp_Delete = Keyboard_KeyUp_First + 0x2e,
		Keyboard_KeyUp_Backspace = Keyboard_KeyUp_First + 8,
		Keyboard_KeyUp_0 = Keyboard_KeyUp_First + '0',
		Keyboard_KeyUp_1, Keyboard_KeyUp_2, Keyboard_KeyUp_3, Keyboard_KeyUp_4, Keyboard_KeyUp_5,
		Keyboard_KeyUp_6, Keyboard_KeyUp_7, Keyboard_KeyUp_8, Keyboard_KeyUp_9,
		Keyboard_KeyUp_A = Keyboard_KeyUp_First + 'A',
		Keyboard_KeyUp_B, Keyboard_KeyUp_C, Keyboard_KeyUp_D, Keyboard_KeyUp_E, Keyboard_KeyUp_F,
		Keyboard_KeyUp_G, Keyboard_KeyUp_H, Keyboard_KeyUp_I, Keyboard_KeyUp_J, Keyboard_KeyUp_K,
		Keyboard_KeyUp_L, Keyboard_KeyUp_M, Keyboard_KeyUp_N, Keyboard_KeyUp_O, Keyboard_KeyUp_P,
		Keyboard_KeyUp_Q, Keyboard_KeyUp_R, Keyboard_KeyUp_S, Keyboard_KeyUp_T, Keyboard_KeyUp_U,
		Keyboard_KeyUp_V, Keyboard_KeyUp_W, Keyboard_KeyUp_X, Keyboard_KeyUp_Y, Keyboard_KeyUp_Z,
		Keyboard_KeyUp_F1 = Keyboard_KeyUp_First + 0x70,
		Keyboard_KeyUp_F2, Keyboard_KeyUp_F3, Keyboard_KeyUp_F4, Keyboard_KeyUp_F5, Keyboard_KeyUp_F6, Keyboard_KeyUp_F7,
		Keyboard_KeyUp_F8, Keyboard_KeyUp_F9, Keyboard_KeyUp_F10, Keyboard_KeyUp_F11, Keyboard_KeyUp_F12,
		Keyboard_KeyUp_Last = Keyboard_KeyUp_First + 255,
	};
private:
	std::array<bool, 256> mKeyState;
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
    cWindowsMessageResult OnKeyUp(WPARAM wParam, LPARAM lParam);
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
    bool isKeyDown(uint32_t KeyCode) const { return mKeyState[KeyCode]; }
};