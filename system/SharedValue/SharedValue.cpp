#include "StdAfx.h"

cLocatableValue cLocatableValue::mRootValue;

tIntrusivePtr<cLocatableValue> cLocatableValue::GetGlobalValue(const cResourceLocation &Location)
{
	return mRootValue.GetSubResource(Location, CanCreate);
}

