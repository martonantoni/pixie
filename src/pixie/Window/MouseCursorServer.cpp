#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cMouseCursorServer::cMouseCursorServer()
{
#ifdef NO_CUSTOM_CURSORS
	return;
#else
	tIntrusivePtr<cConfig2> Config=cLuaBasedConfig::CreateConfig("pixie_system.folders", theLuaState);
	std::string CursorsFolder=Config->get<std::string>("cursors");
	WIN32_FIND_DATA FindData;
	memset(&FindData, 0, sizeof(WIN32_FIND_DATA));
	HANDLE FindHandle=::FindFirstFile(fmt::sprintf("%s/*.ani", CursorsFolder.c_str()).c_str(), &FindData);
	if(FindHandle!=INVALID_HANDLE_VALUE)
	{
		do
		{
			cPath Path(CursorsFolder+"/"+std::string(FindData.cFileName));
			auto CursorHandle=LoadCursorFromFile(Path.c_str());
			if(CursorHandle!=NULL)
			{
				mCursors[Path.GetFileNameWithoutExtension()]=CursorHandle;
			}
			else
			{
				MainLog->Log("Warning: LoadCursorFromFile failed for \"%s\"", FindData.cFileName);
			}
		} while(::FindNextFile(FindHandle, &FindData));
		::FindClose(FindHandle);
	}

	cCursorData CursorData;
	CursorData.mCursorHandle=mCursors["kg_basic"];
	CursorData.mID=0;
	mCursorStack.push_back(CursorData);

	theIDSink=cPrimaryWindow::Get().AddMessageHandler(WM_SETCURSOR,
		[this](auto wp, auto lp) 
		{ 
			::SetCursor(mCursorStack.back().mCursorHandle); 
			return cWindowsMessageResult(TRUE); 
		});
#endif
}

cRegisteredID cMouseCursorServer::SetCursor(const std::string &CursorName)
{
#ifdef NO_CUSTOM_CURSORS
	return cRegisteredID();
#else
	auto i=mCursors.find(CursorName);
	if(i!=mCursors.end())
	{
		cCursorData CursorData;
		CursorData.mCursorHandle=i->second;
		CursorData.mID=++mIDCounter;
		mCursorStack.push_back(CursorData);
		return cRegisteredID(this, mIDCounter);
	}
	MainLog->Log("Warning: Cursor \"%s\" is not available", CursorName.c_str());
	return cRegisteredID();
#endif
}

void cMouseCursorServer::Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType)
{
#ifndef NO_CUSTOM_CURSORS
	auto i=std::find_if(mCursorStack, [ID=RegisteredID.GetID()](auto &CursorData) { return CursorData.mID==ID; });
	if(i!=mCursorStack.end())
		mCursorStack.erase(i);
	::SetCursor(mCursorStack.back().mCursorHandle);
#endif
}
