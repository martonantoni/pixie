#include "StdAfx.h"

cSimpleValue::cSimpleValue(const std::string &Value)
	: mStringValue(Value)
	, mType(eType::String)
	//	, mUpdateStamp(++mUpdateStampCounter)
{
}

cSimpleValue::cSimpleValue(int Value)
	: mIntValue(Value)
	, mType(eType::Int)
	//	, mUpdateStamp(++mUpdateStampCounter)
{
}

cSimpleValue::cSimpleValue(double Value)
	: mDoubleValue(Value)
	, mType(eType::Double)
	//	, mUpdateStamp(++mUpdateStampCounter)
{
}

tIntrusivePtr<cEventDispatcher> cSimpleValue::GetDispatcher() const
{
	if(!mDispatcher)
	{
		// init-on-demand pattern
		const_cast<cSimpleValue &>(*this).mDispatcher=make_intrusive_ptr<cEventDispatcher>(cEventDispatcher::eCoalescing::Coalescing);
	}
	return mDispatcher;
}

const char *cSimpleValue::GetTypeString(eType Type)
{
	switch(Type)
	{
	case eType::String: return "String";
	case eType::Double: return "Double";
	case eType::Int: return "Int";
	case eType::Bool: return "Bool";
	case eType::None: return "None";
	}
	return "N/A";
}

std::string cSimpleValue::GetDebugString() const
{
	switch(mType)
	{
	case eType::String:
		return fmt::sprintf("String (%s)", mStringValue.Get());
	case eType::Double:
		return fmt::sprintf("Double (%.5f)", mDoubleValue.Get());
	case eType::Int:
		return fmt::sprintf("Int (%d)", mIntValue.Get());
	case eType::Bool:
		return fmt::sprintf("Bool (%s)", mIntValue.Get()?"true":"false");
	}
	return "Unknown";
}

int cSimpleValue::GetIntValue() const
{
	if(mIntValue.IsValid())
		return mIntValue;
	switch(mType)
	{
	case eType::String:
		mIntValue=mStringValue.Get()=="true"?1:atoi(mStringValue.Get().c_str());
		break;
	case eType::Int:
		ASSERT(false);
		break;
	case eType::Double:
		mIntValue=mDoubleValue.Get();
		break;
	case eType::Bool:
		ASSERT(false);
		break;
	case eType::None:
		ASSERT(false);
		break;
	}
	return mIntValue;
}

double cSimpleValue::GetDoubleValue() const
{
	if(mDoubleValue.IsValid())
		return mDoubleValue;
	switch(mType)
	{
	case eType::String:
		mDoubleValue=atof(mStringValue.Get().c_str());
		break;
	case eType::Int:
		mDoubleValue=mIntValue.Get();
		break;
	case eType::Double:
		ASSERT(false);
		break;
	case eType::Bool:
		mDoubleValue=mIntValue.Get();
		break;
	case eType::None:
		ASSERT(false);
		break;
	}
	return mDoubleValue;
}

std::string cSimpleValue::GetStringValue() const
{
	if(mStringValue.IsValid())
		return mStringValue;
	switch(mType)
	{
	case eType::String:
		ASSERT(false);
		break;
	case eType::Int:
		mStringValue=std::to_string(mIntValue.Get());
		break;
	case eType::Double:
		mStringValue=std::to_string(mDoubleValue.Get());
		break;
	case eType::Bool:
		mStringValue=mIntValue.Get()?"true":"false";
		break;
	case eType::None:
		ASSERT(false);
		break;
	}
	return mStringValue;
}

bool cSimpleValue::GetBoolValue() const
{
	return GetIntValue();
}

void cSimpleValue::Set(int IntValue)
{
	mType=eType::Int;
	mIntValue=IntValue;
	mDoubleValue.Invalidate();
	mStringValue.Invalidate();
	//	mUpdateStamp=++mUpdateStampCounter;
	if(mDispatcher)
		mDispatcher->PostEvent();
}

void cSimpleValue::Set(double DoubleValue)
{
	mType=eType::Double;
	mDoubleValue=DoubleValue;
	mIntValue.Invalidate();
	mStringValue.Invalidate();
	//	mUpdateStamp=++mUpdateStampCounter;
	if(mDispatcher)
		mDispatcher->PostEvent();
}

void cSimpleValue::Set(const std::string &StringValue)
{
	mType=eType::String;
	mStringValue=StringValue;
	mIntValue.Invalidate();
	mDoubleValue.Invalidate();
	//	mUpdateStamp=++mUpdateStampCounter;
	if(mDispatcher)
		mDispatcher->PostEvent();
}

void cSimpleValue::Set(bool BoolValue)
{
	mType=eType::Bool;
	mIntValue=BoolValue?1:0;
	mStringValue=BoolValue?"true"s:"false"s;
	mDoubleValue.Invalidate();
	//	mUpdateStamp=++mUpdateStampCounter;
	if(mDispatcher)
		mDispatcher->PostEvent();
}

void cSimpleValue::Merge(cSimpleValue &MergedValue, eStrongerValue StrongerValue)
{
	// First, check the dispatchers.
	if(MergedValue.mDispatcher&&MergedValue.mDispatcher->GetRefCount()>1) // true: has dispatcher, and that dispatcher is known to the outside world
	{
		if(!mDispatcher||mDispatcher->GetRefCount()==1)
		{
			// if noone outside has reference to it, then we can treat it as if did not exist.
			mDispatcher=std::move(MergedValue.mDispatcher);
		}
		else
		{
			// both has dispatchers that are known to the outside world. this is a tricky situation
			MergedValue.mDispatcher->BridgeEventsFrom(*mDispatcher);
		}
	}
	else
	{
		MergedValue.mDispatcher.reset();
	}
	// and now deal with the values
	if(MergedValue.mType!=eType::None&&(StrongerValue==eStrongerValue::Merged||mType==eType::None))
	{
		mType=MergedValue.mType;
		mIntValue=std::move(MergedValue.mIntValue);
		mDoubleValue=std::move(MergedValue.mDoubleValue);
		mStringValue=std::move(MergedValue.mStringValue);
	}
	if(mType!=eType::None&&mDispatcher)
		mDispatcher->PostEvent();
}
