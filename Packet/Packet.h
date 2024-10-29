#pragma once
#include <string>
#define BUFSIZE 256

using namespace std;

enum PK_Data
{
	MESSAGE,
	TIME
};


class Packet
{
protected:
	char buffer[BUFSIZE];
public:
	void SetBuffer(char* buffer) { memcpy(this->buffer, buffer, sizeof(buffer)); }
	char* GetBuffer() { return buffer; }
	virtual int Serialaze(char* buffer, int BufLength) = 0;
	virtual int DeSerialaze(char* buffer) = 0;
};

