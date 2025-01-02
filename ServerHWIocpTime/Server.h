#pragma once
#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <iostream>
#include <vector>
#include <thread>
#include <ctime>
#include <string>

#include "PacketSDK.h"


#define PORT 9000
#define MAX_WORKERTHREAD 4
#define MAX_SOCKBUF 256

#define DBSERVERIP "127.0.0.1"
#define DBSERVERPORT 8000

enum class IOOperation
{
	RECV,
	SEND
};

struct mOverlappedEx
{
	WSAOVERLAPPED _wsaOverlapped;
	SOCKET _socketClient;
	WSABUF _wsaBuf;
	IOOperation _operation;
};

struct ClientInfo
{
	SOCKET _socketClient;
	mOverlappedEx _recvOverlappedEx;
	mOverlappedEx _sendOverlappedEx;

	char			mRecvBuf[MAX_SOCKBUF]; //데이터 버퍼
	char			mSendBuf[MAX_SOCKBUF]; //데이터 버퍼

	ClientInfo()
	{
		ZeroMemory(&_recvOverlappedEx, sizeof(mOverlappedEx));
		ZeroMemory(&_sendOverlappedEx, sizeof(mOverlappedEx));
		_socketClient = INVALID_SOCKET;
	}
};

class Server
{
private:
	//데이터베이스 서버 연결 소켓
	SOCKET DBSock;
	int retval;
	char mDBSocketBuf[1024] = { 0 };
	//클라이언트 연결

	SOCKET listenSocket = INVALID_SOCKET;

	std::vector<std::unique_ptr<Packet>> packet;

	std::vector<ClientInfo> mClientInfos;

	std::vector<std::thread> mWorkerThreads;

	std::thread mAccepterThread;

	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;

	bool mIsWorkerRun = true;

	bool mIsAccepterRun = true;

	char mSocketBuf[1024] = { 0 };

public:

	Server();

	~Server() { WSACleanup(); }

	bool InitSocket();

	bool InitDBSocket();

	bool BindSocket();

	bool StartServer(const UINT32 maxClientCount);

	void DestroyThread();

	void CreateClient(const UINT32 maxClientCount);
	
	bool CreateWokerThread();

	bool CreateAccepterThread();


	bool BindRecv(ClientInfo* pClientInfo);

	bool SendMsg(ClientInfo* pClientInfo, char* pMsg, int nLen);

private:
	void WokerThread();

	void AccepterThread();

	string TimeToString(tm* localTime);

	void CloseSocket(ClientInfo* pClientInfo);

	bool BindIOCompletionPort(ClientInfo* pClientInfo);

	ClientInfo* GetEmptyClientInfo();

	void Broadcast(ClientInfo* pClientInfo, char* buffer, int length);
};

