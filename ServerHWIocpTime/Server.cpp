#include "Server.h"

Server::Server()
{
	this->packet.push_back(std::make_unique<PK_MESSAGE>());
	this->packet.push_back(std::make_unique<PK_TIME>());
	this->packet.push_back(std::make_unique<PK_DB_LOGIN>());
}

bool Server::InitSocket()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		printf("[����] WSAStartup() �Լ� ���� : %d\n", WSAGetLastError());
		return false;
	}

	listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

	if (listenSocket == INVALID_SOCKET)
	{
		printf("[����] WSASocket() �Լ� ���� : %d\n", WSAGetLastError());
		return false;
	}

	printf("���� �ʱ�ȭ ����\n");
	return true;
}

bool Server::InitDBSocket()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 1;

	// socket()
	DBSock = socket(AF_INET, SOCK_STREAM, 0);
	if (DBSock == INVALID_SOCKET)
	{
		printf("[DB����] socket() �Լ� ���� : %d\n", GetLastError());
		return false;
	}

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(DBSERVERIP);
	serveraddr.sin_port = htons(DBSERVERPORT);
	int retval = connect(DBSock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
	{
		printf("[DB����] connect() �Լ� ���� : %d\n", GetLastError());
		return false;
	}
}



bool Server::BindSocket()
{
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT); //���� ��Ʈ�� �����Ѵ�.				
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//������ ������ ���� �ּ� ������ cIOCompletionPort ������ �����Ѵ�.
	if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR_IN)))
	{
		printf("[����] bind()�Լ� ���� : %d\n", WSAGetLastError());
		return false;
	}

	if (listen(listenSocket, 5))
	{
		printf("[����] listen()�Լ� ���� : %d\n", WSAGetLastError());
		return false;
	}

	printf("���� ��� ����..\n");
	return true;
}

bool Server::StartServer(const UINT32 maxClientCount)
{
	CreateClient(maxClientCount);

	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
	if (mIOCPHandle == NULL)
	{
		printf("[����] CreateIoCompletionPort() �Լ� ����: %d\n", GetLastError());
		return false;
	}

	if (!CreateWokerThread()) {
		printf("[����] CreateWokerThread() �Լ� ����: %d\n", GetLastError());
		return false;
	}

	if (!CreateAccepterThread()) {
		printf("[����] CreateAccepterThread() �Լ� ����: %d\n", GetLastError());
		return false;
	}

	printf("���� ����\n");
	return true;
}

void Server::DestroyThread()
{
	mIsWorkerRun = false;
	CloseHandle(mIOCPHandle);

	for (auto& th : mWorkerThreads)
	{
		if (th.joinable())
		{
			th.join();
		}
	}

	mIsAccepterRun = false;
	closesocket(listenSocket);

	if (mAccepterThread.joinable())
	{
		mAccepterThread.join();
	}
}

void Server::CreateClient(const UINT32 maxClientCount)
{
	for (UINT32 i = 0; i < maxClientCount; i++)
	{
		mClientInfos.emplace_back();
	}
}

bool Server::CreateWokerThread()
{
	unsigned int uiThreadId = 0;
	//WaingThread Queue�� ��� ���·� ���� ������� ���� ����Ǵ� ���� : (cpu���� * 2) + 1 
	for (int i = 0; i < MAX_WORKERTHREAD; i++)
	{
		mWorkerThreads.emplace_back([this]() { WokerThread(); });
	}

	printf("WokerThread ����..\n");
	return true;
}

bool Server::CreateAccepterThread()
{
	mAccepterThread = std::thread([this]() { AccepterThread(); });

	printf("AccepterThread ����..\n");
	return true;
}

void Server::WokerThread()
{
	ClientInfo* pClientInfo = nullptr;
	BOOL bSuccess = TRUE;
	DWORD dwIoSize = 0;
	LPOVERLAPPED lpOverlapped = NULL;
	PK_Data currentHeader;
	Buffer_Converter bufferCon;
	int bufSize = 0;


	while (mIsWorkerRun)
	{
		bSuccess = GetQueuedCompletionStatus(mIOCPHandle,
			&dwIoSize,					// ������ ���۵� ����Ʈ
			(PULONG_PTR)&pClientInfo,		// CompletionKey
			&lpOverlapped,				// Overlapped IO ��ü
			INFINITE);					// ����� �ð�

		//����� ������ ���� �޼��� ó��..
		if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped)
		{
			mIsWorkerRun = false;
			continue;
		}

		if (NULL == lpOverlapped)
		{
			continue;
		}

		//client�� ������ ��������..			
		if (FALSE == bSuccess || (0 == dwIoSize && TRUE == bSuccess))
		{
			printf("socket(%d) ���� ����\n", (int)pClientInfo->_socketClient);
			CloseSocket(pClientInfo);
			continue;
		}


		auto pOverlappedEx = (mOverlappedEx*)lpOverlapped;

		currentHeader = bufferCon.GetHeader(pClientInfo->mRecvBuf);

		//Overlapped I/O Recv�۾� ��� �� ó��
		if (IOOperation::RECV == pOverlappedEx->_operation)
		{
			bufSize = packet[currentHeader]->DeSerialaze(pClientInfo->mRecvBuf);
			strcpy(pClientInfo->mRecvBuf, packet[currentHeader]->GetBuffer());

			printf("[����] bytes : %d , msg : %s\n", dwIoSize, pClientInfo->mRecvBuf);

			if (currentHeader == MESSAGE)
			{
				//��ε�ĳ��Ʈ ���� ����
				/*int bufLength = packet[currentHeader]->Serialaze(pClientInfo->mRecvBuf, bufSize);
				Broadcast(pClientInfo, pClientInfo->mRecvBuf, bufLength);*/

			}
			else if (currentHeader == TIME)
			{
				// ���� �ð��� time_t Ÿ������ ����
				std::time_t realTime = std::time(nullptr);

				// localtime()�� ����� tm ����ü�� ��ȯ
				std::tm* localTime = std::localtime(&realTime);

				std::string currentTime = TimeToString(localTime);
				currentTime.copy(pClientInfo->mRecvBuf, currentTime.size());
				pClientInfo->mRecvBuf[currentTime.size()] = '\0';

				bufSize =  currentTime.size();
			}
			else if (currentHeader == LOGIN)
			{
				packet[currentHeader]->Serialaze(packet[currentHeader]->GetId(), sizeof(packet[currentHeader]->GetId()), packet[currentHeader]->GetPassword());
				//getId, getPass�� ���ؼ� �����ͺ��̽� ������ ���
				retval = send(DBSock, mDBSocketBuf, BUFSIZE, 0);
				if (retval == SOCKET_ERROR) 
				{
					printf("[DB����] send() �Լ� ���� : %d\n", GetLastError());
					return ;
				}

				retval = recv(DBSock, mDBSocketBuf, BUFSIZE, 0);
				if (retval == SOCKET_ERROR) 
				{
					printf("[DB����] recv() �Լ� ���� : %d\n", GetLastError());
					return ;
				}
			}
			else
			{
				printf("not exists Packet\n");
				continue;
			}

			
			//Ŭ���̾�Ʈ�� �޼����� �����Ѵ�.
			int bufLength = packet[currentHeader]->Serialaze(pClientInfo->mRecvBuf, bufSize);
			//int bufLength = packet[currentHeader]->Serialaze(packet[currentHeader]->GetBuffer());

			SendMsg(pClientInfo, pClientInfo->mRecvBuf, bufLength);

			BindRecv(pClientInfo);
		}
		//Overlapped I/O Send�۾� ��� �� ó��
		else if (IOOperation::SEND == pOverlappedEx->_operation)
		{
			printf("[�۽�] bytes : %d , msg : %s\n", dwIoSize, packet[currentHeader]->GetBuffer());
		}
		//���� ��Ȳ
		else
		{
			printf("socket(%d)���� ���ܻ�Ȳ\n", (int)pClientInfo->_socketClient);
		}
	}
}


void Server::AccepterThread()
{
	SOCKADDR_IN		stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);

	while (mIsAccepterRun)
	{
		//������ ���� ����ü�� �ε����� ���´�.
		ClientInfo* pClientInfo = GetEmptyClientInfo();
		if (NULL == pClientInfo)
		{
			printf("[����] Client Full\n");
			return;
		}

		//Ŭ���̾�Ʈ ���� ��û�� ���� ������ ��ٸ���.
		pClientInfo->_socketClient = accept(listenSocket, (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (INVALID_SOCKET == pClientInfo->_socketClient)
		{
			continue;
		}

		//I/O Completion Port��ü�� ������ �����Ų��.
		if (!BindIOCompletionPort(pClientInfo))
		{
			return;
		}

		//Recv Overlapped I/O�۾��� ��û�� ���´�.
		if (!BindRecv(pClientInfo))
		{
			return;
		}
		
		printf("Ŭ���̾�Ʈ ���� : SOCKET(%d)\n", (int)pClientInfo->_socketClient);

		//Ŭ���̾�Ʈ ���� ����
		//++mClientCnt;
	}
}

ClientInfo* Server::GetEmptyClientInfo()
{
	for (auto& client : mClientInfos)
	{
		if (INVALID_SOCKET == client._socketClient)
		{
			return &client;
		}
	}

	return nullptr;
}

bool Server::BindIOCompletionPort(ClientInfo* pClientInfo)
{
	//socket�� pClientInfo�� CompletionPort��ü�� �����Ų��.
	auto hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->_socketClient
		, mIOCPHandle
		, (ULONG_PTR)(pClientInfo), 0);

	if (NULL == hIOCP || mIOCPHandle != hIOCP)
	{
		printf("[����] CreateIoCompletionPort()�Լ� ����: %d\n", GetLastError());
		return false;
	}

	return true;
}


//WSARecv Overlapped I/O �۾��� ��Ų��.
bool Server::BindRecv(ClientInfo* pClientInfo)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	//Overlapped I/O�� ���� �� ������ ������ �ش�.
	pClientInfo->_recvOverlappedEx._wsaBuf.len = MAX_SOCKBUF;
	pClientInfo->_recvOverlappedEx._wsaBuf.buf = pClientInfo->mRecvBuf;
	pClientInfo->_recvOverlappedEx._operation = IOOperation::RECV;

	int nRet = WSARecv(pClientInfo->_socketClient,
		&(pClientInfo->_recvOverlappedEx._wsaBuf),
		1,
		&dwRecvNumBytes,
		&dwFlag,
		(LPWSAOVERLAPPED) & (pClientInfo->_recvOverlappedEx),
		NULL);

	//socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[����] WSARecv()�Լ� ���� : %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

//WSASend Overlapped I/O�۾��� ��Ų��.
bool Server::SendMsg(ClientInfo* pClientInfo, char* pMsg, int nLen)
{
	DWORD dwRecvNumBytes = 0;

	//���۵� �޼����� ����
	CopyMemory(pClientInfo->mSendBuf, pMsg, nLen);
	pClientInfo->mSendBuf[nLen] = '\0';


	//Overlapped I/O�� ���� �� ������ ������ �ش�.
	pClientInfo->_sendOverlappedEx._wsaBuf.len = nLen;
	pClientInfo->_sendOverlappedEx._wsaBuf.buf = pClientInfo->mSendBuf;
	pClientInfo->_sendOverlappedEx._operation = IOOperation::SEND;

	int nRet = WSASend(pClientInfo->_socketClient,
		&(pClientInfo->_sendOverlappedEx._wsaBuf),
		1,
		&dwRecvNumBytes,
		0,
		(LPWSAOVERLAPPED) & (pClientInfo->_sendOverlappedEx),
		NULL);

	//socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[����] WSASend()�Լ� ���� : %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

string Server::TimeToString(tm* localTime)
{
	std::string year = std::to_string(localTime->tm_year + 1900);
	std::string mon = std::to_string(localTime->tm_mon + 1);
	std::string day = std::to_string(localTime->tm_mday);
	std::string hour = std::to_string(localTime->tm_hour);
	std::string min = std::to_string(localTime->tm_min);
	std::string sec = std::to_string(localTime->tm_sec);

	std::string result = year + "��" + mon + "��" + day + "��" + hour + "��" + min + "��" + sec + "��";
	std::cout << result << std::endl;

	return result;
}

void Server::Broadcast(ClientInfo* pClientInfo, char* buffer, int length)
{
	SOCKET currentClient = pClientInfo->_socketClient;

	int retval;
	for (auto const clientInfo : mClientInfos)
	{
		if (clientInfo._socketClient != currentClient)
		{
			//ClientInfo* pClientInfo = (ClientInfo*)&clientInfo;
			recv(clientInfo._socketClient, buffer, length, 0);
		}
	}
}

void Server::CloseSocket(ClientInfo* pClientInfo)
{
	bool bIsForce = false;
	struct linger stLinger = { 0, 0 };	// SO_DONTLINGER�� ����

	// bIsForce�� true�̸� SO_LINGER, timeout = 0���� �����Ͽ� ���� ���� ��Ų��. ���� : ������ �ս��� ������ ���� 
	if (true == bIsForce)
	{
		stLinger.l_onoff = 1;
	}

	//socketClose������ ������ �ۼ����� ��� �ߴ� ��Ų��.
	shutdown(pClientInfo->_socketClient, SD_BOTH);

	//���� �ɼ��� �����Ѵ�.
	setsockopt(pClientInfo->_socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

	//���� ������ ���� ��Ų��. 
	closesocket(pClientInfo->_socketClient);

	pClientInfo->_socketClient = INVALID_SOCKET;
}