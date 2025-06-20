#include <windows.h>
#include <CommCtrl.h>
#include "windowControl.h"

extern HWND g_hConBtn, g_hNameEdit, g_hNameBtn, g_hRoomNameEdit, g_hIPEdit, g_hRoomList, g_hPortEdit, g_hOutBtn, g_hSendBtn, g_hMsgEdit, g_hMsgList, g_hRoomNameStatic, g_hRoomNumStatic, g_hselectFileEdit, g_hselectFileBtn;
extern int g_isConnected;
/*
void showChat(): 채팅방 윈도우들 보이게 설정
*/
void showChat() {
	ShowWindow(g_hRoomNumStatic, SW_SHOW);
	ShowWindow(g_hRoomNameStatic, SW_SHOW);
	ShowWindow(g_hMsgList, SW_SHOW);
	ShowWindow(g_hMsgEdit, SW_SHOW);
	ShowWindow(g_hSendBtn, SW_SHOW);
	ShowWindow(g_hOutBtn, SW_SHOW);
	ShowWindow(g_hselectFileEdit, SW_SHOW);
	ShowWindow(g_hselectFileBtn, SW_SHOW);
	return;
}

/*
void hideChat(): 채팅방 윈도우들 안보이게 설정
*/
void hideChat() {
	SetWindowText(g_hRoomNumStatic, "");
	SetWindowText(g_hRoomNameStatic, "");
	ShowWindow(g_hRoomNumStatic, SW_HIDE);
	ShowWindow(g_hRoomNameStatic, SW_HIDE);
	ListView_DeleteAllItems(g_hMsgList);
	ShowWindow(g_hMsgList, SW_HIDE);
	SetWindowText(g_hMsgEdit, "");
	ShowWindow(g_hMsgEdit, SW_HIDE);
	ShowWindow(g_hSendBtn, SW_HIDE);
	ShowWindow(g_hOutBtn, SW_HIDE);
	SetWindowText(g_hselectFileEdit, "");
	ShowWindow(g_hselectFileEdit, SW_HIDE);
	ShowWindow(g_hselectFileBtn, SW_HIDE);
	return;
}

/*
void InitWindow(): 윈도우를 초기상태로 돌림
*/
void InitWindow() {
	g_isConnected = 0;
	SetWindowText(g_hConBtn, "서버접속");
	SetWindowText(g_hIPEdit, "");
	SetWindowText(g_hPortEdit, "");
	SendMessage(g_hIPEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
	SendMessage(g_hPortEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
	SetWindowText(g_hNameEdit, "");
	SendMessage(g_hNameEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
	ListView_DeleteAllItems(g_hRoomList);
	ShowWindow(g_hNameBtn, SW_SHOW);
	SetWindowText(g_hRoomNameEdit, "");

	hideChat();
	return;
}

/*
void DrawBitmap(HDC hdc, int x,int y, HBITMAP hBit): hBit 비트맵을 hdc에 x,y위치에 출력
*/
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit) {
	HDC MemDC;
	HBITMAP OldBitmap;
	int bx, by;
	BITMAP bit;

	MemDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

	GetObject(hBit, sizeof(BITMAP), &bit);
	bx = bit.bmWidth;
	by = bit.bmHeight;

	BitBlt(hdc, x, y, bx, by, MemDC, 0, 0, SRCCOPY);

	SelectObject(MemDC, OldBitmap);
	DeleteDC(MemDC);
}