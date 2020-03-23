#include "StdAfx.h"

cLuaObject::cLuaObject(LuaState *State)
{
	mObject.AssignNewTable(State);
}

cLuaObject::cLuaObject(tIntrusivePtr<cLuaObject> Parent, const cLuaObjectKey &Key)
	: mParent(Parent)
	, mKeyInParent(Key)
{
	mObject=Key.GetType()==cLuaObjectKey::eType::Index?Parent->mObject.GetByIndex(Key.GetIndex()):Parent->mObject.GetByName(Key.GetKey().c_str());
	if(!mObject.IsTable())
		mObject.AssignNewTable(mObject.GetState());
}

cLuaObject::cLuaObject(LuaObject Object)
	: mObject(Object)
{
	ASSERT(mObject.IsTable());
}

tIntrusivePtr<cLuaObject> cLuaObject::Create(LuaState *State)
{
	return tIntrusivePtr<cLuaObject>(new cLuaObject(State));
}

tIntrusivePtr<const cLuaObject> cLuaObject::Create(LuaObject Object)
{
	return tIntrusivePtr<const cLuaObject>(new cLuaObject(Object));
}

cLuaObject::~cLuaObject()
{
	if(mParent)
	{
		mKeyInParent.GetType()==cLuaObjectKey::eType::Index?
			mParent->mObject.SetObject(mKeyInParent.GetIndex(), mObject):mParent->mObject.SetObject(mKeyInParent.GetKey().c_str(), mObject);
	}
}

void cLuaObject::SetInt(const cLuaObjectKey &Key, int Value)
{
	if(ASSERTFALSE(Key.GetType()==cLuaObjectKey::eType::None))
		return;
	Key.GetType()==cLuaObjectKey::eType::Index?mObject.SetInteger(Key.GetIndex(), Value):mObject.SetInteger(Key.GetKey().c_str(), Value);
}

int cLuaObject::GetInt(const cLuaObjectKey &Key) const
{
	if(ASSERTFALSE(Key.GetType()==cLuaObjectKey::eType::None))
		return 0;
	LuaObject SubObject=Key.GetType()==cLuaObjectKey::eType::Index?mObject.GetByIndex(Key.GetIndex()):mObject.GetByName(Key.GetKey().c_str());
	if(ASSERTTRUE(SubObject.IsConvertibleToInteger()))
		return SubObject.GetInteger();
	return 0;
}

void cLuaObject::SetBool(const cLuaObjectKey &Key, bool Value)
{
	if(ASSERTFALSE(Key.GetType()==cLuaObjectKey::eType::None))
		return;
	Key.GetType()==cLuaObjectKey::eType::Index?mObject.SetBoolean(Key.GetIndex(), Value):mObject.SetBoolean(Key.GetKey().c_str(), Value);
}

bool cLuaObject::GetBool(const cLuaObjectKey &Key) const
{
	if(ASSERTFALSE(Key.GetType()==cLuaObjectKey::eType::None))
		return false;
	LuaObject SubObject=Key.GetType()==cLuaObjectKey::eType::Index?mObject.GetByIndex(Key.GetIndex()):mObject.GetByName(Key.GetKey().c_str());
	return SubObject.GetBoolean();
}

void cLuaObject::SetString(const cLuaObjectKey &Key, const std::string &Value)
{
	if(ASSERTFALSE(Key.GetType()==cLuaObjectKey::eType::None))
		return;
	Key.GetType()==cLuaObjectKey::eType::Index?mObject.SetString(Key.GetIndex(), Value.c_str()):mObject.SetString(Key.GetKey().c_str(), Value.c_str());
}

std::string cLuaObject::GetString(const cLuaObjectKey &Key) const
{
	if(ASSERTFALSE(Key.GetType()==cLuaObjectKey::eType::None))
		return {};
	LuaObject SubObject=Key.GetType()==cLuaObjectKey::eType::Index?mObject.GetByIndex(Key.GetIndex()):mObject.GetByName(Key.GetKey().c_str());
	if(ASSERTTRUE(SubObject.IsConvertibleToString()))
		return SubObject.GetString();
	return {};
}

tIntrusivePtr<cLuaObject> cLuaObject::GetSubObject(const cLuaObjectKey &Key)
{
	if(Key.GetType()==cLuaObjectKey::eType::None)
		return tIntrusivePtr<cLuaObject>(this);
	return tIntrusivePtr<cLuaObject>(new cLuaObject(tIntrusivePtr<cLuaObject>(this), Key));
}

tIntrusivePtr<const cLuaObject> cLuaObject::GetSubObject(const cLuaObjectKey &Key) const
{
	if(Key.GetType()==cLuaObjectKey::eType::None)
		return tIntrusivePtr<const cLuaObject>(this);
	LuaObject SubObject=Key.GetType()==cLuaObjectKey::eType::Index?mObject.GetByIndex(Key.GetIndex()):mObject.GetByName(Key.GetKey().c_str());
	if(ASSERTFALSE(!SubObject.IsTable()))
		return {};
	return tIntrusivePtr<const cLuaObject>(new cLuaObject(SubObject));
}

int cLuaObject::GetSubObjectCount() const
{
	return (int)mObject.GetTableCount();
}
