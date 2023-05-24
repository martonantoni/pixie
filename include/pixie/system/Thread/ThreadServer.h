#pragma once

class cThreadServer
{
	typedef std::map<std::string,std::unique_ptr<cThread>> cThreadMap;
	cThreadMap mThreadMap;
	cMutex mMutex;
public:
	cThreadServer();
	~cThreadServer();
	cThread *GetThread(const std::string &Name,BOOL UseMessageQueueReactor=false);
};

extern cThreadServer *theThreadServer;