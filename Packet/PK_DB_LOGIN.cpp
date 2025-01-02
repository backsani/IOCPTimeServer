#include "PK_DB_LOGIN.h"

PK_DB_LOGIN::PK_DB_LOGIN()
{
	memset(this->buffer, 0, sizeof(this->buffer));
}

int PK_DB_LOGIN::Serialaze(char* idBuffer, int idBufLength, char* passwordBuffer, int passwordBufLength)
{
	int Length = 0;

	PK_Data header = PK_Data::LOGIN;
	memcpy(this->buffer, idBuffer, idBufLength);

	memcpy(buffer, &header, sizeof(PK_Data));
	Length += sizeof(PK_Data);

	memcpy(buffer + Length, &idBufLength, sizeof(idBufLength));
	Length += sizeof(idBufLength);

	memcpy(buffer + Length, idBuffer, sizeof(char) * idBufLength);
	Length += idBufLength;

	memcpy(buffer + Length, &passwordBufLength, sizeof(passwordBufLength));
	Length += sizeof(passwordBufLength);

	memcpy(buffer + Length, passwordBuffer, sizeof(char) * passwordBufLength);
	Length += passwordBufLength;

	buffer[Length] = '\0';

	return Length;
}

int PK_DB_LOGIN::DeSerialaze(char* buffer)
{
	int Length = 0;
	int idLength = 0;
	int passwordLength = 0;

	char success[] = "Success Deserialaze";

	Length += sizeof(PK_Data);

	memcpy(&idLength, buffer + Length, sizeof(idLength));
	Length += sizeof(idLength);

	memcpy(this->id, buffer + Length, sizeof(char) * idLength);
	Length += idLength;

	memcpy(&passwordLength, buffer + Length, sizeof(passwordLength));
	Length += sizeof(passwordLength);

	memcpy(this->password, buffer + Length, sizeof(char) * passwordLength);
	Length += passwordLength;

	memcpy(this->buffer, success, sizeof(success));

	this->id[idLength] = '\0';
	this->password[passwordLength] = '\0';

	return idLength + passwordLength;
}
