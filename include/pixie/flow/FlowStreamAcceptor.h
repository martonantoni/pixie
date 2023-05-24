#pragma once

class cFlowStreamAcceptor: public cSocketTaker
{
	tIntrusivePtr<cTCPAcceptor> mAcceptor;
protected:
	virtual void OnNewSocket(std::unique_ptr<cSocket> NewSocket) override;
public:
	cFlowStreamAcceptor();
	void SetLog(std::shared_ptr<cTextLog> log);
	void Init(unsigned short Port);
};