#pragma once
#include "Packet.h"

class PK_TIME : public Packet
{

public:
	PK_TIME();
	int Serialaze(char* buffer, int BufLength);
	int DeSerialaze(char* buffer);
};