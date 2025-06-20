#pragma once
#ifndef CHATROOM_CLIENT_H
#define CHATROOM_CLIENT_H

#include "common.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI ThreadFunc(LPVOID);

#endif