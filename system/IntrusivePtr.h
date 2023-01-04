#pragma once

class cIntrusiveRefCount
{
	mutable unsigned int mReferenceCounter=0;
protected:
	virtual ~cIntrusiveRefCount() {}
	virtual void ReferenceCounterReachedZero() const { delete this; }
public:
	template<class T> tIntrusivePtr<T> share(T* object);
	void Ref() const 
	{ 
		++mReferenceCounter; 
	}
	void Unref() const
	{ 
		if(!--mReferenceCounter) 
			ReferenceCounterReachedZero(); 
	}
	auto GetRefCount() const { return mReferenceCounter; }
};

class cThread;

class cIntrusiveThreadsafeRefCount
{
	mutable std::atomic<int> mReferenceCounter=0;
	cThread *mDestructorThread=nullptr;
	void DestroySelf() const;
	virtual void ReferenceCounterReachedZero() const;
protected:
	void PostDestroySelf(cThread *Thread) const;
public:
	virtual ~cIntrusiveThreadsafeRefCount() {}
	void SetDestructorThread(cThread *DestructorThread) 
	{ 
		mDestructorThread=DestructorThread; 
	}
	void Ref() const 
	{ 
		++mReferenceCounter;
	}
	void Unref() const 
	{ 
		if(!--mReferenceCounter) 
			ReferenceCounterReachedZero(); 
	}
	template<class T> void GetRefCount() const { Not_Supported_For_Threadsafe_Version; }
};

template<class T> class tIntrusivePtr
{
	T *mObject=nullptr;
public:
	template<class> friend class tIntrusivePtr;
	tIntrusivePtr() {}
	tIntrusivePtr(std::nullptr_t) {}
	explicit tIntrusivePtr(T *Object): mObject(Object) { if(Object) Object->Ref(); }

	                  tIntrusivePtr(const tIntrusivePtr<T> &Src): mObject(Src.mObject) { if(mObject) mObject->Ref(); }
	template<class U> tIntrusivePtr(const tIntrusivePtr<U> &Src): mObject(Src.mObject) { if(mObject) mObject->Ref(); }
	                  tIntrusivePtr(tIntrusivePtr<T> &&Src):      mObject(Src.mObject) { Src.mObject=nullptr; }
	template<class U> tIntrusivePtr(tIntrusivePtr<U> &&Src):      mObject(Src.mObject) { Src.mObject=nullptr; }

	~tIntrusivePtr() { if(mObject) mObject->Unref(); }
	static tIntrusivePtr<T> CreateWithoutAddingRef(T *Object)
	{
		tIntrusivePtr<T> Ptr;
		Ptr.mObject=Object;
		return Ptr;
	}
	template<class U>
	void reset(U *Object)
	{
		if(Object==mObject)
			return;
		if(mObject)
			mObject->Unref();
		mObject=Object;
		if(mObject)
			mObject->Ref();
	}
	void reset()
	{
		if(mObject)
			mObject->Unref();
		mObject=nullptr;
	}
	T *get() { return mObject; }
	T *operator->() { return mObject; }
	T &operator*() { return *mObject; }
	const T *get() const { return mObject; }
	const T *operator->() const { return mObject; }
	const T &operator*() const { return *mObject; }
	template<class U>
	tIntrusivePtr<T> &operator=(const tIntrusivePtr<U> &Src)
	{
		reset(Src.mObject);
		return *this;
	}
	tIntrusivePtr<T> &operator=(const tIntrusivePtr<T> &Src)
	{
		reset(Src.mObject);
		return *this;
	}
	tIntrusivePtr<T> &operator=(tIntrusivePtr<T> &&Src)
	{
		if(mObject==Src.mObject)
			return *this;
		if(mObject)
			mObject->Unref();
		mObject=Src.mObject;
		Src.mObject=nullptr;
		return *this;
	}
	template<class U>
	tIntrusivePtr<T> &operator=(tIntrusivePtr<U> &&Src)
	{
		if(mObject==Src.mObject)
			return *this;
		if(mObject)
			mObject->Unref();
		mObject=Src.mObject;
		Src.mObject=nullptr;
		return *this;
	}
	bool operator!() const { return mObject==nullptr; }
	operator bool() const { return mObject!=nullptr; }
};

template<class T, class ...Ps>
tIntrusivePtr<T> make_intrusive_ptr(Ps &&...ps)
{
	return tIntrusivePtr<T>(new T(std::forward<Ps>(ps)...));
}

template<class T> tIntrusivePtr<T> cIntrusiveRefCount::share(T* object)
{
	static_assert(std::is_base_of<cIntrusiveRefCount, T>::value, "shared object must be subclass of cIntrusiveRefCount");
	return tIntrusivePtr<T>(object);
}
