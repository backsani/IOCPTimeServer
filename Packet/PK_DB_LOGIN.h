#pragma once
#include "Packet.h"

class PK_DB_LOGIN : public Packet
{

public:
	PK_DB_LOGIN();
	int Serialaze(char* idBuffer, int idBufLength, char* passwordBuffer, int passwordBufLength);
	int DeSerialaze(char* buffer);
};

