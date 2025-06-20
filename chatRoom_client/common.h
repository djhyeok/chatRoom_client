#pragma once
#ifndef COMMON_H
#define COMMON_H

#pragma warning(disable:4996)
#include <windows.h>
#include <stdio.h>

typedef struct env {
	WSADATA wsadata;
	SOCKET clientsock = 0;
	sockaddr_in addr_client;
	int addrlen_clt = sizeof(sockaddr);
	char buf[1024];
}ENV;

#endif