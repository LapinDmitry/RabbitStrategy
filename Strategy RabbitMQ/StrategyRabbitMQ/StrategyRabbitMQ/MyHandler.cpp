#include "pch.h"

//#include "MyHandler.h"

// Конструктор
MyHandler::MyHandler(string address, int port, string login, string password, bool* quitFlag) :
	SimplePocoHandler(address, port),
	m_сonnection(this, AMQP::Login(login, password), "/"),
	m_сhannel(&m_сonnection),
	m_pQuitFlag(quitFlag),
	m_сonnect(false)
{	};

// Деструктор
MyHandler::~MyHandler()
{
	m_сhannel.close();
	m_сonnection.close();
}

// Виртуальный метод вызывается после установки соединения
void MyHandler::onReady(AMQP::Connection* connection)
{
	m_сonnect = true;
}

// Виртуальный метод вызывается при внутренней ошибке
void MyHandler::onError(AMQP::Connection* connection, const char* message)
{
	m_сonnect = false;
	cerr << "Error: message" << endl;
}

// Виртуальный метод вызывается после правильного закрытия соединения
void MyHandler::onClosed(AMQP::Connection* connection)
{
	// Если указателя не существует или он говорит, о том, что этот выход запланированый
	if ((m_pQuitFlag) && (!*m_pQuitFlag))
	{
		m_сonnect = false;
		cerr << "The connection is closed on the server side." << endl;
	}

}