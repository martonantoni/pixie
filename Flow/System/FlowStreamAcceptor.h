#pragma once

class cFlowStreamAcceptor: public cSocketTaker
{
	tIntrusivePtr<cTCPAcceptor> mAcceptor;
protected:
	virtual void OnNewSocket(std::unique_ptr<cSocket> NewSocket) override;
public:
	cFlowStreamAcceptor();
	void Init(unsigned short Port);
};