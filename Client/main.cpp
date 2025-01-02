#define _CRT_SECURE_NO_WARNINGS         // �ֽ� VC++ ������ �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <iostream>
#include <tchar.h>
#include <vector>
#include <cstdio>

#include "PacketSDK.h"

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    50

enum class UserStatus
{
	Main,
	Login,
	Signup
};

// ���� �Լ� ���� ��� �� ����
void err_quit(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// ���� �Լ� ���� ���
void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

int main(int argc, char* argv[])
{
    int retval;

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // connect()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    // ������ ��ſ� ����� ����
    char buf[BUFSIZE];
    char hello[10] = "req time";
    memcpy(buf, hello, 10);

    std::vector<std::unique_ptr<Packet>> packet;
    packet.push_back(std::make_unique<PK_MESSAGE>());
    packet.push_back(std::make_unique<PK_TIME>());
    packet.push_back(std::make_unique<PK_DB_LOGIN>());

	char gameId[21] = { 0 };
	char gamePassword[21] = { 0 };
	char gameName[21] = { 0 };

    Buffer_Converter bufferCon;
    char ch;
    int bufLength = 0;
    PK_Data currentHeader;

	UserStatus currentStatus = UserStatus::Main;

    while (1) 
	{
		if (currentStatus == UserStatus::Main)
		{
			int selectStatus = 0;
			cout << "�α����ϱ� : 1 \nȸ�������ϱ� : 2\n";
			while (!(cin >> selectStatus)) {
				std::cout << "�α����̳� ȸ�������� ���� 1,2�� �Է����ּ���! \n" << std::endl;
				std::cin.clear(); // ��Ʈ���� ���¸� �ʱ�ȭ
				std::cin.ignore(10000, '\n'); // �߸��� �Է��� ���ۿ��� ����
				std::cout << "�α����ϱ� : 1 \nȸ�������ϱ� : 2\n"; // �ٽ� �Է� ��û
			}

			switch (selectStatus)
			{
			case 1:
				currentStatus = UserStatus::Login;
				break;
			case 2:
				currentStatus = UserStatus::Signup;
				break;
			default:
				break;
			}
		}

		else if (currentStatus == UserStatus::Login)
		{

			cout << "--------------------------------\n--          �� �� ��          --\n--------------------------------\n" << endl;
			cout << "���̵� �Է����ּ���(20��) : ";
			std::cin >> gameId;
			cout << "��й�ȣ�� �Է����ּ���(20��) : ";
			std::cin >> gamePassword;

			strcat(buf, gameId);
			strcat(buf, gamePassword);

			bufLength = packet[LOGIN]->Serialaze(buf, strlen(buf));

            retval = send(sock, buf, BUFSIZE, 0);
            if (retval == SOCKET_ERROR) {
                err_display("send()");
                break;
            }

            retval = recv(sock, buf, BUFSIZE, 0);
            if (retval == SOCKET_ERROR) {
                err_display("send()");
                break;
            }
		}

		
    }



    printf("�α��� ����\n");



    while (true)
    {
        printf("s �Է� �� ���� �߼�, t �Է� �� �ð� ��û\n");

        ch = getchar();
        if (ch == 's')
        {
            printf("���� ���ڸ� �Է����ּ���\n");
            scanf("%s", &buf);
            bufLength = packet[MESSAGE]->Serialaze(buf, strlen(buf));
            
        }
        else if (ch == 't')
        {
            memcpy(buf, hello, 10);
            bufLength = packet[TIME]->Serialaze(buf, strlen(buf));
        }
        else
        {
            continue;
        }


        retval = send(sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }

        retval = recv(sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }

        currentHeader = bufferCon.GetHeader(buf);

        packet[currentHeader]->DeSerialaze(buf);
        strcpy(buf, packet[currentHeader]->GetBuffer());

        printf("%s\n", buf);
        ch = getchar();
    }

    // closesocket()
    closesocket(sock);

    // ���� ����
    WSACleanup();
    return 0;
}



/*
else if (currentStatus == UserStatus::Signup)
		{
			cout << "--------------------------------\n--          ȸ������          --\n--------------------------------\n" << endl;
			cout << "���̵� �Է����ּ���(20��) : ";
			std::cin >> gameId;
			cout << "��й�ȣ�� �Է����ּ���(20��) : ";
			std::cin >> gamePassword;
			cout << "�г����� �Է����ּ���(20��) : ";
			std::cin >> gameName;

			gameIdLength = strlen(gameId);
			gamePasswordLength = strlen(gamePassword);
			gameNameLength = strlen(gameName);

			memset(SignupBind, 0, sizeof(SignupBind));

			SignupBind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
			SignupBind[0].buffer = gameName;
			SignupBind[0].buffer_length = sizeof(gameName);
			SignupBind[0].is_null = 0;
			SignupBind[0].length = &gameNameLength;

			SignupBind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
			SignupBind[1].buffer = gameId;
			SignupBind[1].buffer_length = sizeof(gameId);
			SignupBind[1].is_null = 0;
			SignupBind[1].length = &gameIdLength;

			SignupBind[2].buffer_type = MYSQL_TYPE_VAR_STRING;
			SignupBind[2].buffer = gamePassword;
			SignupBind[2].buffer_length = sizeof(gamePassword);
			SignupBind[2].is_null = 0;
			SignupBind[2].length = &gamePasswordLength;

			if (mysql_stmt_bind_param(SignupStmt, SignupBind)) {
				std::cerr << "mysql_stmt_bind_param() failed \n";
				std::cerr << mysql_stmt_error(SignupStmt) << std::endl;
				mysql_stmt_close(SignupStmt);
				mysql_close(conn);
				return EXIT_FAILURE;
			}

			if (mysql_stmt_execute(SignupStmt)) {
				if (mysql_stmt_errno(SignupStmt) == 1062)				//errno�� ���� �ڵ带 �޾ƿ´�. 1062�� �̹� �����ϴ� �����̸Ӹ� Ű�� ���Խõ� �� �߻��Ѵ�.
				{
					cerr << "�̹� �����ϴ� ���̵��Դϴ�." << endl;
				}
				else
				{
					cerr << "mysql_stmt_execute() failed \n";
					cerr << mysql_stmt_error(SignupStmt) << endl;
				}
			}
			else {
				cout << "Data inserted successfully!" << endl;
			}
		}
*/