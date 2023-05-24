#pragma once

class cLuaObjectKey final
{
public:
	enum class eType { Key, Index, None };
private:
	eType mType;
	std::string mKey;
	int mIndex;
public:
	cLuaObjectKey(): mType(eType::None) {}
	cLuaObjectKey(int Index): mType(eType::Index), mIndex(Index) { ASSERT(Index>=1);  }
	cLuaObjectKey(const std::string &Key): mType(eType::Key), mKey(Key) {}
	eType GetType() const { return mType; }
	const std::string &GetKey() const { return mKey; }
	int GetIndex() const { return mIndex; }
};

class cLuaObject final: public cIntrusiveRefCount
{
	LuaObject mObject;
	tIntrusivePtr<cLuaObject> mParent;
	cLuaObjectKey mKeyInParent;

	cLuaObject(LuaState *State);
	cLuaObject(LuaObject Object);
	cLuaObject(tIntrusivePtr<cLuaObject> Parent, const cLuaObjectKey &Key);
public:
	virtual ~cLuaObject();
	void SetInt(const cLuaObjectKey &Key, int Value);
	int GetInt(const cLuaObjectKey &Key) const;
	void SetBool(const cLuaObjectKey &Key, bool Value);
	bool GetBool(const cLuaObjectKey &Key) const;
	void SetString(const cLuaObjectKey &Key, const std::string &Value);
	std::string GetString(const cLuaObjectKey &Key) const;
	tIntrusivePtr<cLuaObject> GetSubObject(const cLuaObjectKey &Key);
	tIntrusivePtr<const cLuaObject> GetSubObject(const cLuaObjectKey &Key) const;
	int GetSubObjectCount() const;
	LuaObject GetObject() const { return mObject; }

	template<class T> void Synch(const cLuaObjectKey &Key, T &Value) const;
	template<class T> void Synch(const cLuaObjectKey &Key, const T &Value);

	static tIntrusivePtr<cLuaObject> Create(LuaState *State);
	static tIntrusivePtr<const cLuaObject> Create(LuaObject Object);
	template<class T>
	static LuaObject IntoLua(LuaState *State, const T &ObjectToStore)
	{
		auto Object=Create(State);
		ObjectToStore.SynchWithLua(*Object);
		return Object->GetObject();
	}
	template<class T>
	static void UpdateFromLua(LuaObject Object, T &ObjectToUpdate)
	{
		auto HelperObject=Create(Object);
		ObjectToUpdate.SynchWithLua(*HelperObject);
	}
};

template<class T> struct tSynchWithLuaObjectHelper
{
	//  Enum-s

	template<typename U=T>
	static void Synch(const cLuaObjectKey &Key, typename std::enable_if_t<std::is_enum<U>::value, T> &Value, const cLuaObject &Object)
	{
		Value=(T)Object.GetInt(Key);
	}
	template<typename U=T>
	static void Synch(const cLuaObjectKey &Key, const typename std::enable_if_t<std::is_enum<U>::value, T> &Value, cLuaObject &Object)
	{
		Object.SetInt(Key, (int)Value);
	}

	// Integral types, but not bool

	template<typename U=T>
	static void Synch(const cLuaObjectKey &Key, typename std::enable_if_t<std::is_integral<U>::value&&!std::is_same<U, bool>::value, T> &Value, const cLuaObject &Object)
	{
		Value=Object.GetInt(Key);
	}
	template<typename U=T>
	static void Synch(const cLuaObjectKey &Key, const typename std::enable_if_t<std::is_integral<U>::value&&!std::is_same<U, bool>::value, T> &Value, cLuaObject &Object)
	{
		Object.SetInt(Key, (int)Value);
	}

	// bool

	template<typename U=T>
	static void Synch(const cLuaObjectKey &Key, typename std::enable_if_t<std::is_same<U, bool>::value, T> &Value, const cLuaObject &Object)
	{
		Value=Object.GetBool(Key);
	}
	template<typename U=T>
	static void Synch(const cLuaObjectKey &Key, const typename std::enable_if_t<std::is_same<U, bool>::value, T> &Value, cLuaObject &Object)
	{
		Object.SetBool(Key, Value);
	}

	// (class/struct) objects, but not containers. They need to have SynchWithConfig functions

	template<typename U=T>
	static void Synch(const cLuaObjectKey &Key, typename std::enable_if_t<std::is_class<U>::value&&!is_container<U>::value, T> &Value, const cLuaObject &Object)
	{
		Value.SynchWithLua(*Object.GetSubObject(Key));
	}
	template<typename U=T>
	static void Synch(const cLuaObjectKey &Key, const typename std::enable_if_t<std::is_class<U>::value&&!is_container<U>::value, T> &Value, cLuaObject &Object)
	{
		Value.SynchWithLua(*Object.GetSubObject(Key));
	}

	// containers

	template<typename U=T>
	static void Synch(const cLuaObjectKey &Key, typename std::enable_if_t<is_container<U>::value&&!std::is_same<U, std::string>::value, T> &Values, const cLuaObject &Object)
	{
		Values.clear();
		tIntrusivePtr<const cLuaObject> MainSubObject=Object.GetSubObject(Key);
		int NumberOfSubSubObjects=MainSubObject->GetSubObjectCount();
		for(int i=1; i<=NumberOfSubSubObjects; ++i)
		{
			U::value_type v;
			MainSubObject->Synch(i, v);
			Values.emplace_back(v);
		}
	}
	template<typename U=T>
	static void Synch(const cLuaObjectKey &Key, const typename std::enable_if_t<is_container<U>::value&&!std::is_same<U, std::string>::value, T> &Values, cLuaObject &Object)
	{
		auto MainSubObject=Object.GetSubObject(Key);
		int Index=1;
		for(auto &Value: Values)
		{
			MainSubObject->Synch(Index, Value);
			++Index;
		}
	}

	static void Synch(const cLuaObjectKey &Key, std::string &Value, const cLuaObject &Object)
	{
		Value=Object.GetString(Key);
	}
	static void Synch(const cLuaObjectKey &Key, const std::string &Value, cLuaObject &Object)
	{
		Object.SetString(Key, Value);
	}
};

template<class T> inline void cLuaObject::Synch(const cLuaObjectKey &Key, T &Value) const
{
	tSynchWithLuaObjectHelper<T>::Synch(Key, Value, *this);
}

template<class T> inline void cLuaObject::Synch(const cLuaObjectKey &Key, const T &Value)
{
	tSynchWithLuaObjectHelper<T>::Synch(Key, Value, *this);
}



