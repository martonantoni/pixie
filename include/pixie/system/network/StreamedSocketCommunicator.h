#pragma once

class cStreamedSocket2;

class cStreamedSocketCommunicator
{
protected:
	std::unique_ptr<cStreamedSocket2> mStream;
public:
	virtual ~cStreamedSocketCommunicator()=default;
	virtual void ParseStream(const cMemoryStream &Stream)=0;      // called in network thread
	virtual void CanSend() {}                                     // called in network thread
	virtual void StreamClosed()=0;                                // called in network thread
	virtual void StreamConnected() {}                             // called in network thread
};

