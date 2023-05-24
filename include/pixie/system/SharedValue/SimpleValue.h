#pragma once

class cEventDispatcher;

class cSimpleValue
{
public:
	enum class eType { None, Int, Double, Bool, String };
private:
	eType mType=eType::None;
	template<class T> class tCached
	{
		T mValue;
		bool mIsValid=false;
	public:
		tCached()=default;
		tCached(T Value): mValue(std::move(Value)), mIsValid(true) {}
		tCached &operator=(const T &Value)
		{
			mValue=Value;
			mIsValid=true;
			return *this;
		}
		void Invalidate() { mIsValid=false; }
		bool IsValid() const { return mIsValid; }
		operator T() const { return mValue; }
		T Get() const { return mValue; }
	};
	mutable tCached<int> mIntValue;
	mutable tCached<double> mDoubleValue;
	mutable tCached<std::string> mStringValue;
	// 	uint64_t mUpdateStamp=0;
	// 	static uint64_t mUpdateStampCounter;

	tIntrusivePtr<cEventDispatcher> mDispatcher;
public:
	cSimpleValue()=default;
	cSimpleValue(const std::string &Value);
	cSimpleValue(int Value);
	cSimpleValue(double Value);
	cSimpleValue(const cSimpleValue &Source) = delete;
	cSimpleValue(cSimpleValue &&)=default;
	cSimpleValue &operator=(const cSimpleValue &Value) = delete;
	cSimpleValue &operator=(cSimpleValue &&)=default;
	eType GetType() const { return mType; }
	static const char *GetTypeString(eType pType);
	tIntrusivePtr<cEventDispatcher> GetDispatcher() const;

	int GetIntValue() const;
	double GetDoubleValue() const;
	std::string GetStringValue() const;
	bool GetBoolValue() const;

	std::string GetDebugString() const;

	void Set(int IntValue);
	void Set(double DoubleValue);
	void Set(const std::string &StringValue);
	void Set(bool BoolValue);

	enum class eStrongerValue { This, Merged };
	void Merge(cSimpleValue &MergedValue, eStrongerValue StrongerValue);
};
