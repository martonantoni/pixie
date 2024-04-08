#pragma once

enum class eSafeObjectsStepResult { Continue, Stop };
enum class eSafeObjectsForEachResult { Done, Stopped };

template<typename T> class tSafeObjects
{
	unsigned int mIDCounter=0;
	struct cObjectData
	{
		T mObject;
		int mOrder;
		unsigned int mID;
		template<class U> cObjectData(U &&Object, int Order, unsigned int ID): mObject(std::forward<U>(Object)), mOrder(Order), mID(ID) {}
		cObjectData(const cObjectData &)=default;
		cObjectData(cObjectData &&)=default;
		cObjectData &operator=(const cObjectData &)=default;
		cObjectData &operator=(cObjectData &&)=default;
		bool operator<(const cObjectData &Other) const { return mOrder<Other.mOrder; }
 		bool IsUnregistered() const { return mID==0; }
 		void Unregister() { mID=0; }
	};
	typedef std::vector<cObjectData> cObjects;
	cObjects mObjects;
	static const int InvalidIndex=std::numeric_limits<int>::max();
	mutable int mForEachIndex=InvalidIndex;
public:
	void Unregister(unsigned int ID)
	{
		auto i=std::ranges::find_if(mObjects, [ID](auto &Object) { return Object.mID==ID; });
		if(i==mObjects.end())
			return;
		if(mForEachIndex!=InvalidIndex)
		{
			i->Unregister();
		}
		else
		{
			mObjects.erase(i);
		}
	}

	template<typename U> void ForEach(U FunctionObjectToCall)
	{
		ASSERT(mForEachIndex==InvalidIndex);
		for(mForEachIndex=0; mForEachIndex<mObjects.size(); ++mForEachIndex)
		{
			if(!mObjects[mForEachIndex].IsUnregistered())
				FunctionObjectToCall(mObjects[mForEachIndex].mObject);
		}
		std::erase_if(mObjects, [](auto &Object) {return Object.IsUnregistered(); });
		mForEachIndex=InvalidIndex;
	}
	template<typename U> void ForEach(U FunctionObjectToCall) const
	{
		ASSERT(mForEachIndex==InvalidIndex);
		for(mForEachIndex=0; mForEachIndex<mObjects.size(); ++mForEachIndex)
		{
			if(!mObjects[mForEachIndex].IsUnregistered())
				FunctionObjectToCall(mObjects[mForEachIndex].mObject);
		}
		std::erase_if // const_cast-s are a hack here, should be fixed at some point
			(const_cast<cObjects &>(mObjects), [](auto &Object) {return Object.IsUnregistered(); });
		mForEachIndex=InvalidIndex;
	}

	template<typename U> eSafeObjectsForEachResult ForEach_Stoppable(U FunctionObjectToCall)
	{
		ASSERT(mForEachIndex==InvalidIndex);
		auto Result=eSafeObjectsForEachResult::Done;
		for(mForEachIndex=0; mForEachIndex<mObjects.size(); ++mForEachIndex)
		{
			if(!mObjects[mForEachIndex].IsUnregistered())
			{
				if(FunctionObjectToCall(mObjects[mForEachIndex].mObject)==eSafeObjectsStepResult::Stop)
				{
					Result=eSafeObjectsForEachResult::Stopped;
					break;
				}
			}
		}
		std::erase_if(mObjects, [](auto &Object) {return Object.IsUnregistered(); });
		mForEachIndex=InvalidIndex;
		return Result;
	}

	void clear()
	{
		if(mForEachIndex!=InvalidIndex)
		{
			for(auto &Object: mObjects)
				Object.Unregister();
		}
		else
		{
			mObjects.clear();
		}
	}

	template<typename U> void RemoveIf(U FunctionObjectToCall)
	{
		ForEach([&FunctionObjectToCall, this](auto &Object)
		{
			if(FunctionObjectToCall(Object))
			{
				mObjects[mForEachIndex].Unregister();
			}
		});
		std::erase_if(mObjects, [](auto &Object) {return Object.IsUnregistered(); });
	}

	template<class U>
	unsigned int Register(U &&Object, int Order=100)
	{
		auto i=std::upper_bound(mObjects.begin(), mObjects.end(), Order, [](int Order, const auto &Callable) { return Callable.mOrder>Order; });
		if(mForEachIndex!=InvalidIndex)
		{
			int Index=i-mObjects.begin();
			if(Index<=mForEachIndex)
				++mForEachIndex;
		}
		mObjects.emplace(i, std::forward<U>(Object), Order, ++mIDCounter);
		return mIDCounter;
	}

	bool empty() const 
	{
		return mObjects.empty();
	}

	template<class P> const T *FindIf(P Condition) const
	{
		auto i=std::ranges::find_if(mObjects, [&Condition](auto &ObjectData) { return Condition(ObjectData.mObject); });
		return i==mObjects.end()?nullptr:&i->mObject;
	}
	template<class P> T *FindIf(P Condition)
	{
		return const_cast<T *>(const_cast<const tSafeObjects<T>*>(this)->FindIf(Condition));
	}

	template<class U> void Copy(const tSafeObjects<T> &Source, U CopyFunction)
	{
		mObjects.reserve(Source.mObjects.size());
		for(auto &ObjectData: Source.mObjects)
		{
			mObjects.emplace_back(CopyFunction(ObjectData.mObject), ObjectData.mOrder, ObjectData.mID);
		}
	}
};

template<typename T> class tRegisteredObjects: protected tSafeObjects<T>, public cRegistrationHandler
{
	virtual void Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType) override
	{
		tSafeObjects<T>::Unregister(RegisteredID.GetID());
	}
public:
	using tSafeObjects<T>::ForEach;
	using tSafeObjects<T>::ForEach_Stoppable;
	using tSafeObjects<T>::empty;
	using tSafeObjects<T>::FindIf;
	//Note: objects can be removed only via the returned cRegisteredID (so no such functions as clear, or remove/erase)
	//      this assures that no bug will arise from using CreateWrappedRegisteredID with the returned cRegisteredID

	template<class U>
	cRegisteredID Register(U &&Object, int Order=100)
	{
		return cRegisteredID(this, tSafeObjects<T>::Register(std::forward<U>(Object),Order));
	}
};
