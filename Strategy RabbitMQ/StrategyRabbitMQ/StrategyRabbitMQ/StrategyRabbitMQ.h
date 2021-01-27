// StrategyRabbitMQ.h: основной файл заголовка для библиотеки DLL StrategyRabbitMQ
//
#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#ifdef MYDLL_EXPORTS
#define MYEXP dllexport
#else
#define MYEXP dllimport
#endif

#include "resource.h"		// основные символы

class __declspec(MYEXP)RabbitBrokerStrategy : public BStrategy
{
private:
	std::mutex m_mutDomens;
	// Поля для клиента
	int m_userId;
	// Поля для сервера
	int m_userIdCount;
	std::mutex m_mutCounter;
	HANDLE m_hCanQuite = CreateEvent(NULL, FALSE, FALSE, NULL);
public:
	RabbitBrokerStrategy();
	~RabbitBrokerStrategy() {}

	void initUser() override;
	

	void GetParams(
		int& d_none_domen,
		int& t_any_message,
		int& t_client_connect,
		int& t_client_disconnect,
		int& a_server,
		int& a_all_clients,
		HANDLE h_can_quit_event) override;

	void CreateServer(std::string ip, std::string port, std::string login, std::string password) override;
	void CreateDomen(int domenId, std::string ip, std::string port, std::string login, std::string password) override;

	void StartDomen(int domenId) override;

	void StopDomen(int domenId) override;

	void StartServer() override;
	void StopServer() override;

	void SendMessageTo(int domenId, int to, int type, std::function<void(CArchive&)> serialize) override;
	void SendMessageTo(int domenId, int to, int type, char* pData = nullptr, size_t size = 0) override;

	void ReadMessage(void* message, std::function<void(CArchive&)> deserialize) override;
	void ReadMessage(void* message, void* pData, size_t& size) override;

	void BIND_HANDLER(int domenId, int messageID, std::function<void(void*)> handler) override;

	int GetAdressFrom(void* message) override;
	int GetTypeMessage(void* message) override;
	int GetDomenID(void* message) override;
};

// CStrategyRabbitMQApp
// Реализацию этого класса см. в файле StrategyRabbitMQ.cpp
//

class CStrategyRabbitMQApp : public CWinApp
{
public:
	CStrategyRabbitMQApp();

// Переопределение
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP();
};

