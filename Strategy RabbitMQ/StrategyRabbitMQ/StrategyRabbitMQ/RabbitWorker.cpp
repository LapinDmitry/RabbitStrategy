#include "pch.h"

//#include "RabbitWorker.h"

// Подключение, создание хендлера, соединения, канала
void RabbitWorker::CreateWorker(string address, int port, std::string login, std::string password)
{
	m_mutChannel.lock();
	m_pHandler = new MyHandler(address, port, login, password, &m_quit);
	m_mutChannel.unlock();

	m_address	= address;
	m_port		= port;
	m_login		= login;
	m_password	= password;
}

// Удаление воркера
void RabbitWorker::KillWorker()
{
	delete m_pHandler;
	//delete m_pHandler.get();
}

// Начало сессии
void RabbitWorker::StartSession()
{
	m_quit = false;
	m_postProcess = make_shared<thread>([&]() { this->SendingLoop(); });
}

// Завершение сессии
void RabbitWorker::StopSession()
{
	m_quit = true;
	m_postProcess->join();
}

// Задекларировать очередь
void RabbitWorker::DeclareQueue(string queueNm, AMQP::QueueCallback callback)
{
	m_mutChannel.lock();
	if (callback == nullptr)
		m_pHandler->GetChannel().declareQueue(queueNm);
	else
		m_pHandler->GetChannel().declareQueue(queueNm).onSuccess(callback);
	m_mutChannel.unlock();
}

// Задекларировать очередь с уникальным именем 
void RabbitWorker::DeclareQueueExclusive(AMQP::QueueCallback callback)
{
	m_mutChannel.lock();
	m_pHandler->GetChannel().declareQueue(AMQP::exclusive).onSuccess(callback);
	m_mutChannel.unlock();
}

// Добавить в очередь отправки
void RabbitWorker::ToSending(string exchangeName, string routeKey,const char* pData, int nLength, AMQP::SuccessCallback callback)
{
	m_mutSendingQueue.lock();
	m_forSending.push(Parcel(exchangeName, routeKey, pData, nLength, callback));
	m_mutSendingQueue.unlock();
}


//Начать слушать очередь
void RabbitWorker::ListenQueue(string queueName, string operationTag, AMQP::MessageCallback  operation)
{
	m_mutChannel.lock();
	m_pHandler->GetChannel().consume(queueName, operationTag, AMQP::noack).onReceived(operation);
	m_mutChannel.unlock();
}

// Прекратить слушать очередь
void RabbitWorker::MuteQueue(string operationTag, AMQP::CancelCallback callback)
{
	m_mutChannel.lock();
	if (callback == nullptr)
		m_pHandler->GetChannel().cancel(operationTag).onSuccess(callback);
	else
		m_pHandler->GetChannel().cancel(operationTag);
	m_mutChannel.unlock();
}

void RabbitWorker::RemoveQueue(string queueName, int flags, AMQP::SuccessCallback callback)
{
	m_mutChannel.lock();
	if (callback == nullptr)
		m_pHandler->GetChannel().removeQueue(queueName, flags).onSuccess(callback);
	else
		m_pHandler->GetChannel().removeQueue(queueName, flags);
	m_mutChannel.unlock();
}

void RabbitWorker::DeclareExchange(string exchangeName, AMQP::ExchangeType type, AMQP::SuccessCallback callback)
{
	m_mutChannel.lock();
	if (callback == nullptr)
		m_pHandler->GetChannel().declareExchange(exchangeName, type);
	else
		m_pHandler->GetChannel().declareExchange(exchangeName, type).onSuccess(callback);
	m_mutChannel.unlock();
}

// Подписка очереди на точку доступа по ключу
void RabbitWorker::BindQueueToExchange(string queueName,string exchangeName, string routeKey, AMQP::SuccessCallback callback)
{
	m_mutChannel.lock();
	if (callback == nullptr)
		m_pHandler->GetChannel().bindQueue(exchangeName, queueName, routeKey);
	else
		m_pHandler->GetChannel().bindQueue(exchangeName, queueName, routeKey).onSuccess(callback);
	m_mutChannel.unlock();
}

bool RabbitWorker::CheckQueue(string queueName)
{
	HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
	bool res;
	m_pHandler->GetChannel().declareQueue(queueName, AMQP::passive).onSuccess([&](const std::string& name, uint32_t messagecount, uint32_t consumercount) {
		res = true;
		SetEvent(event);
	});
	m_pHandler->GetChannel().declareQueue(queueName, AMQP::passive).onError([&](const char* message) {
		res = false;
		SetEvent(event);
		});
	WaitForSingleObject(event, INFINITE);
	return res;
}

// Воркер, который крутиться и отправляет, принимает сообщения
void RabbitWorker::SendingLoop()
{
	while (!m_quit)
	{
		// Отдаём сообщение хендлеру
		m_mutSendingQueue.lock();
		if (!m_forSending.empty())
		{
			m_mutChannel.lock();
			m_pHandler->GetChannel().onReady([&]()
				{
					if (m_pHandler->IsConnect())
					{
						Parcel parcel = m_forSending.front();

						if (parcel.callback == nullptr)
							m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength);
						else
							m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength).onSuccess(parcel.callback);
						
						//m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength).onBegin([](int16_t code, const std::string& message) {
						//	cout << "1" << endl;
						//	});
						//m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength).onBounced([](const AMQP::Message& message, int16_t code, const std::string& description) {
						//	cout << "2" << endl;
						//	});
						//m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength).onComplete([]() {
						//	cout << "3" << endl;
						//	});
						//m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength).onData([](const char* data, size_t size) {
						//	cout << "4" << endl;
						//	});
						//m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength).onError([](const char* message) {
						//	cout << "5" << endl;
						//	});
						//m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength).onFinalize([]() {
						//	cout << "6" << endl;
						//	});
						//m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength).onHeaders([](const AMQP::MetaData& metaData) {
						//	cout << "7" << endl;
						//	});
						//m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength).onMessage([](const AMQP::Message& message, int16_t code, const std::string& description) {
						//	cout << "8" << endl;
						//	});
						//m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength).onReceived([](const AMQP::Message& message, int16_t code, const std::string& description) {
						//	cout << "9" << endl;
						//	});
						//m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength).onReturned([](const AMQP::Message& message, int16_t code, const std::string& description) {
						//	cout << "a" << endl;
						//	});
						//m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength).onSize([](uint64_t messagesize) {
						//	cout << "b" << endl;
						//	});
						//m_pHandler->GetChannel().publish(parcel.exchangeName, parcel.routeKey, parcel.pData, (size_t)parcel.nLength).onSuccess([]() {
						//	cout << "c" << endl;
						//	});

						string msg(parcel.pData, parcel.pData + parcel.nLength);
						m_forSending.pop();
					}

				});
			m_mutChannel.unlock();
		}
		m_mutSendingQueue.unlock();

		// Итерация обмена данными
		m_mutChannel.lock();
		m_pHandler->loop();
		m_mutChannel.unlock();
	}
}
