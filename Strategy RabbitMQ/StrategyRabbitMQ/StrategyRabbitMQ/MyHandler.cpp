#include "pch.h"

//#include "MyHandler.h"

// �����������
MyHandler::MyHandler(string address, int port, string login, string password, bool* quitFlag) :
	SimplePocoHandler(address, port),
	m_�onnection(this, AMQP::Login(login, password), "/"),
	m_�hannel(&m_�onnection),
	m_pQuitFlag(quitFlag),
	m_�onnect(false)
{	};

// ����������
MyHandler::~MyHandler()
{
	m_�hannel.close();
	m_�onnection.close();
}

// ����������� ����� ���������� ����� ��������� ����������
void MyHandler::onReady(AMQP::Connection* connection)
{
	m_�onnect = true;
}

// ����������� ����� ���������� ��� ���������� ������
void MyHandler::onError(AMQP::Connection* connection, const char* message)
{
	m_�onnect = false;
	cerr << "Error: message" << endl;
}

// ����������� ����� ���������� ����� ����������� �������� ����������
void MyHandler::onClosed(AMQP::Connection* connection)
{
	// ���� ��������� �� ���������� ��� �� �������, � ���, ��� ���� ����� ��������������
	if ((m_pQuitFlag) && (!*m_pQuitFlag))
	{
		m_�onnect = false;
		cerr << "The connection is closed on the server side." << endl;
	}

}