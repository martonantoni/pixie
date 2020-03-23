#pragma once

#define ALMOST_NULL ((T *)((char *)-1))

template<class T,int ListCount=1> struct tIntrusiveListItemBase
{
	struct cIntrusiveListItemNeighbours
	{
		T *mNext,*mPrev;
		cIntrusiveListItemNeighbours(): mNext(NULL), mPrev(NULL) {}
		~cIntrusiveListItemNeighbours() { ASSERT(mNext==NULL&&mPrev==NULL); }
	};
	cIntrusiveListItemNeighbours mIntrusiveListItemNeighbours[ListCount];
	virtual ~tIntrusiveListItemBase() {}
	bool IsInIntrusiveList(int ListIndex=0) const { return mIntrusiveListItemNeighbours[ListIndex].mNext!=NULL; }
};

template<class T,int ListIndex=0> struct tDefaultIntrusiveListAccessor
{
	static T *&Next(T *Item) { return Item->mIntrusiveListItemNeighbours[ListIndex].mNext; }
	static T *&Prev(T *Item) { return Item->mIntrusiveListItemNeighbours[ListIndex].mPrev; }
	static const T *Next(const T *Item) { return Item->mIntrusiveListItemNeighbours[ListIndex].mNext; }
	static const T *Prev(const T *Item) { return Item->mIntrusiveListItemNeighbours[ListIndex].mPrev; }
	static bool IsInList(const T *Item) { return Next(Item)!=NULL; }
};

template<class T,int ListIndex=0> class tIntrusiveList   
{
	typedef tDefaultIntrusiveListAccessor<T,ListIndex> Accessor;
	T *mFirst,*mLast;
	size_t mItemCount;
public:
	tIntrusiveList(): mFirst(ALMOST_NULL), mLast(ALMOST_NULL), mItemCount(0) {}
	tIntrusiveList(const tIntrusiveList &) = delete;
	tIntrusiveList(tIntrusiveList &&Source);
	tIntrusiveList &operator=(const tIntrusiveList &) = delete;
	tIntrusiveList &operator=(tIntrusiveList &&Source);
	static const T *GetTerminator() { return (const T *)ALMOST_NULL; }
	static bool IsInList(const T *Item) { return Accessor::Next(Item)!=NULL; }
	void push_back(T *Item);
	void push_front(T *Item);
	T *pop_back();
	T *pop_front();
	T *back() const { return mLast; }
	T *front() const { return mFirst; }
	static T *get_next(T *Item) { return Accessor::Next(Item)==GetTerminator()?NULL:Accessor::Next(Item); }
	static T *get_prev(T *Item) { return Accessor::Prev(Item)==GetTerminator()?NULL:Accessor::Prev(Item); }
	static const T *get_next(const T *Item) { return Accessor::Next(Item)==GetTerminator()?NULL:Accessor::Next(Item); }
	static const T *get_prev(const T *Item) { return Accessor::Prev(Item)==GetTerminator()?NULL:Accessor::Prev(Item); }
	int empty() const { return mFirst==ALMOST_NULL; }
	size_t size() const { return mItemCount; }
	void clear();
	class const_iterator
	{
	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef const T* value_type;
		typedef ptrdiff_t difference_type;
		typedef value_type* pointer;
		typedef value_type& reference;

		const T *mItem;
		friend class tIntrusiveList<T>;
		const_iterator(const T *pItem): mItem(pItem) {}
		const_iterator(const const_iterator &Source): mItem(Source.mItem) {}
		const_iterator(): mItem(ALMOST_NULL) {}
		void operator++() { mItem=Accessor::Next(mItem); }
		const T *operator*() const { return mItem; }
		const T *operator->() const { return mItem; }

		int operator==(const const_iterator &Other) const { return mItem==Other.mItem; }
		int operator!=(const const_iterator &Other) const { return mItem!=Other.mItem; }
	};
	class iterator: public const_iterator
	{
	public:
		typedef T* value_type;
		typedef value_type* pointer;
		typedef value_type& reference;

		iterator(T *pItem): const_iterator(pItem) {}
		iterator(const iterator &Source): const_iterator(Source) {}
		iterator() {}
		void operator++() { this->mItem=Accessor::Next(this->mItem); }
		T *operator*() { return (T *)this->mItem; }
		T *operator->() { return (T *)this->mItem; }

		int operator==(const iterator &Other) const { return this->mItem==Other.mItem; }
		int operator!=(const iterator &Other) const { return this->mItem!=Other.mItem; }
	};
	iterator begin() { return iterator(mFirst); }
	iterator end() { return iterator(ALMOST_NULL); }
	const_iterator begin() const { return const_iterator(mFirst); }
	const_iterator end() const { return const_iterator(ALMOST_NULL); }
	iterator erase(iterator &i);
	iterator insert(iterator &i,T *NewItem); // Item must not be in an other allocless list
	void remove(T *Item);
	void ExchangeLists(tIntrusiveList<T,ListIndex> &OtherList);
	void splice(iterator i,tIntrusiveList<T,ListIndex> &OtherList);
};

template<class T,int ListIndex> 
tIntrusiveList<T,ListIndex>::tIntrusiveList(tIntrusiveList &&Source)
	: mFirst(Source.mFirst)
	, mLast(Source.mLast)
	, mItemCount(Source.mItemCount)
{
	Source.mFirst=Source.mLast=ALMOST_NULL;
	Source.mItemCount=0;
}

template<class T, int ListIndex>
auto tIntrusiveList<T, ListIndex>::operator=(tIntrusiveList &&Source) -> tIntrusiveList &
{
	mFirst=Source.mFirst;
	mLast=Source.mLast;
	mItemCount=Source.mItemCount;
	Source.mFirst=Source.mLast=ALMOST_NULL;
	Source.mItemCount=0;
	return *this;
}

template<class T,int ListIndex> void tIntrusiveList<T,ListIndex>::push_back(T *Item)
{
	ASSERT(Accessor::Next(Item)==NULL&&Accessor::Prev(Item)==NULL);
	++mItemCount;
	Accessor::Next(Item)=ALMOST_NULL;
	if(mLast!=ALMOST_NULL)
	{
		Accessor::Prev(Item)=mLast;
		Accessor::Next(mLast)=Item;
		mLast=Item;
	}
	else
	{
		mFirst=mLast=Item;
		Accessor::Prev(Item)=ALMOST_NULL;
	}
}

template<class T,int ListIndex> void tIntrusiveList<T,ListIndex>::push_front(T *Item)
{
	ASSERT(Accessor::Next(Item)==NULL&&Accessor::Prev(Item)==NULL);
	++mItemCount;
	Accessor::Prev(Item)=ALMOST_NULL;
	if(mFirst!=ALMOST_NULL)
	{
		Accessor::Next(Item)=mFirst;
		Accessor::Prev(mFirst)=Item;
		mFirst=Item;
	}
	else
	{
		mFirst=mLast=Item;
		Accessor::Next(Item)=ALMOST_NULL;
	}
}

template<class T,int ListIndex> T *tIntrusiveList<T,ListIndex>::pop_back()
{
	--mItemCount;
	T *Item=mLast;
	if(Item!=ALMOST_NULL)
	{
		mLast=Accessor::Prev(mLast);
		if(mLast!=ALMOST_NULL)
			Accessor::Next(mLast)=ALMOST_NULL;
		else
			mFirst=ALMOST_NULL;
		Accessor::Prev(Item)=Accessor::Next(Item)=NULL;
	}
	return Item;
}

template<class T,int ListIndex> T *tIntrusiveList<T,ListIndex>::pop_front()
{
	--mItemCount;
	T *Item=mFirst;
	if(Item!=ALMOST_NULL)
	{
		mFirst=Accessor::Next(mFirst);
		if(mFirst!=ALMOST_NULL)
			Accessor::Prev(mFirst)=ALMOST_NULL;
		else
			mLast=ALMOST_NULL;
		Accessor::Prev(Item)=Accessor::Next(Item)=NULL;
	}
	return Item;
}

template<class T,int ListIndex> void tIntrusiveList<T,ListIndex>::remove(T *Item)
{
	if(Item==NULL||Accessor::Prev(Item)==NULL)
	{
		// Invalid item: NULL or not in an allocless list.
		return;
	}
	--mItemCount;
	if(Accessor::Prev(Item)!=ALMOST_NULL)
		Accessor::Next(Accessor::Prev(Item))=Accessor::Next(Item);
	else
		mFirst=Accessor::Next(Item);
	if(Accessor::Next(Item)!=ALMOST_NULL)
		Accessor::Prev(Accessor::Next(Item))=Accessor::Prev(Item);
	else
		mLast=Accessor::Prev(Item);
	Accessor::Prev(Item)=Accessor::Next(Item)=NULL;
}

template<class T,int ListIndex> typename tIntrusiveList<T,ListIndex>::iterator tIntrusiveList<T,ListIndex>::erase(iterator &i)
{
	iterator NextIterator(Accessor::Next(*i));
	remove(*i);
	return NextIterator;
}

template<class T,int ListIndex> typename tIntrusiveList<T,ListIndex>::iterator tIntrusiveList<T,ListIndex>::insert(iterator &i,T *NewItem)
{
	ASSERT(Accessor::Next(NewItem)==NULL&&Accessor::Prev(NewItem)==NULL);
	++mItemCount;
	T *IteratorItem=(T *)i.mItem;
	Accessor::Next(NewItem)=IteratorItem;
	if(IteratorItem!=ALMOST_NULL)
	{
		if(Accessor::Prev(IteratorItem)!=ALMOST_NULL)
			Accessor::Next(Accessor::Prev(IteratorItem))=NewItem;
		else
			mFirst=NewItem;
		Accessor::Prev(NewItem)=Accessor::Prev(IteratorItem);
		Accessor::Prev(IteratorItem)=NewItem;
	}
	else  // this will be the last item
	{
		Accessor::Prev(NewItem)=mLast;
		if(mLast!=ALMOST_NULL)
			Accessor::Next(mLast)=NewItem;
		else
			mFirst=NewItem;
		mLast=NewItem;
	}
	return iterator(NewItem);
}

template<class T,int ListIndex> void tIntrusiveList<T,ListIndex>::clear()
{
	for(T *Item=mFirst;Item!=ALMOST_NULL;)
	{
		T *NextItem=Accessor::Next(Item);
		Accessor::Next(Item)=Accessor::Prev(Item)=NULL;
		Item=NextItem;
	}
	mFirst=mLast=ALMOST_NULL;
	mItemCount=0;
}

template<class T,int ListIndex> void tIntrusiveList<T,ListIndex>::ExchangeLists(tIntrusiveList<T,ListIndex> &OtherList)
{
	std::swap(mFirst,OtherList.mFirst);
	std::swap(mLast,OtherList.mLast);
	std::swap(mItemCount,OtherList.mItemCount);
}

template<class T,int ListIndex> void tIntrusiveList<T,ListIndex>::splice(iterator i,tIntrusiveList<T,ListIndex> &OtherList)
{
	if(OtherList.mItemCount==0)
		return;
	if(mItemCount==0)
	{
		std::swap(mFirst,OtherList.mFirst);
		std::swap(mLast,OtherList.mLast);
		std::swap(mItemCount,OtherList.mItemCount);
		return;
	}
	T *IteratorItem=(T *)i.mItem;
	if(IteratorItem!=ALMOST_NULL)
	{
		// inserting before IteratorItem
		Accessor::Prev(OtherList.mFirst)=Accessor::Prev(IteratorItem);
		if(Accessor::Prev(IteratorItem))
		{
			Accessor::Next(Accessor::Prev(IteratorItem))=OtherList.mFirst;
		}
		else
		{
			mFirst=OtherList.mFirst;
		}
		Accessor::Prev(IteratorItem)=OtherList.mLast;
		Accessor::Next(OtherList.mLast)=IteratorItem;
	}
	else
	{
		// inserting at the end
		Accessor::Next(mLast)=OtherList.mFirst;
		Accessor::Prev(OtherList.mFirst)=mLast;
		mLast=OtherList.mLast;
	}
	mItemCount+=OtherList.mItemCount;
	OtherList.mItemCount=0;
	OtherList.mFirst=OtherList.mLast=ALMOST_NULL;
}

#undef ALMOST_NULL