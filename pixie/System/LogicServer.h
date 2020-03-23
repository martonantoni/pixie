#pragma once

class cLogicServer: public cRegistrationHandler
{
public:
	typedef std::function<void()> cLogicListener;
private:
	struct cLogicUserData
	{
		cLogicListener mListener;
		unsigned int mLastSeenFrameTime=0;
		static unsigned int mIDCounter;
		unsigned int mID=++mIDCounter;
		int mOrder;
		cLogicUserData(const cLogicListener &Listener, int Order): mListener(Listener), mOrder(Order) {}
	};
	typedef std::vector<cLogicUserData> cLogicUsers;
	cLogicUsers mLogicUsers;
	std::vector<cLogicListener> mExecuteOnceReading, mExecuteOnceWriting;
	bool mLogicUsersModifiedTrap=false;
public:
	cLogicServer();
	~cLogicServer();
	void Tick();
	virtual void Unregister(const cRegisteredID &RegisteredID,eCallbackType CallbackType=eCallbackType::Wait) override;

	enum { DefaultLogicOrder = 100 };
	cRegisteredID AddLogic(const cLogicListener &Listener,int Order);

	template<class T> void ExecuteOnce(T &&Listener) { mExecuteOnceWriting.emplace_back(std::forward<T>(Listener)); }
};

extern cLogicServer theLogicServer;
extern unsigned int gFrameTime;