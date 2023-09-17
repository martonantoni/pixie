#pragma once

class cRegisteredID;

class cRegistrationHandler
{
public:
	virtual ~cRegistrationHandler()=default;
	virtual void Unregister(const cRegisteredID &RegisteredID,eCallbackType CallbackType=eCallbackType::Wait)=0;
};

class cRegisteredIDSink;

class cRegisteredID final
{
public:
	using cID = std::variant<void*, uint64_t>;
private:
	friend cRegisteredIDSink;
	cID mID;
	cRegistrationHandler *mRegistrationHandler;
public:

	cRegisteredID(): mRegistrationHandler(nullptr) {}
	cRegisteredID(const cRegisteredID &) = delete;
	cRegisteredID(cRegisteredID &&Source): mID(Source.mID), mRegistrationHandler(Source.mRegistrationHandler)		
	{ 
		Source.mID = nullptr; 
		Source.mRegistrationHandler = nullptr; 
	}
	cRegisteredID &operator=(cRegisteredID &&Source) 
	{
		Unregister();
		mID = Source.mID;
		mRegistrationHandler=Source.mRegistrationHandler; 
		Source.mID =nullptr;
		Source.mRegistrationHandler=nullptr;
		return *this;
	}
	cRegisteredID &operator=(const cRegisteredID &Source) = delete;
	cRegisteredID(cRegistrationHandler *Handler, void *idObject): mRegistrationHandler(Handler), mID(idObject) {}
	cRegisteredID(cRegistrationHandler *Handler, uint64_t id): mRegistrationHandler(Handler), mID(id) {}
	~cRegisteredID() { Unregister(); }
	void Unregister() 
	{ 
		if(mRegistrationHandler) 
		{
			auto RegistrationHandler=mRegistrationHandler;
			mRegistrationHandler=nullptr;
			RegistrationHandler->Unregister(*this);
		}
	}
	void *GetIDData() const { return std::get<void*>(mID); }
	uint64_t GetID() const { return std::get<uint64_t>(mID); }
	bool IsValid() const { return mRegistrationHandler!=nullptr; }
	cRegistrationHandler *GetRegistrationHandler() const { return mRegistrationHandler; }

	cID& AccessID() { return mID; } // very very dangerous construct, use it with extreme care!
};

class cRegisteredIDSink final
{
public:
	cRegisteredIDSink() {}
	cRegisteredIDSink(const cRegisteredIDSink &) = delete;
	cRegisteredIDSink(cRegisteredID &&Source) 
	{
		Source.mRegistrationHandler=nullptr;
	}
	void operator=(cRegisteredID &&Source)
	{
		Source.mRegistrationHandler=nullptr;
	}
};

extern cRegisteredIDSink theIDSink;

typedef std::vector<cRegisteredID> cRegisteredIDList;

inline cRegisteredID CreateWrappedRegisteredID(cRegisteredID ID, const std::function<void()> &FunctionToCallOnUnregister)
{
	struct cWrapper: public cRegistrationHandler
	{
		cRegisteredID mOriginalID;
		std::function<void()> mFunctionToCallOnUnregister;
		virtual void Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType) override
		{
			mFunctionToCallOnUnregister();
			delete this;
		}
		cWrapper(cRegisteredID &&ID, const std::function<void()> &FunctionToCallOnUnregister):
			mOriginalID(std::move(ID)), mFunctionToCallOnUnregister(FunctionToCallOnUnregister) {}
	};
	auto Wrapper=new cWrapper(std::move(ID), FunctionToCallOnUnregister);
	return cRegisteredID(Wrapper, 0ull);
}
