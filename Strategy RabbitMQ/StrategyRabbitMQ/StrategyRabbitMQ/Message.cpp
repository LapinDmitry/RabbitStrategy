#include "pch.h"
#include "Message.h"

Header::Header()
{
}

Header::Header(int addressTo, int addressFrom, int domenTo, int type)
{
	this->addressTo = addressTo;
	this->addressFrom = addressFrom;
	this->domenTo = domenTo;
	this->type = type;
}

Message::Message(int addressTo, int addressFrom, int domenTo, int type, char* pData, int dataSize)
	:pData(pData), dataSize(dataSize), header(addressTo, addressFrom, domenTo, type)
{
	bufferSize = HEADER_SIZE + dataSize;
	char* buf = new char[bufferSize];

	memcpy(buf, &header, HEADER_SIZE);

	if (dataSize > 0)
		memcpy(buf + HEADER_SIZE, pData, dataSize);

	pBuffer = buf;
}

Message::Message(const char* pBuffer, int bufferSize)
	:pBuffer(pBuffer), bufferSize(bufferSize)
{
	memcpy(&header, pBuffer, HEADER_SIZE);

	dataSize = bufferSize - HEADER_SIZE;
	if (dataSize > 0)
	{
		char* dat = new char[dataSize];
		memcpy(dat, pBuffer + HEADER_SIZE, dataSize);
		pData = dat;
	}
	else
	{
		pData = nullptr;
	}
		
}
