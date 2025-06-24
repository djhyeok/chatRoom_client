#include "common.h"
#include "mySocket.h"
#include "windowControl.h"
#include <commctrl.h>

/*
static컨트롤로 방 선택 정보를 넘기는데 참여버튼을 눌렀을 때 넘어가는거 제어해서 생성할 때 문제되지 않게
방생성이 여러개 되어있는 경우 나중에 접속한 클라이언트가 방 목록 제대로 받을 수 있는지 확인
*/
//연결상태 enum
enum CONNECT {
	DISCONNECTED = 0,
	CONNECTED
};

extern ENV g_env;
extern int g_isConnected, isInRoom, g_Fnum;
extern HWND g_hRoomList, g_hWndMain, g_hConBtn, g_hIPEdit, g_hPortEdit, g_hNameEdit, g_hNameBtn, g_hRoomNameStatic, g_hRoomNumStatic, g_hMsgList;
extern char g_UserName[20];
/*
void cutMsg(char* msg,char* buffer): buffer에서 메시지 잘라서 msg에 담음
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
void cutRoomInfo(char* name,char* num, char* buffer): buffer에서 방번호,방이름 자름
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
DWORD WINAPI RecvThreadFunc(LPVOID Param): 리시브 스레드 함수
*/
DWORD WINAPI RecvThreadFunc(LPVOID Param) {
	SOCKET* P = (SOCKET*)Param;
	LVITEM LI;
	int nReturn, i, j, index;
	char tempStr[1024], tempStr2[1024], tempMsg[1024];
	//이미지 수신시 필요변수
	int imgSize, recvSize;
	FILE* fp;
	//이미지 출력시 필요변수
	HDC hdc;
	HBITMAP hBitmap;

	for (;;) {
		nReturn = recv(*P, g_env.buf, 1024, 0);
		if (nReturn > 0) {					//서버에서 정상적으로 데이터를 받은경우
			//채팅방 생성 요청 send 후 server에서 "c방번호|방이름" recv
			if (g_env.buf[0] == 'c') {
				cutRoomInfo(tempStr, tempStr2, g_env.buf);
				//LV에 방이름 정보 추가
				LI.mask = LVIF_TEXT;
				LI.iItem = 0;
				LI.iSubItem = 0;
				LI.pszText = tempStr;
				ListView_InsertItem(g_hRoomList, &LI);
				//LV에 방번호 정보 추가
				ListView_SetItemText(g_hRoomList, 0, 1, tempStr2);
			}
			//채팅방 참여 메시지 수신 "p방번호"
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
				//입장상태변수 TRUE로 전환
				isInRoom = TRUE;
			}
			else if (g_env.buf[0] == 'm') {
				cutMsg(tempStr, g_env.buf);
				//LV에 방이름 정보 추가
				LI.mask = LVIF_TEXT;
				LI.iItem = ListView_GetItemCount(g_hMsgList) + 1;
				LI.iSubItem = 0;
				LI.pszText = tempStr;
				ListView_InsertItem(g_hMsgList, &LI);
			}
			//파일전송 요청 수신 "f이미지크기"
			else if (g_env.buf[0] == 'f') {
				//파일크기 tempStr에 담음
				cutMsg(tempMsg, g_env.buf);
				imgSize = atoi(tempMsg);
				//현재 얼마나 받았는지 확인할 변수
				recvSize = 0;
				//파일명 설정
				wsprintf(tempStr, "FILE%d.bmp", g_Fnum++);
				//저장할 파일 생성
				fp = fopen(tempStr, "wb");
				//다 받을때까지 recv받은 파일내용 1024단위로 쪼개서 새 파일에 저장
				while (recvSize < imgSize) {
					nReturn = recv(*P, tempMsg, 1024, 0);
					fwrite(tempMsg, 1, 1024, fp);
					if (nReturn <= 0) {
						break;
					}
					recvSize += nReturn;
				}
				//파일닫기
				fclose(fp);

				hBitmap = (HBITMAP)LoadImageA(NULL, tempStr, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
				if (recvSize >= imgSize) {
					if (!hBitmap) {
						MessageBox(g_hWndMain, "BMP 파일 로드 실패!", "오류", MB_OK);
					}
					else {
						hdc = GetDC(g_hWndMain);
						DrawBitmap(hdc, 300, 300, hBitmap);
						ReleaseDC(g_hWndMain, hdc);

					}
				}
			}
			//채팅방 나가기 메시지 수신 "e"
			else if (g_env.buf[0] == 'e') {
				//채팅방 삭제 포함 나가기 "e방번호"
				if (g_env.buf[1] != '\0') {
					cutMsg(tempStr, g_env.buf);
					GetWindowText(g_hRoomNumStatic, tempStr2, sizeof(tempStr2));
					//현재 본인의 채팅방을 나가는 경우
					if (lstrcmp(tempStr, tempStr2) == 0) {
						for (i = 0; i < ListView_GetItemCount(g_hRoomList); i++) {
							ListView_GetItemText(g_hRoomList, i, 1, tempStr, sizeof(tempStr));
							if (lstrcmp(tempStr, tempStr2) == 0) {
								ListView_DeleteItem(g_hRoomList, i);
								break;
							}
						}
						//입장상태변수 FALSE 로 전환
						isInRoom = FALSE;
						//static 컨트롤 초기화 후 hideChat으로 채팅관련 윈도우 HIDE
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
				//단순 채팅방 나가기
				else {
					//입장상태변수 FALSE 로 전환
					isInRoom = FALSE;
					//static 컨트롤 초기화 후 hideChat으로 채팅관련 윈도우 HIDE
					SetWindowText(g_hRoomNameStatic, "");
					SetWindowText(g_hRoomNumStatic, "");
					ListView_DeleteAllItems(g_hMsgList);
					hideChat();
				}
			}
		}
		//서버 종료
		else if (nReturn == 0) {
			closesocket(*P);
			MessageBox(g_hWndMain, "서버 연결 해제", "알림", MB_OK);
			//서버 연결상태변수 disconnected로 변경
			g_isConnected = DISCONNECTED;
			//IP,Port readOnly 해제
			SendMessage(g_hIPEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
			SendMessage(g_hPortEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
			//닉네임 변경이 가능하게  edit창 readOnly 해제, 확정 버튼 Show
			SendMessage(g_hNameEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
			ShowWindow(g_hNameBtn, SW_SHOW);
			isInRoom = FALSE;
			lstrcpy(g_UserName, "");
			ExitThread(0);
		}
		else {								//비정상적으로 연결이 끊긴경우
			closesocket(*P);
			MessageBox(g_hWndMain, "서버 연결 해제", "알림", MB_OK);
			//서버 연결상태변수 disconnected로 변경
			g_isConnected = DISCONNECTED;
			//IP,Port readOnly 해제
			SendMessage(g_hIPEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
			SendMessage(g_hPortEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
			//닉네임 변경이 가능하게  edit창 readOnly 해제, 확정 버튼 Show
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