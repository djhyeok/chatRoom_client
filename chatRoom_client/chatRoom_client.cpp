#include "common.h"
#include "chatRoom_client.h"
#include "mySocket.h"
#include "windowControl.h"
#include <CommCtrl.h>
#include <commdlg.h>
#include <windows.h>
#include <stdio.h>

//윈도우 핸들
HWND g_hIPEdit, g_hPortEdit, g_hMsgEdit, g_hRoomNameEdit, g_hRoomList, g_hMsgList, g_hRoomNameStatic, g_hRoomNumStatic
, g_hConBtn, g_hNameEdit, g_hNameBtn, g_hSendBtn, g_hOutBtn, g_hselectFileBtn, g_hselectFileEdit;
//윈도우 ID enum
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

//연결상태 enum
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
	//소캣api의 리턴변수
	int nReturn;
	//에디트 박스 변수
	char strEdit[128], strEdit2[128];
	//리스트 박스 공통 변수
	char strList[128];
	//send 할 메시지 변수
	char strMsg[1024];
	//MessageBox return 변수
	int mReturn;
	//리스트뷰 환경 변수
	LVCOLUMN COL;
	LVITEM LI;
	int i;
	HDC hdc;
	PAINTSTRUCT ps;
	//Acc스레드 ID,Handle
	static DWORD ThreadID;
	static HANDLE hThread;
	//사진 파일 선택 필요 변수
	static char filePath[300] = "";
	OPENFILENAME ofn = { 0 };
	FILE* fp;
	int imgSize = 0;

	switch (iMessage) {
	case WM_CREATE:
		InitCommonControls();
		g_hWndMain = hWnd;
		nReturn = WSAStartup(WORD(2.0), &g_env.wsadata);

		////접속 버튼생성////
		g_hConBtn = CreateWindow("button", "서버접속", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 255, 24, 70, 50, hWnd, (HMENU)ID_CONBTN, g_hInst, NULL);

		////IP,PORT 에디트 윈도우 생성////
		g_hIPEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE, 100, 27, 150, 20, hWnd, (HMENU)ID_IPEDIT, g_hInst, NULL);
		g_hPortEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE, 100, 50, 150, 20, hWnd, (HMENU)ID_PORTEDIT, g_hInst, NULL);

		////채팅방 리스트뷰 생성////
		g_hRoomList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS, 40, 120, 250, 300, hWnd, (HMENU)ID_ROOMLIST, g_hInst, NULL);
		ListView_SetExtendedListViewStyle(g_hRoomList, LVS_EX_FULLROWSELECT);
		//채팅방 리스트뷰에 헤더 추가
		COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		COL.fmt = LVCFMT_LEFT;
		COL.cx = 150;
		COL.pszText = (LPSTR)" 방이름 ";
		COL.iSubItem = 0;
		ListView_InsertColumn(g_hRoomList, 0, &COL);

		COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		COL.fmt = LVCFMT_LEFT;
		COL.cx = 100;
		COL.pszText = (LPSTR)" 방 번호 ";
		COL.iSubItem = 1;
		ListView_InsertColumn(g_hRoomList, 1, &COL);

		////방이름,닉네임 에디트 윈도우 생성////
		g_hNameEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 425, 25, 150, 20, hWnd, (HMENU)ID_NAMEEDIT, g_hInst, NULL);
		g_hRoomNameEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 425, 50, 150, 20, hWnd, (HMENU)ID_ROOMNAMEEDIT, g_hInst, NULL);

		////닉네임 확정,채팅방 참여,생성 버튼 생성////
		g_hNameBtn = CreateWindow("button", "확정", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 580, 23, 50, 25, hWnd, (HMENU)ID_NAMEBTN, g_hInst, NULL);
		CreateWindow("button", "참여", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 580, 48, 50, 25, hWnd, (HMENU)ID_INBTN, g_hInst, NULL);
		CreateWindow("button", "생성", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 48, 50, 25, hWnd, (HMENU)ID_CREATEROOMBTN, g_hInst, NULL);

		////채팅방 이름 스태틱 생성////
		g_hRoomNumStatic = CreateWindow("static", "", WS_CHILD, 420, 100, 30, 20, hWnd, (HMENU)ID_ROOMNUMSTATIC, g_hInst, NULL);
		g_hRoomNameStatic = CreateWindow("static", "", WS_CHILD, 455, 100, 180, 20, hWnd, (HMENU)ID_ROOMNAMESTATIC, g_hInst, NULL);

		////채팅방 대화 리스트뷰 생성////
		g_hMsgList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_BORDER | LVS_REPORT, 420, 120, 250, 300, hWnd, (HMENU)ID_MSGLIST, g_hInst, NULL);
		ListView_SetExtendedListViewStyle(g_hMsgList, LVS_EX_FULLROWSELECT);

		//채팅방 리스트뷰에 헤더 추가
		COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		COL.fmt = LVCFMT_LEFT;
		COL.cx = 250;
		COL.pszText = (LPSTR)"";
		COL.iSubItem = 0;
		ListView_InsertColumn(g_hMsgList, 0, &COL);

		////채팅메시지 에디트 윈도우 생성////
		g_hMsgEdit = CreateWindow("edit", "", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE, 420, 411, 250, 20, hWnd, (HMENU)ID_MSGEDIT, g_hInst, NULL);

		////메시지 보내기 버튼 생성////
		g_hSendBtn = CreateWindow("button", "전송", WS_CHILD | BS_PUSHBUTTON, 670, 412, 40, 20, hWnd, (HMENU)ID_SENDBTN, g_hInst, NULL);

		//파일 선택 static,버튼 생성
		g_hselectFileEdit = CreateWindow("edit", "", WS_CHILD, 420, 435, 250, 20, hWnd, (HMENU)ID_FILEEDIT, g_hInst, NULL);
		g_hselectFileBtn = CreateWindow(TEXT("button"), TEXT("파일선택"), WS_CHILD | BS_PUSHBUTTON, 670, 436, 80, 20, hWnd, (HMENU)ID_FILEBTN, g_hInst, NULL);
		SendMessage(g_hselectFileEdit, EM_SETREADONLY, (WPARAM)TRUE, 0);

		////채팅방 나가기 버튼 생성////
		g_hOutBtn = CreateWindow("button", "나가기", WS_CHILD | BS_PUSHBUTTON, 620, 93, 50, 25, hWnd, (HMENU)ID_OUTBTN, g_hInst, NULL);

		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_NAMEBTN:					//유저이름 확정 버튼
			GetWindowText(g_hNameEdit, g_UserName, 20);
			//g_UserName 길이가 0인 경우
			if (lstrlen(g_UserName) == 0) {
				MessageBox(hWnd, "이름을 작성하세요", NULL, MB_OK);
			}
			//g_UserName 길이가 0이 아닌 경우
			else {
				wsprintf(strMsg, "%s 로 확정하시겠습니까?", g_UserName);
				mReturn = MessageBox(hWnd, strMsg, "확인", MB_OKCANCEL);
				if (mReturn == IDCANCEL) {
					SetWindowText(g_hNameEdit, "");
					lstrcpy(g_UserName, "");
				}
			}
			break;
		case ID_CONBTN:						//서버연결,해제 버튼
			if (g_isConnected == DISCONNECTED) {	//서버 연결시
				//g_UserName이 설정되었는지를 확인
				if (lstrlen(g_UserName) != 0) {	//g_UserName을 설정한 경우
					g_env.clientsock = socket(AF_INET, SOCK_STREAM, 0);

					//IP,Port 입력한 값 g_szIpAddress, 
					GetWindowText(g_hIPEdit, g_szlpAddress, 13);
					GetWindowText(g_hPortEdit, strEdit, 10);
					g_uPort = atoi(strEdit);

					g_env.addr_client.sin_family = AF_INET;
					g_env.addr_client.sin_addr.s_addr = inet_addr(g_szlpAddress);
					g_env.addr_client.sin_port = htons(g_uPort);

					nReturn = connect(g_env.clientsock, (sockaddr*)&g_env.addr_client, g_env.addrlen_clt);
					hThread = CreateThread(NULL, 0, RecvThreadFunc, &g_env.clientsock, 0, &ThreadID);
					if (!nReturn) {				//잘 연결된 경우
						//서버에 유저등록요청(R)	"R유저이름"
						wsprintf(strMsg, "R%s", g_UserName);
						nReturn = send(g_env.clientsock, strMsg, sizeof(strMsg), 0);
						//서버에 채팅방리스트요청(I)	"I"
						Sleep(5);
						nReturn = send(g_env.clientsock, "I", sizeof("I"), 0);
						//서버접속 -> 접속해제, IP,Port,g_UserName readOnly로 전환,확정버튼 Hide
						SetWindowText(g_hConBtn, "접속해제");
						SendMessage(g_hIPEdit, EM_SETREADONLY, (WPARAM)TRUE, 0);
						SendMessage(g_hPortEdit, EM_SETREADONLY, (WPARAM)TRUE, 0);
						SendMessage(g_hNameEdit, EM_SETREADONLY, (WPARAM)TRUE, 0);
						ShowWindow(g_hNameBtn, SW_HIDE);
						g_isConnected = CONNECTED;
						break;
					}
				}
				else {							//g_UserName을 설정하지 않은 경우
					MessageBox(hWnd, "먼저 닉네임을 설정하세요", NULL, MB_OK);
				}
			}
			else if (g_isConnected == CONNECTED) {							//접속 해제시
				//현재 채팅방에 들어가 있는 경우 나가기 요청
				if (isInRoom) {
					GetWindowText(g_hRoomNumStatic, strEdit, sizeof(strEdit));
					wsprintf(strMsg, "E%s", strEdit);
					nReturn = send(g_env.clientsock, strMsg, sizeof(strMsg), 0);
				}
				//서버접속 -> 접속해제, IP,Port,UserName readOnly로 전환,확정버튼 Hide
				ListView_DeleteAllItems(g_hRoomList);
				SetWindowText(g_hConBtn, "서버접속");
				SendMessage(g_hIPEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
				SendMessage(g_hPortEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
				SendMessage(g_hNameEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
				ShowWindow(g_hNameBtn, SW_SHOW);
				//서버에 연결 해제 요청 보냄
				shutdown(g_env.clientsock, 1);
			}
			break;
		case ID_CREATEROOMBTN:				//채팅방 생성 버튼
			//현재 채팅방에 들어가 있는 경우
			if (isInRoom) {
				MessageBox(hWnd, "현재 채팅방을 나온 후 이용하세요.", "알림", MB_OK);
			}
			//현재 채팅방에 들어가 있지 않은 경우
			else {
				//생성할 채팅방 이름 가져오기
				GetWindowText(g_hRoomNameEdit, strEdit, 20);
				if (lstrlen(strEdit) != 0) {				//채팅방 이름 길이가 0이 아닐 경우
					//서버로 C채팅방이름 send
					wsprintf(strMsg, "C%s", strEdit);
					nReturn = send(g_env.clientsock, strMsg, sizeof(strEdit), 0);
					//방이름 에디트 윈도우 초기화
					SetWindowText(g_hRoomNameEdit, "");
				}
			}
			break;
		case ID_INBTN:						//채팅방 참여 버튼
			//현재 채팅방에 들어가 있는 경우
			if (isInRoom) {
				MessageBox(hWnd, "현재 채팅방을 나온 후 이용하세요.", "알림", MB_OK);
			}
			//현재 채팅방에 들어가 있지 않은 경우
			else {
				int selIndex;
				//선택된 방이 있는지 확인
				selIndex = ListView_GetNextItem(g_hRoomList, -1, LVNI_ALL | LVNI_SELECTED);
				if (selIndex == -1) {
					MessageBox(hWnd, "채팅방 리스트에서 채팅방을 다시 선택 후 참여하세요.", "알림", MB_OK);
					break;
				}
				//선택된 방이 있는 경우 선택한 방과 현재 edit의 값이 같은지 확인
				GetWindowText(g_hRoomNameStatic, strEdit, sizeof(strEdit));
				GetWindowText(g_hRoomNameEdit, strEdit2, sizeof(strEdit2));
				//선택된 방과 현재 방이름 edit에 있는 문자열이 같은 경우
				if (lstrcmp(strEdit, strEdit2) == 0) {
					//서버로 "P방번호" send
					GetWindowText(g_hRoomNumStatic, strEdit, sizeof(strEdit));
					wsprintf(strMsg, "P%s", strEdit);
					nReturn = send(g_env.clientsock, strMsg, sizeof(strMsg), 0);
					//방이름 에디트 윈도우 초기화
					SetWindowText(g_hRoomNameEdit, "");
					isInRoom = TRUE;
				}
				else {
					SetWindowText(g_hRoomNameEdit, "");
					SetWindowText(g_hRoomNameStatic, "");
					SetWindowText(g_hRoomNumStatic, "");
					MessageBox(hWnd, "채팅방을 다시 선택하세요.", NULL, MB_OK);
				}
			}
			break;
		case ID_SENDBTN:
			InvalidateRect(hWnd, NULL, TRUE);
			//메시지 입력 edit 윈도우에서 입력한 메시지 가져다 "M메시지" 형태로 send
			GetWindowText(g_hMsgEdit, strEdit, sizeof(strEdit));
			if (lstrlen(strEdit) != 0) {
				wsprintf(strMsg, "M%s", strEdit);
				nReturn = send(g_env.clientsock, strMsg, sizeof(strMsg), 0);
				//메시지 보낸후 메시지 입력 창 초기화
				SetWindowText(g_hMsgEdit, "");
			}
			//파일선택이 되어있다면 파일 send
			GetWindowText(g_hselectFileEdit, strEdit, sizeof(strEdit));
			if (lstrlen(strEdit) != 0) {
				//선택된 이미지 파일을 binary로 읽음
				fp = fopen(strEdit, "rb");
				fseek(fp, 0, SEEK_END);
				//이미지파일 크기 측정
				imgSize = ftell(fp);
				fseek(fp, 0, SEEK_SET);

				//서버로 send  "F[이미지크기]"
				wsprintf(strMsg, "F%d", imgSize);
				nReturn = send(g_env.clientsock, strMsg, sizeof(strMsg), 0);

				i = 0;
				while (i < imgSize) {
					fread(strMsg, 1, 1024, fp);
					//서버로 이미지 send "이미지정보"
					send(g_env.clientsock, strMsg, 1024, 0);
					i += 1024;
				}
				fclose(fp);
				SetWindowText(g_hselectFileEdit, "");
			}
			break;
		case ID_FILEBTN:
			//ofn 세팅
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = "이미지 파일\0*.bmp;*.jpg;*.png\0모든 파일\0*.*\0";
			ofn.lpstrFile = filePath;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
			ofn.lpstrTitle = "이미지 선택";

			if (GetOpenFileName(&ofn)) {
				//선택한 파일 경로 edit 윈도우에 출력
				SetWindowText(g_hselectFileEdit, filePath);
			}
			else {
				MessageBox(hWnd, "파일선택에 실패하였습니다.", "파일 선택 실패", MB_OK);
			}
			break;
		case ID_OUTBTN:						//채팅방 나가기 버튼
			InvalidateRect(hWnd, NULL, TRUE);
			GetWindowText(g_hRoomNumStatic, strEdit, sizeof(strEdit));
			wsprintf(strMsg, "E%s", strEdit);
			nReturn = send(g_env.clientsock, strMsg, sizeof(strMsg), 0);
			//send가 성공적으로 완료된 경우
			if (nReturn < 0) {
				MessageBox(hWnd, "서버응답오류", "오류", MB_OK);
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
				if (nlv->uChanged == LVIF_STATE && nlv->uNewState == (LVIS_SELECTED | LVIS_FOCUSED)) {//리스트뷰에서 방 선택 이벤트 발생
					if (isInRoom == FALSE) {
						//채팅방 리스트뷰에서 채팅방 번호 edit와 static에 담기
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
		//방 이름 static 컨트롤 배경색 제거
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
		TextOut(hdc, 45, 100, TEXT("채팅방 목록"), lstrlen(TEXT("채팅방 목록")));
		TextOut(hdc, 360, 26, TEXT("닉네임"), lstrlen(TEXT("닉네임")));
		TextOut(hdc, 360, 51, TEXT("방 이름"), lstrlen(TEXT("방 이름")));
		//TextOut(hdc, 300, 400, TEXT("A"), lstrlen(TEXT("A")));
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		//현재 채팅방에 들어가 있는 경우 나가기 요청
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

