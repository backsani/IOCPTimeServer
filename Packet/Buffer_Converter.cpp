#define _CRT_SECURE_NO_WARNINGS
#include "Buffer_Converter.h"
#include <string.h>

Buffer_Converter::Buffer_Converter() {

}
Buffer_Converter::~Buffer_Converter() {

}

PK_Data Buffer_Converter::GetHeader(char* buffer)
{
	PK_Data header;
	memcpy(&header, buffer, sizeof(PK_Data));

	return header;
}