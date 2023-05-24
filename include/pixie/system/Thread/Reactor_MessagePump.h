#pragma once

class cReactor_MessagePump: public cReactor
{
	virtual DWORD Wait(DWORD NumberOfHandles,DWORD Timeout) override;
};