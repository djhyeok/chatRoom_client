#include "common.h"
#include "chatRoom_client.h"
#include "mySocket.h"
#include "windowControl.h"
#include <CommCtrl.h>
#include <commdlg.h>
#include <windows.h>
#include <stdio.h>

//������ �ڵ�
HWND g_hIPEdit, g_hPortEdit, g_hMsgEdit, g_hRoomNameEdit, g_hRoomList, g_hMsgList, g_hRoomNameStatic, g_hRoomNumStatic
, g_hConBtn, g_hNameEdit, g_hNameBtn, g_hSendBtn, g_hOutBtn, g_hselectFileBtn, g_hselectFileEdit;
//������ ID enum
enum WINDOW_ID {
	ID_IPEDIT = 1,
	ID_PORTEDIT,
	ID_ROOMLIST,
	ID_ROOMNAMEEDIT,
	ID_MSGLIST,
	ID_MSGEDIT,
	ID_CONBTN,
	ID_CREATEROOMBTN,
	ID_INBTN,
	ID_OUTBTN,
	ID_ROOMNAMESTATIC,
	ID_ROOMNUMSTATIC,
	ID_SENDBTN,
	ID_NAMEEDIT,
	ID_NAMEBTN,
	ID_FILEBTN,
	ID_FILEEDIT
};

//������� enum
enum CONNECT {
	DISCONNECTED = 0,
	CONNECTED,
};

char g_szlpAddress[17];
unsigned short g_uPort;

ENV g_env;
int g_isConnected = DISCONNECTED;
BOOL isInRoom = FALSE;
char g_UserName[20] = "";
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("Socket");
HWND g_hWndMain;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_CAPTION | WS_SYSMENU, 0, 0, 800, 500, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	//��Ĺapi�� ���Ϻ���
	int nReturn;
	//����Ʈ �ڽ� ����
	char strEdit[128], strEdit2[128];
	//����Ʈ �ڽ� ���� ����
	char strList[128];
	//send �� �޽��� ����
	char strMsg[1024];
	//MessageBox return ����
	int mReturn;
	//����Ʈ�� ȯ�� ����
	LVCOLUMN COL;
	LVITEM LI;
	int i;
	HDC hdc;
	PAINTSTRUCT ps;
	//Acc������ ID,Handle
	static DWORD ThreadID;
	static HANDLE hThread;
	//���� ���� ���� �ʿ� ����
	static char filePath[300] = "";
	OPENFILENAME ofn = { 0 };
	FILE* fp;
	int imgSize = 0;

	switch (iMessage) {
	case WM_CREATE:
		InitCommonControls();
		g_hWndMain = hWnd;
		nReturn = WSAStartup(WORD(2.0), &g_env.wsadata);

		////���� ��ư����////
		g_hConBtn = CreateWindow("button", "��������", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 255, 24, 70, 50, hWnd, (HMENU)ID_CONBTN, g_hInst, NULL);

		////IP,PORT ����Ʈ ������ ����////
		g_hIPEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE, 100, 27, 150, 20, hWnd, (HMENU)ID_IPEDIT, g_hInst, NULL);
		g_hPortEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE, 100, 50, 150, 20, hWnd, (HMENU)ID_PORTEDIT, g_hInst, NULL);

		////ä�ù� ����Ʈ�� ����////
		g_hRoomList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS, 40, 120, 250, 300, hWnd, (HMENU)ID_ROOMLIST, g_hInst, NULL);
		ListView_SetExtendedListViewStyle(g_hRoomList, LVS_EX_FULLROWSELECT);
		//ä�ù� ����Ʈ�信 ��� �߰�
		COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		COL.fmt = LVCFMT_LEFT;
		COL.cx = 150;
		COL.pszText = (LPSTR)" ���̸� ";
		COL.iSubItem = 0;
		ListView_InsertColumn(g_hRoomList, 0, &COL);

		COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		COL.fmt = LVCFMT_LEFT;
		COL.cx = 100;
		COL.pszText = (LPSTR)" �� ��ȣ ";
		COL.iSubItem = 1;
		ListView_InsertColumn(g_hRoomList, 1, &COL);

		////���̸�,�г��� ����Ʈ ������ ����////
		g_hNameEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 425, 25, 150, 20, hWnd, (HMENU)ID_NAMEEDIT, g_hInst, NULL);
		g_hRoomNameEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 425, 50, 150, 20, hWnd, (HMENU)ID_ROOMNAMEEDIT, g_hInst, NULL);

		////�г��� Ȯ��,ä�ù� ����,���� ��ư ����////
		g_hNameBtn = CreateWindow("button", "Ȯ��", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 580, 23, 50, 25, hWnd, (HMENU)ID_NAMEBTN, g_hInst, NULL);
		CreateWindow("button", "����", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 580, 48, 50, 25, hWnd, (HMENU)ID_INBTN, g_hInst, NULL);
		CreateWindow("button", "����", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 48, 50, 25, hWnd, (HMENU)ID_CREATEROOMBTN, g_hInst, NULL);

		////ä�ù� �̸� ����ƽ ����////
		g_hRoomNumStatic = CreateWindow("static", "", WS_CHILD, 420, 100, 30, 20, hWnd, (HMENU)ID_ROOMNUMSTATIC, g_hInst, NULL);
		g_hRoomNameStatic = CreateWindow("static", "", WS_CHILD, 455, 100, 180, 20, hWnd, (HMENU)ID_ROOMNAMESTATIC, g_hInst, NULL);

		////ä�ù� ��ȭ ����Ʈ�� ����////
		g_hMsgList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_BORDER | LVS_REPORT, 420, 120, 250, 300, hWnd, (HMENU)ID_MSGLIST, g_hInst, NULL);
		ListView_SetExtendedListViewStyle(g_hMsgList, LVS_EX_FULLROWSELECT);

		//ä�ù� ����Ʈ�信 ��� �߰�
		COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		COL.fmt = LVCFMT_LEFT;
		COL.cx = 250;
		COL.pszText = (LPSTR)"";
		COL.iSubItem = 0;
		ListView_InsertColumn(g_hMsgList, 0, &COL);

		////ä�ø޽��� ����Ʈ ������ ����////
		g_hMsgEdit = CreateWindow("edit", "", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE, 420, 411, 250, 20, hWnd, (HMENU)ID_MSGEDIT, g_hInst, NULL);

		////�޽��� ������ ��ư ����////
		g_hSendBtn = CreateWindow("button", "����", WS_CHILD | BS_PUSHBUTTON, 670, 412, 40, 20, hWnd, (HMENU)ID_SENDBTN, g_hInst, NULL);

		//���� ���� static,��ư ����
		g_hselectFileEdit = CreateWindow("edit", "", WS_CHILD, 420, 435, 250, 20, hWnd, (HMENU)ID_FILEEDIT, g_hInst, NULL);
		g_hselectFileBtn = CreateWindow(TEXT("button"), TEXT("���ϼ���"), WS_CHILD | BS_PUSHBUTTON, 670, 436, 80, 20, hWnd, (HMENU)ID_FILEBTN, g_hInst, NULL);
		SendMessage(g_hselectFileEdit, EM_SETREADONLY, (WPARAM)TRUE, 0);

		////ä�ù� ������ ��ư ����////
		g_hOutBtn = CreateWindow("button", "������", WS_CHILD | BS_PUSHBUTTON, 620, 93, 50, 25, hWnd, (HMENU)ID_OUTBTN, g_hInst, NULL);

		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_NAMEBTN:					//�����̸� Ȯ�� ��ư
			GetWindowText(g_hNameEdit, g_UserName, 20);
			//g_UserName ���̰� 0�� ���
			if (lstrlen(g_UserName) == 0) {
				MessageBox(hWnd, "�̸��� �ۼ��ϼ���", NULL, MB_OK);
			}
			//g_UserName ���̰� 0�� �ƴ� ���
			else {
				wsprintf(strMsg, "%s �� Ȯ���Ͻðڽ��ϱ�?", g_UserName);
				mReturn = MessageBox(hWnd, strMsg, "Ȯ��", MB_OKCANCEL);
				if (mReturn == IDCANCEL) {
					SetWindowText(g_hNameEdit, "");
					lstrcpy(g_UserName, "");
				}
			}
			break;
		case ID_CONBTN:						//��������,���� ��ư
			if (g_isConnected == DISCONNECTED) {	//���� �����
				//g_UserName�� �����Ǿ������� Ȯ��
				if (lstrlen(g_UserName) != 0) {	//g_UserName�� ������ ���
					g_env.clientsock = socket(AF_INET, SOCK_STREAM, 0);

					//IP,Port �Է��� �� g_szIpAddress, 
					GetWindowText(g_hIPEdit, g_szlpAddress, 13);
					GetWindowText(g_hPortEdit, strEdit, 10);
					g_uPort = atoi(strEdit);

					g_env.addr_client.sin_family = AF_INET;
					g_env.addr_client.sin_addr.s_addr = inet_addr(g_szlpAddress);
					g_env.addr_client.sin_port = htons(g_uPort);

					nReturn = connect(g_env.clientsock, (sockaddr*)&g_env.addr_client, g_env.addrlen_clt);
					hThread = CreateThread(NULL, 0, RecvThreadFunc, &g_env.clientsock, 0, &ThreadID);
					if (!nReturn) {				//�� ����� ���
						//������ ������Ͽ�û(R)	"R�����̸�"
						wsprintf(strMsg, "R%s", g_UserName);
						nReturn = send(g_env.clientsock, strMsg, sizeof(strMsg), 0);
						//������ ä�ù渮��Ʈ��û(I)	"I"
						Sleep(5);
						nReturn = send(g_env.clientsock, "I", sizeof("I"), 0);
						//�������� -> ��������, IP,Port,g_UserName readOnly�� ��ȯ,Ȯ����ư Hide
						SetWindowText(g_hConBtn, "��������");
						SendMessage(g_hIPEdit, EM_SETREADONLY, (WPARAM)TRUE, 0);
						SendMessage(g_hPortEdit, EM_SETREADONLY, (WPARAM)TRUE, 0);
						SendMessage(g_hNameEdit, EM_SETREADONLY, (WPARAM)TRUE, 0);
						ShowWindow(g_hNameBtn, SW_HIDE);
						g_isConnected = CONNECTED;
						break;
					}
				}
				else {							//g_UserName�� �������� ���� ���
					MessageBox(hWnd, "���� �г����� �����ϼ���", NULL, MB_OK);
				}
			}
			else if (g_isConnected == CONNECTED) {							//���� ������
				//���� ä�ù濡 �� �ִ� ��� ������ ��û
				if (isInRoom) {
					GetWindowText(g_hRoomNumStatic, strEdit, sizeof(strEdit));
					wsprintf(strMsg, "E%s", strEdit);
					nReturn = send(g_env.clientsock, strMsg, sizeof(strMsg), 0);
				}
				//�������� -> ��������, IP,Port,UserName readOnly�� ��ȯ,Ȯ����ư Hide
				ListView_DeleteAllItems(g_hRoomList);
				SetWindowText(g_hConBtn, "��������");
				SendMessage(g_hIPEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
				SendMessage(g_hPortEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
				SendMessage(g_hNameEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
				ShowWindow(g_hNameBtn, SW_SHOW);
				//������ ���� ���� ��û ����
				shutdown(g_env.clientsock, 1);
			}
			break;
		case ID_CREATEROOMBTN:				//ä�ù� ���� ��ư
			//���� ä�ù濡 �� �ִ� ���
			if (isInRoom) {
				MessageBox(hWnd, "���� ä�ù��� ���� �� �̿��ϼ���.", "�˸�", MB_OK);
			}
			//���� ä�ù濡 �� ���� ���� ���
			else {
				//������ ä�ù� �̸� ��������
				GetWindowText(g_hRoomNameEdit, strEdit, 20);
				if (lstrlen(strEdit) != 0) {				//ä�ù� �̸� ���̰� 0�� �ƴ� ���
					//������ Cä�ù��̸� send
					wsprintf(strMsg, "C%s", strEdit);
					nReturn = send(g_env.clientsock, strMsg, sizeof(strEdit), 0);
					//���̸� ����Ʈ ������ �ʱ�ȭ
					SetWindowText(g_hRoomNameEdit, "");
				}
			}
			break;
		case ID_INBTN:						//ä�ù� ���� ��ư
			//���� ä�ù濡 �� �ִ� ���
			if (isInRoom) {
				MessageBox(hWnd, "���� ä�ù��� ���� �� �̿��ϼ���.", "�˸�", MB_OK);
			}
			//���� ä�ù濡 �� ���� ���� ���
			else {
				int selIndex;
				//���õ� ���� �ִ��� Ȯ��
				selIndex = ListView_GetNextItem(g_hRoomList, -1, LVNI_ALL | LVNI_SELECTED);
				if (selIndex == -1) {
					MessageBox(hWnd, "ä�ù� ����Ʈ���� ä�ù��� �ٽ� ���� �� �����ϼ���.", "�˸�", MB_OK);
					break;
				}
				//���õ� ���� �ִ� ��� ������ ��� ���� edit�� ���� ������ Ȯ��
				GetWindowText(g_hRoomNameStatic, strEdit, sizeof(strEdit));
				GetWindowText(g_hRoomNameEdit, strEdit2, sizeof(strEdit2));
				//���õ� ��� ���� ���̸� edit�� �ִ� ���ڿ��� ���� ���
				if (lstrcmp(strEdit, strEdit2) == 0) {
					//������ "P���ȣ" send
					GetWindowText(g_hRoomNumStatic, strEdit, sizeof(strEdit));
					wsprintf(strMsg, "P%s", strEdit);
					nReturn = send(g_env.clientsock, strMsg, sizeof(strMsg), 0);
					//���̸� ����Ʈ ������ �ʱ�ȭ
					SetWindowText(g_hRoomNameEdit, "");
					isInRoom = TRUE;
				}
				else {
					SetWindowText(g_hRoomNameEdit, "");
					SetWindowText(g_hRoomNameStatic, "");
					SetWindowText(g_hRoomNumStatic, "");
					MessageBox(hWnd, "ä�ù��� �ٽ� �����ϼ���.", NULL, MB_OK);
				}
			}
			break;
		case ID_SENDBTN:
			InvalidateRect(hWnd, NULL, TRUE);
			//�޽��� �Է� edit �����쿡�� �Է��� �޽��� ������ "M�޽���" ���·� send
			GetWindowText(g_hMsgEdit, strEdit, sizeof(strEdit));
			if (lstrlen(strEdit) != 0) {
				wsprintf(strMsg, "M%s", strEdit);
				nReturn = send(g_env.clientsock, strMsg, sizeof(strMsg), 0);
				//�޽��� ������ �޽��� �Է� â �ʱ�ȭ
				SetWindowText(g_hMsgEdit, "");
			}
			//���ϼ����� �Ǿ��ִٸ� ���� send
			GetWindowText(g_hselectFileEdit, strEdit, sizeof(strEdit));
			if (lstrlen(strEdit) != 0) {
				//���õ� �̹��� ������ binary�� ����
				fp = fopen(strEdit, "rb");
				fseek(fp, 0, SEEK_END);
				//�̹������� ũ�� ����
				imgSize = ftell(fp);
				fseek(fp, 0, SEEK_SET);

				//������ send  "F[�̹���ũ��]"
				wsprintf(strMsg, "F%d", imgSize);
				nReturn = send(g_env.clientsock, strMsg, sizeof(strMsg), 0);

				i = 0;
				while (i < imgSize) {
					fread(strMsg, 1, 1024, fp);
					//������ �̹��� send "�̹�������"
					send(g_env.clientsock, strMsg, 1024, 0);
					i += 1024;
				}
				fclose(fp);
				SetWindowText(g_hselectFileEdit, "");
			}
			break;
		case ID_FILEBTN:
			//ofn ����
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = "�̹��� ����\0*.bmp;*.jpg;*.png\0��� ����\0*.*\0";
			ofn.lpstrFile = filePath;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
			ofn.lpstrTitle = "�̹��� ����";

			if (GetOpenFileName(&ofn)) {
				//������ ���� ��� edit �����쿡 ���
				SetWindowText(g_hselectFileEdit, filePath);
			}
			else {
				MessageBox(hWnd, "���ϼ��ÿ� �����Ͽ����ϴ�.", "���� ���� ����", MB_OK);
			}
			break;
		case ID_OUTBTN:						//ä�ù� ������ ��ư
			InvalidateRect(hWnd, NULL, TRUE);
			GetWindowText(g_hRoomNumStatic, strEdit, sizeof(strEdit));
			wsprintf(strMsg, "E%s", strEdit);
			nReturn = send(g_env.clientsock, strMsg, sizeof(strMsg), 0);
			//send�� ���������� �Ϸ�� ���
			if (nReturn < 0) {
				MessageBox(hWnd, "�����������", "����", MB_OK);
			}
			break;
		}
		return 0;
	case WM_NOTIFY:
		LPNMHDR hdr;
		LPNMLISTVIEW nlv;
		hdr = (LPNMHDR)lParam;
		nlv = (LPNMLISTVIEW)lParam;
		if (hdr->hwndFrom == g_hRoomList) {
			switch (hdr->code) {
			case LVN_ITEMCHANGED:
				if (nlv->uChanged == LVIF_STATE && nlv->uNewState == (LVIS_SELECTED | LVIS_FOCUSED)) {//����Ʈ�信�� �� ���� �̺�Ʈ �߻�
					if (isInRoom == FALSE) {
						//ä�ù� ����Ʈ�信�� ä�ù� ��ȣ edit�� static�� ���
						ListView_GetItemText(g_hRoomList, nlv->iItem, 0, strEdit, sizeof(strEdit));
						SetWindowText(g_hRoomNameStatic, strEdit);
						SetWindowText(g_hRoomNameEdit, strEdit);
						ListView_GetItemText(g_hRoomList, nlv->iItem, 1, strEdit, sizeof(strEdit));
						SetWindowText(g_hRoomNumStatic, strEdit);
					}
				}
				return TRUE;

			}
		}

		return 0;
	case WM_CTLCOLORSTATIC:
	{
		//�� �̸� static ��Ʈ�� ���� ����
		if ((HWND)lParam == g_hRoomNameStatic || g_hRoomNumStatic) {
			SetBkMode((HDC)wParam, TRANSPARENT);
			return (LRESULT)GetStockObject(NULL_BRUSH);
		}
		break;
	}
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		Rectangle(hdc, 47, 20, 330, 78);
		TextOut(hdc, 55, 27, TEXT("IP"), lstrlen(TEXT("IP")));
		TextOut(hdc, 55, 52, TEXT("PORT"), lstrlen(TEXT("PORT")));
		TextOut(hdc, 45, 100, TEXT("ä�ù� ���"), lstrlen(TEXT("ä�ù� ���")));
		TextOut(hdc, 360, 26, TEXT("�г���"), lstrlen(TEXT("�г���")));
		TextOut(hdc, 360, 51, TEXT("�� �̸�"), lstrlen(TEXT("�� �̸�")));
		//TextOut(hdc, 300, 400, TEXT("A"), lstrlen(TEXT("A")));
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		//���� ä�ù濡 �� �ִ� ��� ������ ��û
		if (isInRoom) {
			GetWindowText(g_hRoomNumStatic, strEdit, sizeof(strEdit));
			wsprintf(strMsg, "E%s", strEdit);
			nReturn = send(g_env.clientsock, strMsg, sizeof(strMsg), 0);
		}
		shutdown(g_env.clientsock, 1);
		closesocket(g_env.clientsock);

		WSACleanup();
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

