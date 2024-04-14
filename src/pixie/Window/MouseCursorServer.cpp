#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cMouseCursorServer::cMouseCursorServer()
{
#ifdef NO_CUSTOM_CURSORS
	return;
#else
	//std::shared_ptr<cConfig> Config=cLuaBasedConfig::CreateConfig("pixie_system.folders", theLuaState);
	//std::string CursorsFolder=Config->get<std::string>("cursors");
	//WIN32_FIND_DATA FindData;
	//memset(&FindData, 0, sizeof(WIN32_FIND_DATA));
	//HANDLE FindHandle=::FindFirstFile(fmt::sprintf("%s/*.ani", CursorsFolder.c_str()).c_str(), &FindData);
	//if(FindHandle!=INVALID_HANDLE_VALUE)
	//{
	//	do
	//	{
	//		cPath Path(CursorsFolder+"/"+std::string(FindData.cFileName));
	//		auto CursorHandle=LoadCursorFromFile(Path.c_str());
	//		if(CursorHandle!=NULL)
	//		{
	//			mCursors[Path.GetFileNameWithoutExtension()]=CursorHandle;
	//		}
	//		else
	//		{
	//			MainLog->Log("Warning: LoadCursorFromFile failed for \"%s\"", FindData.cFileName);
	//		}
	//	} while(::FindNextFile(FindHandle, &FindData));
	//	::FindClose(FindHandle);
	//}

	//cCursorData CursorData;
	//CursorData.mCursorHandle=mCursors["kg_basic"];
	//CursorData.mID=0;
	//mCursorStack.push_back(CursorData);

	mCursors["arrow"] = LoadCursor(NULL, IDC_ARROW);
	mCursors["ibeam"] = LoadCursor(NULL, IDC_IBEAM);
	mCursors["wait"] = LoadCursor(NULL, IDC_WAIT);
	mCursors["cross"] = LoadCursor(NULL, IDC_CROSS);
	mCursors["uparrow"] = LoadCursor(NULL, IDC_UPARROW);
	mCursors["size_nwse"] = LoadCursor(NULL, IDC_SIZENWSE);
	mCursors["size_nesw"] = LoadCursor(NULL, IDC_SIZENESW);
	mCursors["size_we"] = LoadCursor(NULL, IDC_SIZEWE);
	mCursors["size_ns"] = LoadCursor(NULL, IDC_SIZENS);
	mCursors["size_all"] = LoadCursor(NULL, IDC_SIZEALL);
	mCursors["no"] = LoadCursor(NULL, IDC_NO);
	mCursors["hand"] = LoadCursor(NULL, IDC_HAND);
	mCursors["appstarting"] = LoadCursor(NULL, IDC_APPSTARTING);
	mCursors["help"] = LoadCursor(NULL, IDC_HELP);

	mCursorStack.emplace_back(mCursors["arrow"], 0);

	theIDSink=cPrimaryWindow::Get().AddMessageHandler(WM_SETCURSOR,
		[this](auto wp, auto lp) 
		{ 
			::SetCursor(mCursorStack.back().mCursorHandle); 
			return cWindowsMessageResult(TRUE); 
		});
#endif
}

cRegisteredID cMouseCursorServer::SetCursor(const std::string& id)
{
#ifdef NO_CUSTOM_CURSORS
	return cRegisteredID();
#else
	auto i=mCursors.find(id);
	if(i!=mCursors.end())
	{
		mCursorStack.emplace_back(i->second, ++mIDCounter);
		return cRegisteredID(this, mIDCounter);
	}
	MainLog->Log("Warning: Cursor \"%s\" is not available", id.c_str());
	return cRegisteredID();
#endif
}

void cMouseCursorServer::Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType)
{
#ifndef NO_CUSTOM_CURSORS
	auto i=std::ranges::find_if(mCursorStack, [ID=RegisteredID.GetID()](auto &CursorData) { return CursorData.mID==ID; });
	if(i!=mCursorStack.end())
		mCursorStack.erase(i);
	::SetCursor(mCursorStack.back().mCursorHandle);
#endif
}
