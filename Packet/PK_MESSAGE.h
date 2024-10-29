#pragma once
#include "Packet.h"

class PK_MESSAGE : public Packet
{

public:
	PK_MESSAGE();
	int Serialaze(char* buffer, int BufLength);
	int DeSerialaze(char* buffer);
};