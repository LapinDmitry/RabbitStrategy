#pragma once
#include "SimplePocoHandler.h"
class MyHandler :
	public SimplePocoHandler
{
public:

	MyHandler(string address, int port, string login, string password, bool* quitFlag = nullptr);

	~MyHandler();

	AMQP::Channel& GetChannel() { return m_�hannel; }
	bool IsConnect() { return m_�onnect; }


private:
	AMQP::Connection m_�onnection;
	AMQP::Channel m_�hannel;

	bool* m_pQuitFlag;
	bool m_�onnect;

	void onReady(AMQP::Connection* connection)						override;
	void onError(AMQP::Connection* connection, const char* message)	override;
	void onClosed(AMQP::Connection* connection)						override;
};

