#include "pch.h"
#include "Message.h"
#include "Domen.h"

void Domen::startSession()
{
	m_worker.StartSession();
}

void Domen::stopSession()
{
	m_worker.StopSession();
}

string genQueueName(int address, int domen)
{
	return QUEUE_PREFIX + to_string(address) + "_" + to_string(domen);
}

string genRouteKey(int address, int domen)
{
	return ROUTE_PREFIX + to_string(address) + "_" + to_string(domen);
}

void Domen::sendMessage(int to, int type, char* pData, int dataSize, function<void()> callback)
{	sendSuperMessage(to, m_domenId, to, m_userId, m_domenId, type, pData, dataSize, callback);	}

void Domen::sendSuperMessage(int addressTo, int domenTo, int to, int from, int domen, int type, char* pData, int dataSize, function<void()> callback)
{	sendSuperMessage(EXC_DIRECT, genRouteKey(addressTo,domenTo), to, from, domen, type, pData, dataSize, callback);	}

void Domen::sendSuperMessage(string queueName, int to, int from, int domen, int type, char* pData, int dataSize, function<void()> callback)
{	sendSuperMessage(EXC_NONE, queueName, to, from, domen, type, pData, dataSize, callback);	}

void Domen::sendSuperMessage(string exchange, string routeKey, int to, int from, int domen, int type, char* pData, int dataSize, function<void()> callback)
{
	Message mes(to, from, domen, type, pData, dataSize);
	const char* buf = mes.pBuffer;
	m_worker.ToSending(exchange, routeKey, buf, mes.bufferSize, [buf, callback]() {
		delete[] buf;
		if (callback)
		{
			thread t(callback);
			t.detach();
		}
		});
}


void Domen::getMapHandlers(mutex*& pMutHandlers, map <int, HandlerFunc>*& pHandlers)
{
	pMutHandlers = &m_mutHandlers;
	pHandlers = &m_handlers;
}



int Domen::requestToServerDomenNone()
{
	HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
	string temporaryQueueName;
	// Регестрируем уникальную очередь
	m_worker.DeclareQueueExclusive([&](const std::string& name, uint32_t messagecount, uint32_t consumercount) {
		temporaryQueueName = name;
		SetEvent(event);
		});

	WaitForSingleObject(event, INFINITE);

	// Отправляем серверу адреc своей уникальной очереди
	sendSuperMessage(A_SERVER, D_REGISTER, A_SERVER, m_userId, m_domenId, M_REGISTRATION, const_cast<char*>(temporaryQueueName.c_str()), temporaryQueueName.length());

	int userId = A_NULL;

	// Слушаем ответ сервера, узнаем свой адресс
	m_worker.ListenQueue(temporaryQueueName, T_REGESTRATON, [&](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered) {
		Message mes(message.body(), message.bodySize());
		userId = mes.header.addressTo;
		SetEvent(event);
		});

	WaitForSingleObject(event, INFINITE);
	// Прекращаем обрабатывать сообщения регистрации
	m_worker.RemoveQueue(temporaryQueueName);
	return userId;
}

HandlerFunc findAndCall(mutex* pMutHandlers, map<int, HandlerFunc>* pHandlers, int type)
{
	HandlerFunc func = nullptr;

	pMutHandlers->lock();
	if (pHandlers->find(M_ANY) != pHandlers->end())
		func = (*pHandlers)[M_ANY];
	else if (pHandlers->find(type) != pHandlers->end())
		func = (*pHandlers)[type];
	pMutHandlers->unlock();

	return func;
}

Domen::Domen(int domenId)
{
	m_domenId = domenId;
}

Domen::Domen(string address, int port, string login, string password, int userId, int domenId, mutex* none_pMutHandlers, map<int, HandlerFunc>* none_pHandlers)
{
	m_userId = userId;
	m_domenId = domenId;
	m_none_pMutHandlers = none_pMutHandlers;
	m_none_pHandlers = none_pHandlers;

	// Создаём воркера
	m_worker.CreateWorker(address, port, login, password);
	m_worker.StartSession();

	// Если нет адреса клиента, то узнаем его у сервера + зарегаем домен на стороне сервера
	if (userId != A_SERVER)
	{
		if (userId == A_NULL)
			userId = requestToServerDomenNone();	// Если на стороне сервера не зареган домен, то зарегаем его
		else if (!m_worker.CheckQueue(genQueueName(A_SERVER, m_domenId)))
			requestToServerDomenNone();
	}


	m_userId = userId;

	// Генерируем имя очереди
	m_queueName = genQueueName(m_userId, m_domenId);

	// Регестрируем очередь
	HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_worker.DeclareQueue(m_queueName, [&](const std::string& name, uint32_t messagecount, uint32_t consumercount) {
		SetEvent(event);
		});
	WaitForSingleObject(event, INFINITE);

	HANDLE event2 = CreateEvent(NULL, FALSE, FALSE, NULL);

	// Слушаем по общему ключу и по своему через точку обмена direct
	m_worker.BindQueueToExchange(m_queueName, EXC_DIRECT, genRouteKey(m_userId, m_domenId), [&]() {
		SetEvent(event);
		});
	m_worker.BindQueueToExchange(m_queueName, EXC_DIRECT, genRouteKey(A_ALL_CLIENTS, m_domenId), [&]() {
		SetEvent(event2);
		});
	
	WaitForSingleObject(event, INFINITE);
	WaitForSingleObject(event2, INFINITE);

	m_worker.StopSession();
}

void Domen::bindListening()
{
	// Если пришло сообщение очередь, то вызываем обработчик
	m_worker.ListenQueue(m_queueName, T_OPERATION,
		// Если пришло сообщение очередь, то вызываем обработчик
		[&](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered) {

			// Создаём объект сообщения, которое копирует в себя данные, отправляем его и себя в новый поток
			Message* pMes = new Message(message.body(), message.bodySize());
			thread t(
				// Обрабатываем в новом потоке
				[this, pMes]() {
					
					bool veto = (pMes->header.addressTo == A_ALL_CLIENTS && pMes->header.addressFrom == m_userId);

					if (!veto)
					{
						HandlerFunc func = findAndCall(&m_mutHandlers, &m_handlers, pMes->header.type);
						if ((func == nullptr) && (m_none_pHandlers) && (m_none_pMutHandlers))
							func = findAndCall(m_none_pMutHandlers, m_none_pHandlers, pMes->header.type);

						if (func != nullptr)
							func(pMes);
					}
			
					delete[] pMes->pData;
					delete pMes;
				});
			t.detach();
		});
}

void Domen::addHandler(int typeMes, HandlerFunc handler)
{
	m_mutHandlers.lock();
	m_handlers[typeMes] = handler;
	m_mutHandlers.unlock();
}
