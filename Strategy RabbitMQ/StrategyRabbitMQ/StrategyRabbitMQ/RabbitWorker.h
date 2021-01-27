#pragma once
#include "MyHandler.h"


struct Parcel
{
	string exchangeName;
	string routeKey;
	const char* pData;
	int nLength;
	AMQP::SuccessCallback callback;

	Parcel(string exchangeName,string routeKey, const char* pData, int nLength, AMQP::SuccessCallback callback)
		: exchangeName(exchangeName), routeKey(routeKey), pData(pData), nLength(nLength), callback(callback){};
};

class RabbitWorker
{
public:
	void CreateWorker(string address, int port, string login, string password);
	void KillWorker();

	void StartSession();
	void StopSession();

	// Задекларировать очередь
	// QueueCallback = std::function<void(const std::string &name, uint32_t messagecount, uint32_t consumercount)>
	void DeclareQueue(string queueNm, AMQP::QueueCallback callback = nullptr);
	// QueueCallback = std::function<void(const std::string &name, uint32_t messagecount, uint32_t consumercount)>
	void DeclareQueueExclusive(AMQP::QueueCallback callback);
	
	// Положить в очередь на отправку
	// SuccessCallback  =  std::function<void()>
	void ToSending(string exchangeName, string routeKey, const char* pData,int nLength, AMQP::SuccessCallback callback = nullptr);
	
	//Начать слушать очередь
	//MessageCallback   =   std::function<void[&](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered)>;
	void ListenQueue(string queueName, string operationTag, AMQP::MessageCallback operation);

	// Прекратить слушать очередь
	// CancelCallback = std::function<void(const std::string & consumer)>;
	void MuteQueue(string operationTag, AMQP::CancelCallback callback = nullptr);

	// Удалить очередь с сообщениями
	// SuccessCallback  =  std::function<void()>
	void RemoveQueue(string queueName, int flags = 0, AMQP::SuccessCallback callback = nullptr);

	// Задекларировать точку доступа
	// SuccessCallback  =  std::function<void()>
	void DeclareExchange(string exchangeName, AMQP::ExchangeType type, AMQP::SuccessCallback callback = nullptr);

	// Подписка очереди на точку доступа по ключу
	// SuccessCallback  =  std::function<void()>
	void BindQueueToExchange(string queueName, string exchangeName, string routeKey, AMQP::SuccessCallback callback = nullptr);

	bool CheckQueue(string queueName);

private:
	string m_address;
	int m_port;
	string m_login;
	string m_password;
	MyHandler* m_pHandler;
	shared_ptr<thread> m_postProcess;

	bool m_quit;

	queue<Parcel> m_forSending;
	mutex m_mutSendingQueue;
	mutex m_mutChannel;

	void SendingLoop();
};

