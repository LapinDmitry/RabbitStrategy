#pragma once
struct Header
{
	int addressTo;
	int addressFrom;
	int domenTo;
	int type;

	Header();
	Header(int addressTo, int addressFrom, int domenTo, int type);
};

struct Message
{
	Header header;

	// Байты данных клиента				// КЛАСС НЕ ОСВОБОЖДАЕТ ЭТУ ПАМЯТЬ
	char* pData;
	int dataSize;

	// Байты упакованные для отправки	// КЛАСС НЕ ОСВОБОЖДАЕТ ЭТУ ПАМЯТЬ
	const char* pBuffer;
	int bufferSize;

	// Запаковка и распаковка происходит сразу в конструкторах

	// Конструктор на отправку
	Message(int addressTo, int addressFrom, int domenTo, int type, char* pData, int dataSize);
	// Конструктор на приём
	Message(const char* pBuffer, int bufferSize);

private:
	const unsigned char HEADER_SIZE = sizeof(Header);
};

