#pragma once
#include "Packet.h"

class Buffer_Converter {

public:
	Buffer_Converter();

	PK_Data GetHeader(char* buffer);

	~Buffer_Converter();
};