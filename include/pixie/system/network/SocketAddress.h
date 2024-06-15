#pragma once

class cSocketAddress
{
	struct cUnresolvedAddress
	{
		std::string mAddressString; 
		int mPort;
		bool mIsValid=false;
	} mUnresolvedAddress;
	mutable struct cResolvedAddress
	{
		sockaddr_in mAddress;
		bool mIsValid=false;
	} mResolvedAddress;
	void Resolve() const;
public:
	cSocketAddress();
	cSocketAddress(const std::string &AddressString, int Port=-1);
	const sockaddr_in *GetSockaddr_in() const;
	void SetSockaddr_in(const sockaddr_in &Address);
	void SetAddressString(const std::string &AddressString, int Port=-1);
	void SetPort(int Port);
	std::string GetHostName() const;
	std::string toString() const;
};