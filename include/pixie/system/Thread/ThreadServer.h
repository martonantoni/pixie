#pragma once

class cThreadServer final
{
public:
	enum class eReactorType
	{
		Normal, MessagePump
	};
private:
	using cThreadMap = std::unordered_map<std::string, std::unique_ptr<cThread>>;
	cThreadMap mThreadMap;
	std::mutex mThreadMapMutex;
	static std::unique_ptr<cReactor> createReactor(eReactorType reactorType);
public:
	cThreadServer();
	~cThreadServer();
	cThread* GetThread(const std::string &name, eReactorType reactorType = eReactorType::Normal);
	void createMainThread(eReactorType reactorType = eReactorType::MessagePump);
};

extern cThreadServer *theThreadServer;