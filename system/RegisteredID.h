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
	using cID=unsigned int;
private:
	friend cRegisteredIDSink;
	union
	{
		void *mIDData;     // this will be scrapped in the future
		cID mID;  // and only this will remain
	};
	cRegistrationHandler *mRegistrationHandler;
public:

	cRegisteredID(): mRegistrationHandler(nullptr), mIDData(nullptr) {}
	cRegisteredID(const cRegisteredID &) = delete;
	cRegisteredID(cRegisteredID &&Source): mIDData(Source.mIDData), mRegistrationHandler(Source.mRegistrationHandler) 
		{ Source.mIDData=nullptr; Source.mRegistrationHandler=nullptr; }
	cRegisteredID &operator=(cRegisteredID &&Source) 
	{
		Unregister();
		mIDData=Source.mIDData; mRegistrationHandler=Source.mRegistrationHandler; 
		Source.mIDData=nullptr; Source.mRegistrationHandler=nullptr;
		return *this;
	}
	cRegisteredID &operator=(const cRegisteredID &Source) = delete;
	cRegisteredID(cRegistrationHandler *Handler,void *IDData): mRegistrationHandler(Handler), mIDData(IDData) {}
	cRegisteredID(cRegistrationHandler *Handler, cID ID): mRegistrationHandler(Handler), mID(ID) {}
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
	void *GetIDData() const { return mIDData; }
	auto GetID() const { return mID; }
	bool IsValid() const { return mRegistrationHandler!=nullptr; }
	cRegistrationHandler *GetRegistrationHandler() const { return mRegistrationHandler; }

	unsigned int &AccessID() { return mID; } // very very dangerous construct, use it with extreme care!
};

class cRegisteredIDSink final
{
public:
	cRegisteredIDSink() {}
	cRegisteredIDSink(const cRegisteredIDSink &) = delete;
	cRegisteredIDSink(cRegisteredID &&Source) 
	{
		Source.mIDData=nullptr; Source.mRegistrationHandler=nullptr;
	}
	void operator=(cRegisteredID &&Source)
	{
		Source.mIDData=nullptr; Source.mRegistrationHandler=nullptr;
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
	return cRegisteredID(Wrapper, 0u);
}
