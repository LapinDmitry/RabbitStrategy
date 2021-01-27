#pragma once
#include "../../../asgard/Asgard/Core/Messages/AsgardMessages.Core.h"
#include "../../../asgard/Asgard/Core/AsgardDefines.h"

#define T_REGESTRATON "reg"
#define T_OPERATION "oper"

enum eAddress : int
{
	A_NULL = -1,
	A_SERVER = AR_SERVER,
	A_CLIENT_FIRST = 101,
	A_ALL_CLIENTS = AR_CLIENTS
};

enum eDomen : int
{
	D_NULL = -1000,
	D_REGISTER,
	D_NONE = ACT_NONE,
	
	
};

enum eMessage : int
{
	M_ANY = AM_ANY_MESSAGE,
	M_CLIENT_CONNECT = AWM_CLIENT_CONNECT,
	M_CLIENT_DISCONNECT = AWM_CLIENT_DISCONNECT,
	M_REGISTRATION = M_CLIENT_DISCONNECT + 1000
};

// При отправлении сообщения напрямую в очередь
#define EXC_NONE "" 
#define EXC_DIRECT "amq.direct"
// Ключ при отправке в fonout (широковещательный запрос)
#define KEY_NONE ""

#define QUEUE_PREFIX "q_"
#define ROUTE_PREFIX "k_"