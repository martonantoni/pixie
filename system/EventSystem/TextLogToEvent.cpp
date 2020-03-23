void cTextLogToEvent::PostMessage(const std::string &Text)
{
	mDispatcher->PostEvent(cEvent(mTextHolder.StoreData(Text)));
	Unref();
}

void cTextLogToEvent::Log(const std::string &Text)
{
	Ref();
	::CallBack(theMainThread, eCallbackType::Normal, this, &cTextLogToEvent::PostMessage, Text);
}
