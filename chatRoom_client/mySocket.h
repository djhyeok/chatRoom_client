#pragma once
#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <windows.h>

void cutMsg(char*, char*);
void cutRoomInfo(char*, char*, char*);
DWORD WINAPI RecvThreadFunc(LPVOID);

#endif