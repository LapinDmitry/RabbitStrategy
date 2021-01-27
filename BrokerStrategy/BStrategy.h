#pragma once

// Инструмент для ассинхроннгого сетевого общения
class BStrategy
{
public:
	BStrategy() {}
	~BStrategy() {}

	virtual void initUser() {}
	
	virtual void GetParams(
		int& d_none_domen,
		int& t_any_message,
		int& t_client_connect,
		int& t_client_disconnect,
		int& a_server,
		int& a_all_clients,
		HANDLE h_can_quit_event) {}

	virtual void CreateServer(std::string ip, std::string port, std::string login, std::string password) {}
	virtual void CreateDomen(int domenId, std::string ip, std::string port, std::string login, std::string password){}

	virtual void StartDomen(int domenId){}
	virtual void StopDomen(int domenId){}

	virtual void StartServer(){}
	virtual void StopServer() {}

	virtual void SendMessageTo(int domenId, int to, int type, std::function<void(CArchive&)> serialize) {}
	virtual void SendMessageTo(int domenId, int to, int type, char* pData = nullptr, size_t size = 0) {}

	virtual void BIND_HANDLER(int domenId, int messageID, std::function<void(void*)> handler) {}

	virtual void ReadMessage(void* message, std::function<void(CArchive&)> deserialize) {}
	virtual void ReadMessage(void* message, void* pData, size_t& size) {}

	virtual int GetAdressFrom(void* message) { return 0; }
	virtual int GetTypeMessage(void* message) { return 0; }
	virtual int GetDomenID(void* message) { return 0; }
};