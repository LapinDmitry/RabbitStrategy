#pragma once

#define HandlerFunc function<void(void*)>

class Domen
{
public:
	// Конструктор пустого домена, который только может хранить map из обработчиков
	Domen(int domenId = D_NONE);
	// Конструктор полного домена, он сразу подключается к брокеру и запрашивает у сервера адрес, если нужно
	Domen(string address, int port, string login, string password,
		int userId, int domenId, mutex* none_pMutHandlers = nullptr, map <int, HandlerFunc>* none_pHandlers = nullptr);
	
	void startSession();
	void stopSession();
	
	// Привязать общий обработчик к сообщениям от брокера
	void bindListening();
	// Добавить обработчик
	void addHandler(int typeMes, HandlerFunc handler);

	// Отправить сообщение
	void sendMessage(int to, int type, char* pData = nullptr, int dataSize = 0, function<void()> callback = nullptr);
	void sendSuperMessage(int addressTo, int domenTo, int to, int from, int domen, int type, char* pData = nullptr, int dataSize = 0, function<void()> callback = nullptr);
	void sendSuperMessage(string queueName, int to, int from, int domen, int type, char* pData = nullptr, int dataSize = 0, function<void()> callback = nullptr);
	void sendSuperMessage(string exchange, string routeKey, int to, int from, int domen, int type, char* pData = nullptr, int dataSize = 0, function<void()> callback = nullptr);
	
	// Гетеры для некоторых нужд
	void getMapHandlers(mutex*& pMutHandlers, map <int, HandlerFunc>*& pHandlers);
	int getUserId() { return m_userId; }
private:
	// Запрос адреса у сервера
	int requestToServerDomenNone();

	// Данные домена
	int m_userId;
	int m_domenId;
	string m_queueName;

	// Сетевой обработчик
	RabbitWorker m_worker;

	// Список обработчиков
	mutex m_mutHandlers;
	map <int, HandlerFunc> m_handlers;

	// Указатель на список обработчиков NONE домена
	mutex* m_none_pMutHandlers;
	map <int, HandlerFunc>* m_none_pHandlers;
};

