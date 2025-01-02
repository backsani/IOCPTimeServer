#pragma once
#include <string>
#define BUFSIZE 256

using namespace std;

enum PK_Data
{
	MESSAGE,
	TIME,
	LOGIN
};


class Packet
{
protected:
	char buffer[BUFSIZE];
	char id[15];
	char password[15];
public:
	void SetBuffer(char* buffer) { memcpy(this->buffer, buffer, sizeof(buffer)); }
	char* GetBuffer() { return buffer; }
	char* GetId() { return id; }
	char* GetPassword() { return password; }
	virtual int Serialaze(char* buffer, int BufLength) = 0;
	virtual int DeSerialaze(char* buffer) = 0;
};

