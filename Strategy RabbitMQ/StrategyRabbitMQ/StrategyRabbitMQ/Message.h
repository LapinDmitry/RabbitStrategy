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

	// ����� ������ �������				// ����� �� ����������� ��� ������
	char* pData;
	int dataSize;

	// ����� ����������� ��� ��������	// ����� �� ����������� ��� ������
	const char* pBuffer;
	int bufferSize;

	// ��������� � ���������� ���������� ����� � �������������

	// ����������� �� ��������
	Message(int addressTo, int addressFrom, int domenTo, int type, char* pData, int dataSize);
	// ����������� �� ����
	Message(const char* pBuffer, int bufferSize);

private:
	const unsigned char HEADER_SIZE = sizeof(Header);
};

