#pragma once

class cThreadLocalStorage
{
	DWORD mThreadDataIndex;
public:
	cThreadLocalStorage();
	~cThreadLocalStorage();
	void *GetValue() const;
	void SetValue(void *NewValue) const;
};