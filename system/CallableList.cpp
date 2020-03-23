#include "StdAfx.h"

cCallableListItemBase::~cCallableListItemBase()
{
	if(mItemHolder)
		mItemHolder->ItemDied();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cCallableListItemHolder::cCallableListItemHolder(cCallableList &List,cCallableListItemBase *Item)
	: mItem(Item)
	, mList(List)
{
	ASSERT(!mItem->mItemHolder);
	mItem->mItemHolder=this;
}

cCallableListItemHolder::~cCallableListItemHolder()
{
	InvalidateItem();
}

void cCallableListItemHolder::ItemDied()
{
	mList.UnregisterItem(mItem);
}

void cCallableListItemHolder::Call(bool InvalidateBeforeCall)
{
	if(IsItemValid())
	{
		cCallableListItemBase *Item=mItem;
		if(InvalidateBeforeCall)
		{
			InvalidateItem();
		}
		Item->Call();
	}
}

void cCallableListItemHolder::InvalidateItem()
{
	if(mItem)
	{
		ASSERT(mItem->mItemHolder==this);
		mItem->mItemHolder=nullptr;
		mItem=nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cCallableList::cCallableList(int FlagsToUse)
	: mIsInCall(false)
	, mFlags(FlagsToUse)
{
}

cCallableList::~cCallableList()
{
}

void cCallableList::Call()
{
	mIsInCall=true;
	for(cCallableListItemHolder *ItemHolder: mList)
	{
		ItemHolder->Call(mFlags&int(Flags::AutoUnregisterAfterCall));
	}
	mIsInCall=false;
	for(auto i=mList.begin(),iend=mList.end();i!=iend;)
	{
		cCallableListItemHolder *ItemHolder=*i;
		if(ItemHolder->IsItemValid())
		{
			++i;
		}
		else
		{
			i=mList.erase(i);
			delete ItemHolder;
		}
	}
}

void cCallableList::RegisterItem(cCallableListItemBase *Item)
{
	if(Item->mItemHolder)
	{
		ASSERT(mFlags&int(Flags::IgnoreReregistering)); // Registering already registered
		return;
	}
	if(mFlags&int(Flags::AddItemsToFront))
		mList.push_front(new cCallableListItemHolder(*this,Item));
	else
		mList.push_back(new cCallableListItemHolder(*this,Item));
}

void cCallableList::UnregisterItem(cCallableListItemBase *Item)
{
	cCallableListItemHolder *ItemHolder=Item->mItemHolder;
	if(!ItemHolder)
	{
		ASSERT(mFlags&int(Flags::IgnoreUnunregistering)); // calling Unregister without Register
		return;
	}
	if(mIsInCall)
	{
		Item->mItemHolder->InvalidateItem();
	}
	else
	{
		mList.remove(ItemHolder);
		delete ItemHolder;
	}
}

cCallableList::cCallableList(cCallableList &&Source)
	: mList(std::move(Source.mList))
	, mFlags(Source.mFlags)
	, mIsInCall(false)
{
	ASSERT(!Source.mIsInCall);
}

cCallableList &cCallableList::operator=(cCallableList &&Source)
{
	mList=std::move(Source.mList);
	mFlags=Source.mFlags;
	mIsInCall=false;
	ASSERT(!Source.mIsInCall);
	return *this;
}
