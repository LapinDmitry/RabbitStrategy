// StrategyRabbitMQ.cpp: определяет процедуры инициализации для библиотеки DLL.
//

#include "pch.h"

#define MYDLL_EXPORTS

#include "framework.h"

#include "Message.h"
#include "Domen.h"
#include "../../../BrokerStrategy/BStrategy.h"
#include "StrategyRabbitMQ.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

map<int, shared_ptr<Domen>> g_domens;

//
//TODO: если эта библиотека DLL динамически связана с библиотеками DLL MFC,
//		все функции, экспортированные из данной DLL-библиотеки, которые выполняют вызовы к
//		MFC, должны содержать макрос AFX_MANAGE_STATE в
//		самое начало функции.
//
//		Например:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// тело нормальной функции
//		}
//
//		Важно, чтобы данный макрос был представлен в каждой
//		функции до вызова MFC.  Это означает, что
//		должен стоять в качестве первого оператора в
//		функции и предшествовать даже любым объявлениям переменных объекта,
//		поскольку их конструкторы могут выполнять вызовы к MFC
//		DLL.
//
//		В Технических указаниях MFC 33 и 58 содержатся более
//		подробные сведения.
//

// CStrategyRabbitMQApp

BEGIN_MESSAGE_MAP(CStrategyRabbitMQApp, CWinApp)
END_MESSAGE_MAP()


// Создание CStrategyRabbitMQApp

CStrategyRabbitMQApp::CStrategyRabbitMQApp()
{
	// TODO: добавьте код создания,
	// Размещает весь важный код инициализации в InitInstance
}


// Единственный объект CStrategyRabbitMQApp

CStrategyRabbitMQApp theApp;


// Инициализация CStrategyRabbitMQApp

BOOL CStrategyRabbitMQApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

RabbitBrokerStrategy::RabbitBrokerStrategy()
{
	m_userId = A_NULL;
}

void RabbitBrokerStrategy::initUser()
{
	m_mutDomens.lock();
	g_domens[D_NONE] = make_shared<Domen>(D_NONE);
	m_mutDomens.unlock();
}

void RabbitBrokerStrategy::CreateServer(string ip, string port, string login, string password)
{
	m_userIdCount = A_CLIENT_FIRST;
	m_userId = A_SERVER;

	g_domens[D_NONE] = make_shared<Domen>(D_NONE);
	this->CreateDomen(D_REGISTER, ip, port, login, password);

	g_domens[D_REGISTER]->addHandler(M_REGISTRATION, [this, ip, port, login, password](void* message) {
		Message* pMes = static_cast<Message*>(message);
		// Вытащим имя очереди
		string queueName(pMes->pData, pMes->pData + pMes->dataSize);
		
		int domen = pMes->header.domenTo;

		// Выясним, надо ли давать адрес
		int id;
		// Если у клиента нет адреса, выдадим его
		if (pMes->header.addressFrom == A_NULL)
		{
			m_mutCounter.lock();
			id = m_userIdCount++;
			m_mutCounter.unlock();
		}
		else
		{
			id = pMes->header.addressFrom;
		}

		// Если на стороне сервера не зареган домен, то создадим его
		m_mutDomens.lock();
		if (g_domens.find(pMes->header.domenTo) == g_domens.end())
		{
			this->CreateDomen(domen, ip, port, login, password);
			g_domens[domen]->startSession();
		}			
		m_mutDomens.unlock();

		g_domens[D_REGISTER]->sendSuperMessage(queueName, id, A_SERVER, domen, M_REGISTRATION, nullptr, 0);
		});
}

void RabbitBrokerStrategy::GetParams(int& d_none_domen, int& t_any_message, int& t_client_connect, int& t_client_disconnect, int& a_server, int& a_all_clients, HANDLE h_can_quit_event)
{
	d_none_domen = D_NONE;
	t_any_message = M_ANY;
	t_client_connect = M_CLIENT_CONNECT;
	t_client_disconnect = M_CLIENT_DISCONNECT;
	a_all_clients = A_ALL_CLIENTS;
	a_server = A_SERVER;
	h_can_quit_event = m_hCanQuite;
}

void RabbitBrokerStrategy::CreateDomen(int domenId, string ip, string port, string login, string password)
{
	// Берём указатель на map none домена
	map <int, HandlerFunc>* p_none_handlers = nullptr;
	mutex* p_none_mutex = nullptr;
	g_domens[D_NONE]->getMapHandlers(p_none_mutex, p_none_handlers);

	// Создаём домен
	g_domens[domenId]
		= make_shared<Domen>(ip, stoi(port), login, password, m_userId, domenId, p_none_mutex, p_none_handlers);
	g_domens[domenId]->bindListening();

	// Если до этого не был известен адрес, то берём его у домена, он по надобности запрашивает адрес у сервера
	if (m_userId == A_NULL)
		m_userId = g_domens[domenId]->getUserId();
}

void RabbitBrokerStrategy::StartDomen(int domenId)
{
	g_domens[domenId]->startSession();
	//			g_domens[domenId]->sendSuperMessage(A_SERVER, D_NONE, A_SERVER, m_userId, domenId, M_CLIENT_CONNECT);
	g_domens[domenId]->sendMessage(A_SERVER, M_CLIENT_CONNECT);
}

void RabbitBrokerStrategy::StopDomen(int domenId)
{
	//HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
	//g_domens[domenId]->sendSuperMessage(A_SERVER, D_NONE, A_SERVER, m_userId, domenId, M_CLIENT_DISCONNECT, nullptr, 0,
	//	[&]() {
	//		SetEvent(event);
	//	});
	//WaitForSingleObject(event, INFINITE);
	//			g_domens[domenId]->sendSuperMessage(A_SERVER, D_NONE, A_SERVER, m_userId, domenId, M_CLIENT_DISCONNECT);
	g_domens[domenId]->sendMessage(A_SERVER, M_CLIENT_DISCONNECT);
	Sleep(2000);
	g_domens[domenId]->stopSession();
}

void RabbitBrokerStrategy::StartServer()
{
	for (auto it = g_domens.begin(); it != g_domens.end(); ++it)
		if (it->first != D_NONE) 
			it->second->startSession();
}

void RabbitBrokerStrategy::StopServer()
{
	for (auto it = g_domens.begin(); it != g_domens.end(); ++it)
		if (it->first != D_NONE)
			it->second->stopSession();
	Sleep(2000);
	SetEvent(m_hCanQuite);
}

void RabbitBrokerStrategy::SendMessageTo(int domenId, int to, int type, function<void(CArchive&)> serialize)
{
	CMemFile memFile;
	CArchive archive(&memFile, CArchive::store | CArchive::bNoFlushOnDelete);

	serialize(archive);
	
	archive.Flush();
	int dataSize = int(memFile.GetLength());
	char* data = reinterpret_cast<char*>(memFile.Detach());
	g_domens[domenId]->sendMessage(to, type, data, dataSize);
	delete[]data;
}

void RabbitBrokerStrategy::SendMessageTo(int domenId, int to, int type, char* pData, size_t size)
{
	g_domens[domenId]->sendMessage(to, type, pData, size);
}

void RabbitBrokerStrategy::ReadMessage(void* message, function<void(CArchive&)> deserialize)
{
	Message* pMes = static_cast<Message*>(message);
	CMemFile memFile((BYTE*)pMes->pData, pMes->dataSize);
	CArchive archive(&memFile, CArchive::load);

	deserialize(archive);

	memFile.Detach();
}

void RabbitBrokerStrategy::ReadMessage(void* message, void* pData, size_t& size)
{
	Message* pMes = static_cast<Message*>(message);
	pData = pMes->pData;
	size = pMes->dataSize;
}

void RabbitBrokerStrategy::BIND_HANDLER(int domenId, int messageID, function<void(void*)> handler)
{
	g_domens[domenId]->addHandler(messageID, handler);
}

int RabbitBrokerStrategy::GetAdressFrom(void* message)
{
	return static_cast<Message*>(message)->header.addressFrom;
}

int RabbitBrokerStrategy::GetTypeMessage(void* message)
{
	return static_cast<Message*>(message)->header.type;
}

int RabbitBrokerStrategy::GetDomenID(void* message)
{
	return static_cast<Message*>(message)->header.domenTo;
}




