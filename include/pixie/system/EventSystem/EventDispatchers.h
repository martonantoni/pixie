#pragma once

class cEventDispatchers
{
public:
	typedef std::vector<std::string> cEventNames;
	struct cDispatcherRangeInfo
	{
		size_t mFirstIndex;
		cEventNames mEventNames;
		cDispatcherRangeInfo(size_t FirstIndex, const cEventNames &EventNames): mFirstIndex(FirstIndex), mEventNames(EventNames) {}
		cDispatcherRangeInfo(size_t FirstIndex, cEventNames &&EventNames): mFirstIndex(FirstIndex), mEventNames(std::move(EventNames)) {}
		cDispatcherRangeInfo(cDispatcherRangeInfo &&)=default;
		cDispatcherRangeInfo &operator=(cDispatcherRangeInfo &&)=default;
		cDispatcherRangeInfo(const cDispatcherRangeInfo &)=delete;
		cDispatcherRangeInfo &operator=(const cDispatcherRangeInfo &)=delete;
	};
private:
	tIntrusivePtr<cEventDispatcher> mRootDispatcher;
	typedef std::vector<tIntrusivePtr<cEventDispatcher>> cDispatchers;
	struct cDispatcherRange
	{
		const cDispatcherRangeInfo &mInfo;
		cDispatchers mDispatchers;
		cDispatcherRange(const cDispatcherRangeInfo &Info): mInfo(Info) { mDispatchers.resize(mInfo.mEventNames.size());  }
	};
	typedef std::vector<cDispatcherRange> cRanges;
	cRanges mRanges;
public:
	cEventDispatchers()=default;
	cEventDispatchers(const cEventDispatchers &)=delete;
	cEventDispatchers(cEventDispatchers &&)=delete;
	cEventDispatchers &operator=(const cEventDispatchers &)=delete;
	cEventDispatchers &operator=(cEventDispatchers &&)=delete;

	void Init(const tIntrusivePtr<cEventDispatcher> &RootDispatcher);
	void Init(const cResourceLocation &Location);
	void AddEvents(const cDispatcherRangeInfo &RangeInfo);
	void addEvents(const cEventNames& eventNames);

	void PostEvent(size_t DispatcherIndex, cEvent &&Event=cEvent());
	tIntrusivePtr<cEventDispatcher> operator[](size_t Index) const;
};