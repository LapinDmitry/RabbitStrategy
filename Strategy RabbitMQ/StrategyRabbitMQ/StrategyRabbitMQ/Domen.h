#pragma once

#define HandlerFunc function<void(void*)>

class Domen
{
public:
	// ����������� ������� ������, ������� ������ ����� ������� map �� ������������
	Domen(int domenId = D_NONE);
	// ����������� ������� ������, �� ����� ������������ � ������� � ����������� � ������� �����, ���� �����
	Domen(string address, int port, string login, string password,
		int userId, int domenId, mutex* none_pMutHandlers = nullptr, map <int, HandlerFunc>* none_pHandlers = nullptr);
	
	void startSession();
	void stopSession();
	
	// ��������� ����� ���������� � ���������� �� �������
	void bindListening();
	// �������� ����������
	void addHandler(int typeMes, HandlerFunc handler);

	// ��������� ���������
	void sendMessage(int to, int type, char* pData = nullptr, int dataSize = 0, function<void()> callback = nullptr);
	void sendSuperMessage(int addressTo, int domenTo, int to, int from, int domen, int type, char* pData = nullptr, int dataSize = 0, function<void()> callback = nullptr);
	void sendSuperMessage(string queueName, int to, int from, int domen, int type, char* pData = nullptr, int dataSize = 0, function<void()> callback = nullptr);
	void sendSuperMessage(string exchange, string routeKey, int to, int from, int domen, int type, char* pData = nullptr, int dataSize = 0, function<void()> callback = nullptr);
	
	// ������ ��� ��������� ����
	void getMapHandlers(mutex*& pMutHandlers, map <int, HandlerFunc>*& pHandlers);
	int getUserId() { return m_userId; }
private:
	// ������ ������ � �������
	int requestToServerDomenNone();

	// ������ ������
	int m_userId;
	int m_domenId;
	string m_queueName;

	// ������� ����������
	RabbitWorker m_worker;

	// ������ ������������
	mutex m_mutHandlers;
	map <int, HandlerFunc> m_handlers;

	// ��������� �� ������ ������������ NONE ������
	mutex* m_none_pMutHandlers;
	map <int, HandlerFunc>* m_none_pHandlers;
};

