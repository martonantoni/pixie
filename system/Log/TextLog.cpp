
void cTextLog::StringLog(const std::string &Text)
{
	std::lock_guard<std::mutex> Guard(mMutex);
	mPlugins.ForEach([Text](auto &Plugin) {Plugin->Log(Text); });
}

void cTextLog::Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType)
{
	ASSERT(CallbackType==eCallbackType::Wait);
	std::lock_guard<std::mutex> Guard(mMutex);
	mPlugins.Unregister(RegisteredID.GetID());
}

cRegisteredID cTextLog::RegisterPlugin(tIntrusivePtr<cTextLogPlugin> Plugin)
{
	std::lock_guard<std::mutex> Guard(mMutex);
	return cRegisteredID { this, mPlugins.Register(std::move(Plugin)) };
}

void cStdOutTextLogPlugin::Log(const std::string& Text)
{
	printf("%s\n", Text.c_str());
}