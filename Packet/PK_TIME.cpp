#include "PK_TIME.h"

PK_TIME::PK_TIME()
{
	memset(this->buffer, 0, sizeof(this->buffer));
}

int PK_TIME::Serialaze(char* buffer, int BufLength)
{
	int Length = 0;

	PK_Data header = PK_Data::TIME;
	memcpy(this->buffer, buffer, BufLength);

	memcpy(buffer, &header, sizeof(PK_Data));
	Length += sizeof(PK_Data);

	memcpy(buffer + Length, &BufLength, sizeof(BufLength));
	Length += sizeof(BufLength);

	memcpy(buffer + Length, &this->buffer, sizeof(char) * BufLength);
	Length += BufLength;

	buffer[Length] = '\0';

	return Length;
}

int PK_TIME::DeSerialaze(char* buffer)
{
	int Length = 0;
	int bufLength = 0;

	Length += sizeof(PK_Data);

	memcpy(&bufLength, buffer + Length, sizeof(bufLength));
	Length += sizeof(bufLength);

	memcpy(this->buffer, buffer + Length, sizeof(char) * bufLength);
	Length += bufLength;

	this->buffer[bufLength] = '\0';
	return bufLength;
}
