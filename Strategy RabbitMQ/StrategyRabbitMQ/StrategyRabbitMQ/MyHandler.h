#pragma once
#include "SimplePocoHandler.h"
class MyHandler :
	public SimplePocoHandler
{
public:

	MyHandler(string address, int port, string login, string password, bool* quitFlag = nullptr);

	~MyHandler();

	AMQP::Channel& GetChannel() { return m_ñhannel; }
	bool IsConnect() { return m_ñonnect; }


private:
	AMQP::Connection m_ñonnection;
	AMQP::Channel m_ñhannel;

	bool* m_pQuitFlag;
	bool m_ñonnect;

	void onReady(AMQP::Connection* connection)						override;
	void onError(AMQP::Connection* connection, const char* message)	override;
	void onClosed(AMQP::Connection* connection)						override;
};

