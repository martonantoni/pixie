#pragma once

class cLocatableValue: public cSimpleValue, public tLocatableResourceBase<cLocatableValue>
{
	static cLocatableValue mRootValue;
public:
	cLocatableValue() {}
	cLocatableValue(const cLocatableValue &) = delete;
	cLocatableValue &operator=(const cLocatableValue &) = delete;
	cLocatableValue &operator=(cSimpleValue &&);
	static tIntrusivePtr<cLocatableValue> GetGlobalValue(const cResourceLocation &Location);
};

template<class T>
class tSharedValue
{
	cSimpleValue mUnboundValue;
	tIntrusivePtr<cLocatableValue> mValue;
public:
	tSharedValue() {}
	tSharedValue(const T &Value) { mUnboundValue.Set(Value); }
	tSharedValue(tSharedValue<T> &&Value)=default;
	enum class eStrongerValue { This, Located }; // if one of the two values is of type None, the other one will be the stronger!
	void BindTo(const cResourceLocation &Location, eStrongerValue StrongerValue=eStrongerValue::Located);
	void BindTo(const cResourceLocation &Location, const cResourceLocation &SubLocation, eStrongerValue StrongerValue=eStrongerValue::Located);
	operator T() const;
	T GetValue() const { return (T)*this; }
	T operator=(const T &Value);
	tSharedValue<T> &operator=(const tSharedValue<T> &Other);
	tSharedValue<T> &operator=(tSharedValue<T> &&Other) { return *this=Other; }
	template<class U> bool operator==(const U &Other) const { return (T)(*this)==Other; }
	template<class U> bool operator!=(const U &Other) const { return (T)(*this)!=Other; }
	tIntrusivePtr<cEventDispatcher> GetDispatcher() const;
	bool HasValue() const { return mValue&&mValue->GetType()!=cSimpleValue::eType::None; }
	//	const char *c_str() const;
};

typedef tSharedValue<int> cSharedInt;
typedef tSharedValue<std::string> cSharedString;
typedef tSharedValue<double> cSharedDouble;
typedef tSharedValue<bool> cSharedBool;

template<class T>
inline void SetGlobalValue(const cResourceLocation &Location, const T &Value)
{
	cLocatableValue::GetGlobalValue(Location)->Set(Value);
}

template<class T>
inline void tSharedValue<T>::BindTo(const cResourceLocation &Location, eStrongerValue StrongerValue)
{
	if(Location.empty())
		return;
	auto NewValue=cLocatableValue::GetGlobalValue(Location);
	if(NewValue==mValue)
		return;
	mValue=NewValue;
	mValue->Merge(mUnboundValue, StrongerValue==eStrongerValue::Located?
		cSimpleValue::eStrongerValue::This:cSimpleValue::eStrongerValue::Merged);
}

template<class T>
inline void tSharedValue<T>::BindTo(const cResourceLocation &Location, const cResourceLocation &SubLocation, eStrongerValue StrongerValue)
{
	if(Location.empty())
		return;
	mValue=cLocatableValue::GetGlobalValue(Location)->GetSubResource(SubLocation, cLocatableValue::eNodeCreation::CanCreate);
	mValue->Merge(mUnboundValue, StrongerValue==eStrongerValue::Located?
		cSimpleValue::eStrongerValue::This:cSimpleValue::eStrongerValue::Merged);
}

template<class T>
inline T tSharedValue<T>::operator=(const T &Value)
{
	if(mValue)
		mValue->Set(Value);
	else
		mUnboundValue.Set(Value);
	return Value;
}

template<class T>
inline tSharedValue<T> &tSharedValue<T>::operator=(const tSharedValue<T> &Other)
{
	if(this==&Other)
		return;
	*this=Other.GetValue();
	return *this;
}

template<class T>
tIntrusivePtr<cEventDispatcher> tSharedValue<T>::GetDispatcher() const
{
	return mValue?mValue->GetDispatcher():mUnboundValue.GetDispatcher();
}

template<> inline tSharedValue<int>::operator int() const
{
	return (mValue?*mValue:mUnboundValue).GetIntValue();
}

template<> inline tSharedValue<bool>::operator bool() const
{
	return (mValue?*mValue:mUnboundValue).GetBoolValue();
}

template<> inline tSharedValue<std::string>::operator std::string() const
{
	return (mValue?*mValue:mUnboundValue).GetStringValue();
}

template<> inline tSharedValue<double>::operator double() const
{
	return (mValue?*mValue:mUnboundValue).GetDoubleValue();
}

