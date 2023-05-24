#pragma once

class cCallableListItemHolder;
class cCallableList;

class cCallableListItemBase
{
	friend cCallableList;
	friend cCallableListItemHolder;
	cCallableListItemHolder *mItemHolder;
public:
	cCallableListItemBase(): mItemHolder(nullptr) {}
	virtual ~cCallableListItemBase();
	virtual void Call()=0;
};

class cCallableList
{
public:
	enum class Flags
	{
		AddItemsToFront         = 1,
		IgnoreReregistering     = 2,
		IgnoreUnunregistering   = 4,
		AutoUnregisterAfterCall = 8   // recommended to add Flags::IgnoreReregistering | Flags::IgnoreUnunregistering
	};
private:
	int mFlags;
	typedef tIntrusiveList<cCallableListItemHolder> cItemHolders;
	cItemHolders mList;
	bool mIsInCall;
public:
	cCallableList(int FlagsToUse=0);
	cCallableList(const cCallableList &) = delete;
	cCallableList(cCallableList &&);
	cCallableList &operator=(const cCallableList &) = delete;
	cCallableList &operator=(cCallableList &&);
	~cCallableList();
	void Call();

	void RegisterItem(cCallableListItemBase *Item);
	void UnregisterItem(cCallableListItemBase *Item);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation side, do not use this outside:

class cCallableListItemHolder: public tIntrusiveListItemBase<cCallableListItemHolder>
{
	cCallableList &mList;
	cCallableListItemBase *mItem;
public:
	cCallableListItemHolder(cCallableList &List, cCallableListItemBase *Item);
	cCallableListItemHolder(const cCallableListItemHolder &) = delete;
	cCallableListItemHolder &operator =(const cCallableListItemHolder &) = delete;
	~cCallableListItemHolder();
	void ItemDied();
	void InvalidateItem();
	void Call(bool InvalidateBeforeCall);
	bool IsItemValid() const { return mItem!=nullptr; }
};
