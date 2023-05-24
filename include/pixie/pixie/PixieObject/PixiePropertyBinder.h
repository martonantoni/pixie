#pragma once

class cPixieObject::cPropertyBinder
{
public:
	virtual ~cPropertyBinder()=default;
};

template<class T> class cPixieObject::tPropertyBinder: public cPixieObject::cPropertyBinder
{
	tSharedValue<T> mSharedValue;
	unsigned int mPropertyFlags;
	cRegisteredID mChangeWatcherID;
	cPixieObject &mObject;
	void VariableChanged();
public:
	tPropertyBinder(cPixieObject &Object, const cResourceLocation &ResourceLocation, unsigned int PropertyFlags);
};

template<class T>
inline cPixieObject::tPropertyBinder<T>::tPropertyBinder(cPixieObject &Object, const cResourceLocation &ResourceLocation, unsigned int PropertyFlags)
	: mObject(Object)
	, mPropertyFlags(PropertyFlags)
{
	mSharedValue.BindTo(ResourceLocation);
	mChangeWatcherID=mSharedValue.GetDispatcher()->RegisterListener([this](auto &) { VariableChanged(); });
	if(mSharedValue.HasValue())
		VariableChanged();
}

template<class T>
inline void cPixieObject::tPropertyBinder<T>::VariableChanged()
{
	mObject.SetProperty(mPropertyFlags, cPropertyValues(mSharedValue));
}

template<>
inline void cPixieObject::tPropertyBinder<std::string>::VariableChanged()
{
	mObject.SetStringProperty(mPropertyFlags, mSharedValue);
}
