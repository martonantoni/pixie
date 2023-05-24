#include "StdAfx.h"

bool Global_HandleAssert(const std::string &Text,int *Ignored)
{
	if(!Ignored||!*Ignored)
	{
		auto FinalText=Text+"\nSelect \"yes\" to debug, \"no\" to continue!";
		switch(::MessageBox(NULL, FinalText.c_str(),"spellfire",Ignored?MB_YESNOCANCEL:MB_YESNO))
		{
		case IDYES:
			return true;
		case IDNO:
			break;
		case IDCANCEL:
			*Ignored=true;
			break;
		}
	}
	return false;
}

void ThrowLastError(const std::string &Text)
{
	DWORD Error=GetLastError();
	RELEASE_ASSERT_EXT(false,fmt::sprintf("%s failed, errorcode: %u",Text,Error));
}
