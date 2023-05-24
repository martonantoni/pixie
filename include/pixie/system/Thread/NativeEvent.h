#pragma once

class cNativeEvent
{
	friend class cReactor;
public:
	enum eType { AutoReset, ManualReset };
private:
	HANDLE mHandle;
public:
	cNativeEvent(eType Type);
	explicit cNativeEvent(HANDLE Handle);
	cNativeEvent(const cNativeEvent &) = delete;
	const cNativeEvent &operator=(const cNativeEvent &) = delete;
	~cNativeEvent();
	void Set();
	void Reset();
	BOOL Wait(DWORD TimeOut);

	HANDLE GetHandle() const { return mHandle; } // only the cReactor should call this
};
