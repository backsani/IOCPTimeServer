#define _CRT_SECURE_NO_WARNINGS         // 최신 VC++ 컴파일 시 경고 방지
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
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

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

    // 윈속 초기화
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

    // 데이터 통신에 사용할 변수
    char buf[BUFSIZE];
    char hello[10] = "req time";
    memcpy(buf, hello, 10);

    std::vector<std::unique_ptr<Packet>> packet;
    packet.push_back(std::make_unique<PK_MESSAGE>());
    packet.push_back(std::make_unique<PK_TIME>());

    Buffer_Converter bufferCon;
    char ch;
    int bufLength = 0;
    PK_Data currentHeader;

    printf("서버 연결 성공\n");

    while (true)
    {
        printf("s 입력 시 문자 발송, t 입력 시 시간 요청\n");

        ch = getchar();
        if (ch == 's')
        {
            printf("보낼 문자를 입력해주세요\n");
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

    // 윈속 종료
    WSACleanup();
    return 0;
}
