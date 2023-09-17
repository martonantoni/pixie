#pragma once

template<class T, class Dummy=void> class tDataHolder;

template<class cDataType>
class tDataHolder<cDataType, typename std::enable_if<!(std::is_pod<cDataType>::value&&(sizeof(cDataType)<=sizeof(cRegisteredID::cID)))>::type>
	: public cRegistrationHandler
	, public tSingleton<tDataHolder<cDataType, typename std::enable_if<!(std::is_pod<cDataType>::value&&(sizeof(cDataType)<=sizeof(cRegisteredID::cID)))>::type>>
		// warning: not thread safe!
{       // (by design)
	typedef std::pair<unsigned int, cDataType> cDataAndID;
	typedef std::vector<cDataAndID> cStoredData;
	cStoredData mStoredData;
	unsigned int mDataIDCounter=0;
public:
	cRegisteredID StoreData(cDataType &&Data);
	cRegisteredID StoreData(const cDataType &Data);
	const cDataType *GetData(const cRegisteredID &RegisteredID) const; // returns nullptr if item is not found / id is invalid (also ASSERTs)
	void UpdateData(const cRegisteredID &RegisteredID, const cDataType &Data);
	virtual void Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType=eCallbackType::Wait) override;
};

template<class cDataType>
cRegisteredID tDataHolder<cDataType, typename std::enable_if<!(std::is_pod<cDataType>::value&&(sizeof(cDataType)<=sizeof(cRegisteredID::cID)))>::type>::
	StoreData(cDataType &&Data)
{
	mStoredData.push_back(std::make_pair(++mDataIDCounter, std::move(Data)));
	return cRegisteredID(this, mDataIDCounter);
}

template<class cDataType>
cRegisteredID tDataHolder<cDataType, typename std::enable_if<!(std::is_pod<cDataType>::value&&(sizeof(cDataType)<=sizeof(cRegisteredID::cID)))>::type>::
	StoreData(const cDataType &Data)
{
	mStoredData.push_back(std::make_pair(++mDataIDCounter, Data));
	return cRegisteredID(this, mDataIDCounter);
}

template<class cDataType>
const cDataType *tDataHolder<cDataType, typename std::enable_if<!(std::is_pod<cDataType>::value&&(sizeof(cDataType)<=sizeof(cRegisteredID::cID)))>::type>::
	GetData(const cRegisteredID &RegisteredID) const
{
	if(ASSERTFALSE(RegisteredID.GetRegistrationHandler()!=this))
		return nullptr;
	auto i=std::find_if(mStoredData, [ID=RegisteredID.GetID()](const auto &DataAndID) { return DataAndID.first==ID; });
	return ASSERTFALSE(i==mStoredData.end())?nullptr:&i->second;
}

template<class cDataType>
void tDataHolder<cDataType, typename std::enable_if<!(std::is_pod<cDataType>::value&&(sizeof(cDataType)<=sizeof(cRegisteredID::cID)))>::type>::
	Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType)
{
	auto i=std::find_if(mStoredData, [ID=RegisteredID.GetID()](const auto &DataAndID) { return DataAndID.first==ID; });
	if(ASSERTTRUE(i!=mStoredData.end()))
		mStoredData.erase(i);
}

template<class cDataType>
void tDataHolder<cDataType, typename std::enable_if<!(std::is_pod<cDataType>::value&&(sizeof(cDataType)<=sizeof(cRegisteredID::cID)))>::type>::
	UpdateData(const cRegisteredID &RegisteredID, const cDataType &Data)
{
	auto i=std::find_if(mStoredData, [ID=RegisteredID.GetID()](const auto &DataAndID) { return DataAndID.first==ID; });
	if(ASSERTTRUE(i!=mStoredData.end()))
		i->second=Data;
}

///-------------------------------------------------------------------------------------------------------------------------------------

template<class cDataType>
class tDataHolder<cDataType, typename std::enable_if<std::is_pod<cDataType>::value&&(sizeof(cDataType)<=sizeof(cRegisteredID::cID))>::type>
	: public cRegistrationHandler, public tSingleton<tDataHolder<cDataType, typename std::enable_if<std::is_pod<cDataType>::value&&(sizeof(cDataType)<=sizeof(cRegisteredID::cID))>::type>>
	// warning: not thread safe!
{   // (by design)
public:
	cRegisteredID StoreData(cDataType &&Data)
	{
		return cRegisteredID(this, (cRegisteredID::cID)Data);
	}
	cRegisteredID StoreData(const cDataType &Data)
	{
		return cRegisteredID(this, (cRegisteredID::cID)Data);
	}
	const cDataType *GetData(const cRegisteredID &RegisteredID) const
	{
		return (const cDataType *)&const_cast<cRegisteredID &>(RegisteredID).AccessID();
	}
	void UpdateData(const cRegisteredID &RegisteredID, const cDataType &Data)
	{
		ASSERT(false);
		const_cast<cRegisteredID &>(RegisteredID).AccessID()=(uint64_t &)Data;
	}
	virtual void Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType=eCallbackType::Wait) override
	{
	}
};
