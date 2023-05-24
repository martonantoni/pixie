#pragma once

template<class T> class tLocatableResourceBase: public cIntrusiveRefCount
{
public:
	enum eNodeCreation { CanCreate, DoNotCreate };
private:
	struct cNode
	{
		T *mResource;
		typedef std::map<std::string,cNode *> cNodeMap;
		cNodeMap mChildren;
		cNode *mParentNode;
		cNode(cNode *ParentNode): mParentNode(ParentNode), mResource(NULL) {}
		void ResourceDestroyed();
		tIntrusivePtr<T> GetResource(const cResourceLocation &Location,eNodeCreation NodeCreation);
	};
	template<class U> friend class tLocatableResourceBase;
	typename cNode *mLocatableResourceNode;
public:
	tLocatableResourceBase(): mLocatableResourceNode(NULL) {}
	virtual ~tLocatableResourceBase();
	static T *Create(typename cNode *Node);
	tIntrusivePtr<T> GetSubResource(const cResourceLocation &SubLocation,eNodeCreation NodeCreation) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
tIntrusivePtr<T> tLocatableResourceBase<T>::cNode::GetResource(const cResourceLocation &Location,eNodeCreation NodeCreation)
{
	cNode *Node=this;
	if(NodeCreation==DoNotCreate)
	{
		for(auto &i: Location)
		{
			cNode::cNodeMap::iterator j=Node->mChildren.find(i);
			if(j==Node->mChildren.end())
			{
				return tIntrusivePtr<T>();
			}
			Node=j->second;
		}
		return tIntrusivePtr<T>(Node->mResource);  // it is valid for Node->mResource to be NULL here
	}
	else  // CanCreate:
	{
		for(auto &i: Location)
		{
			cNode *&ChildNode=Node->mChildren[i];
			if(!ChildNode)
				ChildNode=new cNode(Node);
			Node=ChildNode;
		}
		if(!Node->mResource)
			Node->mResource=tLocatableResourceBase<T>::Create(Node);
		return tIntrusivePtr<T>(Node->mResource);
	}
}

template<class T>
void tLocatableResourceBase<T>::cNode::ResourceDestroyed()
{
	mResource=NULL;
	cNode *Node=this;
	while(Node->mParentNode&&Node->mResource==NULL&&Node->mChildren.empty())
	{
		cNode *ParentNode=Node->mParentNode;
		for(cNode::cNodeMap::iterator i=ParentNode->mChildren.begin(),iend=ParentNode->mChildren.end();i!=iend;++i)
		{
			if(i->second==Node)
			{
				ParentNode->mChildren.erase(i);
				break;
			}
		}
		delete Node;
		Node=ParentNode;
	}
	if(!Node->mParentNode&&!Node->mResource&&Node->mChildren.empty())
	{
		delete Node;
	}
}


template<class T>
tLocatableResourceBase<T>::~tLocatableResourceBase() 
{ 
	if(mLocatableResourceNode) 
	{ 
		mLocatableResourceNode->ResourceDestroyed(); 
	} 
}

template<class T>
T *tLocatableResourceBase<T>::Create(typename tLocatableResourceBase<T>::cNode *Node) 
{ 
	T *Resource=new T; 
	Resource->mLocatableResourceNode=Node; 
	return Resource; 
}

template<class T>
tIntrusivePtr<T> tLocatableResourceBase<T>::GetSubResource(const cResourceLocation &SubLocation,eNodeCreation NodeCreation) const
{
	if(!mLocatableResourceNode)
	{
		const_cast<cNode *&>(mLocatableResourceNode)=new cNode(NULL);  // const_cast: late init
		const_cast<cNode *&>(mLocatableResourceNode)->mResource=(T *)(this);
	}
	return mLocatableResourceNode->GetResource(SubLocation,NodeCreation);
}
