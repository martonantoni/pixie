#include <ws2tcpip.h>

cSocketAddress::cSocketAddress()
{
	memset(&mResolvedAddress.mAddress,0,sizeof(sockaddr_in));
}

cSocketAddress::cSocketAddress(const std::string &AddressString, int Port)
	: cSocketAddress()
{
	SetAddressString(AddressString, Port);
}

const sockaddr_in *cSocketAddress::GetSockaddr_in() const
{
	if(!mResolvedAddress.mIsValid)
		Resolve();
	return mResolvedAddress.mIsValid?&mResolvedAddress.mAddress:NULL;
}

void cSocketAddress::SetSockaddr_in(const sockaddr_in &Address)
{
	mResolvedAddress.mAddress=Address;
	mResolvedAddress.mIsValid=true;
	int Port=ntohs(Address.sin_port);
	mUnresolvedAddress.mAddressString=fmt::sprintf("%d.%d.%d.%d:%d",
		Address.sin_addr.S_un.S_un_b.s_b1, Address.sin_addr.S_un.S_un_b.s_b2,
		Address.sin_addr.S_un.S_un_b.s_b3, Address.sin_addr.S_un.S_un_b.s_b4,
		Port);
	mUnresolvedAddress.mPort=Port;
	mUnresolvedAddress.mIsValid=true;
}

void cSocketAddress::SetAddressString(const std::string &AddressString, int Port)
{
	size_t PortPos=AddressString.find(':');
	if(PortPos!=std::string::npos)
	{
		mUnresolvedAddress.mAddressString=AddressString.substr(0,PortPos);
		mUnresolvedAddress.mPort=atoi(AddressString.substr(PortPos+1).c_str());
	}
	else
	{
		mUnresolvedAddress.mAddressString=AddressString;
		mUnresolvedAddress.mPort=Port;
	}
	mUnresolvedAddress.mIsValid=true;
	mResolvedAddress.mIsValid=false;
}

void cSocketAddress::SetPort(int Port)
{
	mUnresolvedAddress.mPort=Port;
	mResolvedAddress.mAddress.sin_port=htons(Port);
}

void cSocketAddress::Resolve() const
{
	addrinfo aiHints;
	addrinfo *aiList = NULL;

// Setup the hints address info structure
// which is passed to the getaddrinfo() function
	memset(&aiHints, 0, sizeof(aiHints));
	aiHints.ai_family = AF_INET;
	aiHints.ai_socktype = SOCK_STREAM;
	aiHints.ai_protocol = IPPROTO_TCP;

// Call getaddrinfo(). If the call succeeds,
// the aiList variable will hold a linked list
// of addrinfo structures containing response
// information about the host
	if(getaddrinfo(mUnresolvedAddress.mAddressString.c_str(), NULL, &aiHints, &aiList)||!aiList)
		return;

	mResolvedAddress.mAddress=*(sockaddr_in *)aiList->ai_addr;
	mResolvedAddress.mAddress.sin_port=htons(mUnresolvedAddress.mPort);
	mResolvedAddress.mIsValid=true;
	freeaddrinfo(aiList);
}

std::string cSocketAddress::GetHostName() const
{ 
	return mUnresolvedAddress.mIsValid?mUnresolvedAddress.mAddressString:std::string();
}

std::string cSocketAddress::ToString() const
{ 
	return mUnresolvedAddress.mIsValid?fmt::sprintf("%s:%d", mUnresolvedAddress.mAddressString.c_str(), mUnresolvedAddress.mPort):std::string(); 
}
