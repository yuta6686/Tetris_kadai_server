// udpserver.cpp

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <winsock.h>
#include <signal.h>
#include <fcntl.h>
#pragma comment (lib, "Ws2_32.lib")

int main()
{
	WSADATA wsaData;
	HOSTENT* lpHost;
	IN_ADDR inaddr;

	SOCKET s;
	SOCKADDR_IN addrin;
	SOCKADDR_IN from;
	int fromlen;
	int nRtn;
	u_short port;
	char szBuf[256], szIP[256];

	printf(" Enter Any Ephemeral Port Number(49152-65535): ");
	gets_s(szBuf);

	port = atoi(szBuf);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		perror(" WSAStartup Error\n");
		return -1;
	}

	printf(" wVersion: %d.%d\n",
		LOBYTE(wsaData.wVersion),
		HIBYTE(wsaData.wVersion));
	printf(" wHighVersion: %d.%d\n",
		LOBYTE(wsaData.wHighVersion),
		HIBYTE(wsaData.wHighVersion));
	printf(" szDescription: %s\n", wsaData.szDescription);
	printf(" szSystemStatus: %s\n", wsaData.szSystemStatus);
	printf(" iMaxSockets: %d\n", wsaData.iMaxSockets);
	printf(" iMaxUdpDg: %d\n", wsaData.iMaxUdpDg);
	printf(" WSAStartup Succeeded\n");
	gethostname(szBuf, (int)sizeof(szBuf));
	printf("\n Host Name: %s\n", szBuf);
	lpHost = gethostbyname(szBuf);
	memcpy(&inaddr, lpHost->h_addr_list[0], 4);
	strcpy_s(szIP, inet_ntoa(inaddr));
	printf(" IP Address: %s\n", szIP);
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror(" socket Failure\n");
		WSACleanup();
		return -2;
	}

	memset(&addrin, 0, sizeof(addrin));
	addrin.sin_port = htons(port);
	addrin.sin_family = AF_INET;
	addrin.sin_addr.s_addr = htonl(INADDR_ANY);

	nRtn = bind(s, (LPSOCKADDR)&addrin, (int)sizeof(addrin));
	if (nRtn == SOCKET_ERROR) {
		perror(" bind Error\n");
		closesocket(s);
		WSACleanup();
		return -3;
	}

	//	add
	//	ノンブロッキング処理
	u_long val = 1;
	ioctlsocket(s, FIONBIO, &val);

	while (1) {
		fromlen = (int)sizeof(from);
		nRtn = recvfrom(s,
			szBuf,
			(int)sizeof(szBuf) - 1,
			0,
			(SOCKADDR*)&from,
			&fromlen);

		//	add
		if (nRtn < 1) {
			if (errno == EAGAIN) {
				printf("MADA KONAI\n");
			}						
		}
		else {
			printf("%s>%s\n", inet_ntoa(from.sin_addr), szBuf);			
			szBuf[nRtn] = '\0';
		}

		//	delete
		/*if (nRtn == SOCKET_ERROR) {
			perror(" recvform Error\n");
			closesocket(s);
			WSACleanup();
			return -4;			
		}*/
		

		if (strcmp(szBuf, "end") == 0) {
			printf(" Terminate Server\n");
			break;
		}

		//	add
		//printf("%s>%s\n", inet_ntoa(from.sin_addr), szBuf);

	}
	closesocket(s);
	WSACleanup();
	printf(" Termination\n");

	return 0;
}