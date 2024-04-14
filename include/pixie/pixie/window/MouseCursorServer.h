#pragma once

class cMouseCursorServer: public tSingleton<cMouseCursorServer>, public cRegistrationHandler
{
	std::unordered_map<std::string, HCURSOR> mCursors;
	struct cCursorData
	{
		HCURSOR mCursorHandle;
		unsigned int mID;
	};
	unsigned int mIDCounter=0;
	std::vector<cCursorData> mCursorStack;
	virtual void Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType=eCallbackType::Wait) override;
public:
	cMouseCursorServer();
	cRegisteredID SetCursor(const std::string& id);
};