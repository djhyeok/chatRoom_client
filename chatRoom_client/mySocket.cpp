#include "common.h"
#include "mySocket.h"
#include "windowControl.h"
#include <commctrl.h>

/*
static��Ʈ�ѷ� �� ���� ������ �ѱ�µ� ������ư�� ������ �� �Ѿ�°� �����ؼ� ������ �� �������� �ʰ�
������� ������ �Ǿ��ִ� ��� ���߿� ������ Ŭ���̾�Ʈ�� �� ��� ����� ���� �� �ִ��� Ȯ��
*/
//������� enum
enum CONNECT {
	DISCONNECTED = 0,
	CONNECTED
};

extern ENV g_env;
extern int g_isConnected, isInRoom, g_Fnum;
extern HWND g_hRoomList, g_hWndMain, g_hConBtn, g_hIPEdit, g_hPortEdit, g_hNameEdit, g_hNameBtn, g_hRoomNameStatic, g_hRoomNumStatic, g_hMsgList;
extern char g_UserName[20];
/*
void cutMsg(char* msg,char* buffer): buffer���� �޽��� �߶� msg�� ����
*/
void cutMsg(char* msg, char* buffer) {
	int i = 1, j = 0;
	while (buffer[i] != '\0') {
		msg[j++] = buffer[i++];
	}
	msg[j] = '\0';
	return;
}

/*
void cutRoomInfo(char* name,char* num, char* buffer): buffer���� ���ȣ,���̸� �ڸ�
*/
void cutRoomInfo(char* name, char* num, char* buffer) {
	int i = 1, j = 0;
	while (buffer[i] != '|') {
		num[j++] = buffer[i++];
	}
	num[j] = '\0';
	j = 0;
	i++;
	while (buffer[i] != '\0') {
		name[j++] = buffer[i++];
	}
	name[j] = '\0';
	return;
}

/*
DWORD WINAPI RecvThreadFunc(LPVOID Param): ���ú� ������ �Լ�
*/
DWORD WINAPI RecvThreadFunc(LPVOID Param) {
	SOCKET* P = (SOCKET*)Param;
	LVITEM LI;
	int nReturn, i, j, index;
	char tempStr[1024], tempStr2[1024], tempMsg[1024];
	//�̹��� ���Ž� �ʿ亯��
	int imgSize, recvSize;
	FILE* fp;
	//�̹��� ��½� �ʿ亯��
	HDC hdc;
	HBITMAP hBitmap;

	for (;;) {
		nReturn = recv(*P, g_env.buf, 1024, 0);
		if (nReturn > 0) {					//�������� ���������� �����͸� �������
			//ä�ù� ���� ��û send �� server���� "c���ȣ|���̸�" recv
			if (g_env.buf[0] == 'c') {
				cutRoomInfo(tempStr, tempStr2, g_env.buf);
				//LV�� ���̸� ���� �߰�
				LI.mask = LVIF_TEXT;
				LI.iItem = 0;
				LI.iSubItem = 0;
				LI.pszText = tempStr;
				ListView_InsertItem(g_hRoomList, &LI);
				//LV�� ���ȣ ���� �߰�
				ListView_SetItemText(g_hRoomList, 0, 1, tempStr2);
			}
			//ä�ù� ���� �޽��� ���� "p���ȣ"
			else if (g_env.buf[0] == 'p') {
				cutMsg(tempStr2, g_env.buf);
				for (i = 0; i < ListView_GetItemCount(g_hRoomList); i++) {
					ListView_GetItemText(g_hRoomList, i, 1, tempStr, sizeof(tempStr));
					if (lstrcmp(tempStr, tempStr2) == 0) {
						SetWindowText(g_hRoomNumStatic, tempStr2);
						ListView_GetItemText(g_hRoomList, i, 0, tempStr, sizeof(tempStr));
						SetWindowText(g_hRoomNameStatic, tempStr);
						break;
					}
				}
				showChat();
				//������º��� TRUE�� ��ȯ
				isInRoom = TRUE;
			}
			else if (g_env.buf[0] == 'm') {
				cutMsg(tempStr, g_env.buf);
				//LV�� ���̸� ���� �߰�
				LI.mask = LVIF_TEXT;
				LI.iItem = ListView_GetItemCount(g_hMsgList) + 1;
				LI.iSubItem = 0;
				LI.pszText = tempStr;
				ListView_InsertItem(g_hMsgList, &LI);
			}
			//�������� ��û ���� "f�̹���ũ��"
			else if (g_env.buf[0] == 'f') {
				//����ũ�� tempStr�� ����
				cutMsg(tempMsg, g_env.buf);
				imgSize = atoi(tempMsg);
				//���� �󸶳� �޾Ҵ��� Ȯ���� ����
				recvSize = 0;
				//���ϸ� ����
				wsprintf(tempStr, "FILE%d.bmp", g_Fnum++);
				//������ ���� ����
				fp = fopen(tempStr, "wb");
				//�� ���������� recv���� ���ϳ��� 1024������ �ɰ��� �� ���Ͽ� ����
				while (recvSize < imgSize) {
					nReturn = recv(*P, tempMsg, 1024, 0);
					fwrite(tempMsg, 1, 1024, fp);
					if (nReturn <= 0) {
						break;
					}
					recvSize += nReturn;
				}
				//���ϴݱ�
				fclose(fp);

				hBitmap = (HBITMAP)LoadImageA(NULL, tempStr, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
				if (recvSize >= imgSize) {
					if (!hBitmap) {
						MessageBox(g_hWndMain, "BMP ���� �ε� ����!", "����", MB_OK);
					}
					else {
						hdc = GetDC(g_hWndMain);
						DrawBitmap(hdc, 300, 300, hBitmap);
						ReleaseDC(g_hWndMain, hdc);

					}
				}
			}
			//ä�ù� ������ �޽��� ���� "e"
			else if (g_env.buf[0] == 'e') {
				//ä�ù� ���� ���� ������ "e���ȣ"
				if (g_env.buf[1] != '\0') {
					cutMsg(tempStr, g_env.buf);
					GetWindowText(g_hRoomNumStatic, tempStr2, sizeof(tempStr2));
					//���� ������ ä�ù��� ������ ���
					if (lstrcmp(tempStr, tempStr2) == 0) {
						for (i = 0; i < ListView_GetItemCount(g_hRoomList); i++) {
							ListView_GetItemText(g_hRoomList, i, 1, tempStr, sizeof(tempStr));
							if (lstrcmp(tempStr, tempStr2) == 0) {
								ListView_DeleteItem(g_hRoomList, i);
								break;
							}
						}
						//������º��� FALSE �� ��ȯ
						isInRoom = FALSE;
						//static ��Ʈ�� �ʱ�ȭ �� hideChat���� ä�ð��� ������ HIDE
						ListView_DeleteAllItems(g_hMsgList);
						SetWindowText(g_hRoomNameStatic, "");
						SetWindowText(g_hRoomNumStatic, "");
						hideChat();
					}
					else {
						for (i = 0; i < ListView_GetItemCount(g_hRoomList); i++) {
							ListView_GetItemText(g_hRoomList, i, 1, tempStr2, sizeof(tempStr2));
							if (lstrcmp(tempStr, tempStr2) == 0) {
								ListView_DeleteItem(g_hRoomList, i);
								break;
							}
						}
					}
				}
				//�ܼ� ä�ù� ������
				else {
					//������º��� FALSE �� ��ȯ
					isInRoom = FALSE;
					//static ��Ʈ�� �ʱ�ȭ �� hideChat���� ä�ð��� ������ HIDE
					SetWindowText(g_hRoomNameStatic, "");
					SetWindowText(g_hRoomNumStatic, "");
					ListView_DeleteAllItems(g_hMsgList);
					hideChat();
				}
			}
		}
		//���� ����
		else if (nReturn == 0) {
			closesocket(*P);
			MessageBox(g_hWndMain, "���� ���� ����", "�˸�", MB_OK);
			//���� ������º��� disconnected�� ����
			g_isConnected = DISCONNECTED;
			//IP,Port readOnly ����
			SendMessage(g_hIPEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
			SendMessage(g_hPortEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
			//�г��� ������ �����ϰ�  editâ readOnly ����, Ȯ�� ��ư Show
			SendMessage(g_hNameEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
			ShowWindow(g_hNameBtn, SW_SHOW);
			isInRoom = FALSE;
			lstrcpy(g_UserName, "");
			ExitThread(0);
		}
		else {								//������������ ������ ������
			closesocket(*P);
			MessageBox(g_hWndMain, "���� ���� ����", "�˸�", MB_OK);
			//���� ������º��� disconnected�� ����
			g_isConnected = DISCONNECTED;
			//IP,Port readOnly ����
			SendMessage(g_hIPEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
			SendMessage(g_hPortEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
			//�г��� ������ �����ϰ�  editâ readOnly ����, Ȯ�� ��ư Show
			SendMessage(g_hNameEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
			ShowWindow(g_hNameBtn, SW_SHOW);
			isInRoom = FALSE;
			InitWindow();
			lstrcpy(g_UserName, "");
			ExitThread(0);
		}
	}
	return 0;
}