#pragma once

class cTextLogPlugin: public cIntrusiveThreadsafeRefCount
{
public:
	virtual void Log(const std::string &Text)=0;
};

class cTextLog: public cRegistrationHandler
{
	mutable std::mutex mMutex;
	tSafeObjects<tIntrusivePtr<cTextLogPlugin>> mPlugins;
public:
	template<class... Ts> void Log(const char *Format, Ts &&...Args)
	{
		StringLog(fmt::sprintf(Format, std::forward<Ts>(Args)...));
	}
	void StringLog(const std::string &Text);
	cRegisteredID RegisterPlugin(tIntrusivePtr<cTextLogPlugin> Plugin);
	virtual void Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType=eCallbackType::Wait) override;
};
