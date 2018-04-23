// plantsVSmonster.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "plantsVSmonster.h"
#include <list>
#include <windowsx.h>
#include <mmsystem.h>

using namespace std;
#pragma comment(lib,"msimg32.lib")
#pragma comment(lib,"winmm.lib")
#define MAX_LOADSTRING 100

// ȫ�ֱ���: 
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

// �˴���ģ���а����ĺ�����ǰ������: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

//�궨��
#define START_X_POS 60			//���ӵ���ʼX����
#define START_Y_POS 90			//���ӵ���ʼY����
#define LATTICE_WIDTH 80		//ÿ�����ӵĿ�
#define LATTICE_HEIGHT 110      //ÿ�����ӵĸ�
#define MONSTER_WIDTH 55		//�ֵĿ�
#define MONSTER_HEIGHT 106      //�ֵĸ�
#define GunFireTimeElapce 1500  //ǹ�����ӵ���ʱ���� 
#define BULLETATTACKVALUE 8		//�ӵ�������
#define GUN_PRICE 70	//ǹ�ļ۸�
#define FlOWER_PRICE 100	//���ļ۸�
#define FlOWER_CREATEGOLDTIME 6000	//����ô���һ����ң�30��
#define HPITEM_H	10	//Ѫ����
#define HPITEM_W	50	//Ѫ�����
#define MAXMONSTERSIZE 25	//��ʬ�������

//��������
enum TimerType
{
	Timer_Draw,
	Timer_CreateMonster,
	Timer_CreateBullet,
	Timer_CreateGold
};

struct stObject
{
	tagPOINT ptPost;	//��������꣬���ڻ���ǹ�Ǹ������꣬�ӵ��͹���ʵ������
	int hpValue;		//����Ѫ��
	UINT speed;			//�ٶ�
	UINT picIndex;		//��ʬ��ʾ�ĵ�ǰͼƬ
	//ǹ�ͻ�
	DWORD lastActionTime;	//����ǹ�ϴη����ӵ���ʱ�䣬���ڻ��ϴι�ʵ����ʱ��
};

//����
void DrawAll();		//������������
void PreLoadBitmap();	//Ԥ����������Դ
void CreateMonster();	//������ʬ
void CreateBullet();	//�����ӵ�
void CheckCollise();	//�����ײ
void OnLButtonDown(LPARAM lParam);	//������
void OnLButtonUp(LPARAM lParam);  //�������
void CreateGold();	//�������
bool IsInRect(tagPOINT pt, RECT rect);	//�ж�һ�����Ƿ���һ������������
bool CheckGameOver();		//�����Ϸ�Ƿ����
bool CheckGameSucceed();	//����Ƿ���Ϸ����
list<stObject> g_listFlower;	//��
list<stObject> g_listGun;		//ǹ
list<stObject> g_listMonster;	//��
list<stObject> g_listBullet;	//�ӵ�

//����
HWND g_hWnd = NULL;	//���ھ��
HBITMAP g_hBitmapBack = NULL;	//����
HBITMAP g_hBitmapGun = NULL;	//ǹ
HBITMAP g_hBitmapFlower = NULL;	//��
HBITMAP g_hBitmapBullet = NULL;	//�ӵ�
HBITMAP g_hBitmapMonster = NULL;//����
int g_mouseItemType = 1;		//���Ϊ1���ǻ������Ϊ2����ǹ
bool g_bMouseHaveItem= false;		//��ʶ����Ƿ�������
tagPOINT g_mouseItemCenterPos;	//���λ��
int g_money = 600;					//�����
static int g_MonsterCount = 0;		//��ʬĿǰ����
static int g_MonsterCount2 = MAXMONSTERSIZE;	//��ʬĿǰ����

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PLANTSVSMONSTER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PLANTSVSMONSTER));

	// ����Ϣѭ��: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PLANTSVSMONSTER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCE(IDC_PLANTSVSMONSTER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME &
	   ~WS_MAXIMIZEBOX,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   g_hWnd = hWnd;
   PreLoadBitmap();	//������Դ

   stObject obj;
   obj.ptPost.x = 0;
   obj.ptPost.y = 0;
   obj.lastActionTime = GetTickCount();
   g_listFlower.push_back(obj);
   /*obj.ptPost.x = 2;
   obj.ptPost.y = 1;
   g_listFlower.push_back(obj);
   obj.ptPost.x = 300;
   obj.ptPost.y = 420;
   obj.speed = 5;
   g_listBullet.push_back(obj);
   obj.ptPost.x = 0;
   obj.ptPost.y = 2;
   obj.lastActionTime = GetTickCount();
   g_listGun.push_back(obj);

   obj.picIndex = 5;
   obj.ptPost.x = 200;
   obj.ptPost.y = 300;
   obj.speed = 3;
   g_listMonster.push_back(obj);*/

   MoveWindow(hWnd, 500, 200, 900+16, 506+38, FALSE);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   SetTimer(hWnd, Timer_Draw, 50, NULL);
   SetTimer(hWnd, Timer_CreateMonster, 3000, NULL);
   SetTimer(hWnd, Timer_CreateBullet, 800, NULL);
   SetTimer(hWnd, Timer_CreateGold, 1000, NULL);
   srand(GetTickCount());
 //  16  38
//   RECT rectWnd;
 //  RECT rectClient;
//   GetWindowRect(hWnd, &rectWnd);
 //  GetClientRect(hWnd, &rectClient);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		{
	//		TCHAR path[MAX_PATH];
	//		GetCurrentDirectory(MAX_PATH, path);
	//		wsprintf(path, _T("%s\\..\\Debug\\music\\ֲ���ս��ʬ��������.mp3"), path);
			MCI_OPEN_PARMS m_mciOpen;
			MCI_PLAY_PARMS m_mciPlay;
			UINT m_curDeviceID;
			m_mciOpen.lpstrDeviceType = _T("mpegvideo");
	 	    m_mciOpen.lpstrElementName = _T("music/ֲ���ս��ʬ��������.mp3");
	//		m_mciOpen.lpstrElementName = path;
			MCIERROR mcierror = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&m_mciOpen);
			m_curDeviceID = m_mciOpen.wDeviceID;
			mciSendCommand(m_curDeviceID, MCI_PLAY, MCI_NOTIFY,
				(DWORD)&m_mciPlay);
		}
	case WM_COMMAND:
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO:  �ڴ���������ͼ����...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, Timer_Draw);
		KillTimer(hWnd, Timer_CreateMonster);
		KillTimer(hWnd, Timer_CreateBullet);
		if (!CheckGameSucceed)
			KillTimer(hWnd, Timer_CreateGold);
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
	{
		//ȷ�������λ���ǻ�����ǹ
		OnLButtonDown(lParam);
		break;
	}
	case WM_LBUTTONUP:
		OnLButtonUp(lParam);
		g_bMouseHaveItem = false;
		break;
	case WM_MOUSEMOVE:
		if (!g_bMouseHaveItem)
		{
			//����������û�а��£���Ҫ������
			break;
		}
		g_mouseItemCenterPos.x = GET_X_LPARAM(lParam);
		g_mouseItemCenterPos.y = GET_Y_LPARAM(lParam);
		break;
	case WM_TIMER:
		switch (wParam)
		{
		case Timer_Draw:
			DrawAll();
			break;
		case Timer_CreateBullet:
			CreateBullet();
			break;
		case Timer_CreateMonster:
			CreateMonster();
			break;
		case Timer_CreateGold:
			CreateGold();
			break;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void DrawAll()
{
	//��������Ȼ���
	//�����㡢����㡢UI��
	//������
	HDC hDc = GetDC(g_hWnd);

	//����DC
	HDC hDcMem = CreateCompatibleDC(hDc);
	HBITMAP hBitmapBack = CreateCompatibleBitmap(hDc, 900, 506);
	SelectObject(hDcMem, hBitmapBack);

	HDC dcTemp = CreateCompatibleDC(hDc);
	SelectObject(dcTemp, g_hBitmapBack);
	BITMAP bitInfo;
	GetObject(g_hBitmapBack, sizeof(BITMAP), &bitInfo);
	BitBlt(hDcMem, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight, dcTemp, 0, 0, SRCCOPY);
	//���ƶ��󣨻���ǹ���ӵ����֣�
	//��
	for (stObject obj:g_listFlower)
	{
		SelectObject(dcTemp, g_hBitmapFlower);
		GetObject(g_hBitmapFlower, sizeof(BITMAP), &bitInfo);
		tagPOINT pt = obj.ptPost;
		pt.x = pt.x*LATTICE_WIDTH + START_X_POS;	//�����ڸ��ӵ�X����
		pt.y = pt.y*LATTICE_HEIGHT + START_Y_POS;   //�����ڸ��ӵ�Y����
		pt.x = pt.x + (LATTICE_WIDTH - bitInfo.bmWidth) / 2;	//��������X����
		pt.y = pt.y + (LATTICE_HEIGHT - bitInfo.bmHeight) / 2;	//��������Y����
		TransparentBlt(hDcMem, pt.x, pt.y, bitInfo.bmWidth, bitInfo.bmHeight,
			dcTemp, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight,RGB(255,0,255));
	}
	//ǹ
	for (stObject obj:g_listGun)
	{
		SelectObject(dcTemp, g_hBitmapGun);
		GetObject(g_hBitmapGun, sizeof(BITMAP), &bitInfo);
		tagPOINT pt = obj.ptPost;
		pt.x = pt.x*LATTICE_WIDTH + START_X_POS;	//ǹ���ڸ��ӵ�X����
		pt.y = pt.y*LATTICE_HEIGHT + START_Y_POS;   //ǹ���ڸ��ӵ�Y����
		pt.x = pt.x + (LATTICE_WIDTH - bitInfo.bmWidth) / 2;	//ǹ������X����
		pt.y = pt.y + (LATTICE_HEIGHT - bitInfo.bmHeight) / 2;	//ǹ������Y����
		TransparentBlt(hDcMem, pt.x, pt.y, bitInfo.bmWidth, bitInfo.bmHeight,
			dcTemp, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));
	}

	//�ӵ�
	for (stObject& obj:g_listBullet)
	{
		SelectObject(dcTemp, g_hBitmapBullet);
		GetObject(g_hBitmapBullet, sizeof(BITMAP), &bitInfo);
		TransparentBlt(hDcMem, obj.ptPost.x, obj.ptPost.y, bitInfo.bmWidth, bitInfo.bmHeight,
			dcTemp, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));
		obj.ptPost.x += obj.speed;
	}
	//��
	for (stObject& obj:g_listMonster)
	{
		SelectObject(dcTemp, g_hBitmapMonster);
		int index = obj.picIndex;
		int lineIndex = index / 4;
		int rankIndex = index % 4;
		tagPOINT picPos;
		picPos.x = rankIndex*MONSTER_WIDTH;
		picPos.y = lineIndex*MONSTER_HEIGHT;
		TransparentBlt(hDcMem, obj.ptPost.x, obj.ptPost.y, MONSTER_WIDTH, MONSTER_HEIGHT,
			dcTemp, picPos.x, picPos.y, MONSTER_WIDTH, MONSTER_HEIGHT, RGB(255, 0, 255));
		obj.picIndex = (obj.picIndex + 1) % 14;
		obj.ptPost.x -= obj.speed;
		if (CheckGameOver())
		{
	//		TCHAR path[MAX_PATH];
	//		GetCurrentDirectory(MAX_PATH, path);
	//		wsprintf(path, _T("%s\\..\\Debug\\music\\ֲ���ս��ʬʧ����Ч.mp3"), path);
			MCI_OPEN_PARMS m_mciOpen;
			MCI_PLAY_PARMS m_mciPlay;
			UINT m_curDeviceID;
			m_mciOpen.lpstrDeviceType = _T("mpegvideo");
			m_mciOpen.lpstrElementName = _T("music/ֲ���ս��ʬʧ����Ч.mp3");
	//		m_mciOpen.lpstrElementName = path;
			MCIERROR mcierror = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&m_mciOpen);
			m_curDeviceID = m_mciOpen.wDeviceID;
			mciSendCommand(m_curDeviceID, MCI_PLAY, MCI_NOTIFY,
				(DWORD)&m_mciPlay);
			LOGFONT font;
			memset(&font, 0, sizeof(LOGFONT));
			font.lfCharSet = HANGUL_CHARSET;
			font.lfWidth = 20;
			font.lfHeight = 40;
			font.lfWeight = 900;
			HFONT hFont = CreateFontIndirect(&font);
			SelectObject(hDc, hFont);
			TCHAR overText[40] = _T("��ʬ�Ե����������!");
			SetBkMode(hDc, 0);
			SetTextColor(hDc, RGB(255, 0, 0));
			TextOut(hDc, 200, 100, overText, lstrlen(overText));
			DeleteObject(hFont);
 		//	sndPlaySound(_T("videos/ֲ���ս��ʬʧ����Ч.wav"), SND_FILENAME | SND_ASYNC);
			goto exit;
		}
	}

	//���ƻ���ǹ��ͼ��
	SelectObject(dcTemp, g_hBitmapFlower);
	GetObject(g_hBitmapFlower, sizeof(BITMAP), &bitInfo);
	TransparentBlt(hDcMem, 200, 10, bitInfo.bmWidth, bitInfo.bmHeight, dcTemp, 0, 0,
		bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));
	SelectObject(dcTemp, g_hBitmapGun);
	GetObject(g_hBitmapGun, sizeof(BITMAP), &bitInfo);
	TransparentBlt(hDcMem, 250, 10, bitInfo.bmWidth, bitInfo.bmHeight, dcTemp, 0, 0,
		bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));

	//����Ǯ
	SetBkMode(hDcMem, 0);
	SetTextColor(hDcMem, RGB(235, 235, 31));
	wchar_t moneyText[20];
	wsprintf(moneyText, _T("gold: %d"), g_money);
	TextOut(hDcMem, 100, 25, moneyText, lstrlen(moneyText));

	//UI�㣬����ϵ�����
	HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
	for (stObject& obj : g_listMonster)
	{
		int hpValue = obj.hpValue;
		int maxHp = 100;
		float f = hpValue / (float)maxHp;
		//�Ȼ��Ƶ�ͼ��͸�����ľ��ο�
		HBRUSH hRush = (HBRUSH)GetStockObject(NULL_BRUSH);
		SelectObject(hDcMem, hRush);
		Rectangle(hDcMem, obj.ptPost.x , obj.ptPost.y - 10,
			obj.ptPost.x + HPITEM_W, obj.ptPost.y - 10 + HPITEM_H);
		//SelectObject(hDcMem, hBrush);
		RECT rect;
		rect.left = obj.ptPost.x + 1;
		rect.top = obj.ptPost.y - 10 + 1;
		rect.right = rect.left+f*(HPITEM_W-2);
		rect.bottom = rect.top + HPITEM_H-2;
		FillRect(hDcMem, &rect,hBrush);
	}

	if (g_bMouseHaveItem)
	{
		if (g_mouseItemType == 1)
		{
			//��
			SelectObject(dcTemp, g_hBitmapFlower);
			GetObject(g_hBitmapFlower, sizeof(BITMAP), &bitInfo);
		}
		else
		{
			//ǹ
			SelectObject(dcTemp, g_hBitmapGun);
			GetObject(g_hBitmapGun, sizeof(BITMAP), &bitInfo);
		}
		tagPOINT ptPos;
		ptPos.x = g_mouseItemCenterPos.x - bitInfo.bmWidth/2;
		ptPos.y = g_mouseItemCenterPos.y - bitInfo.bmHeight/2;
		TransparentBlt(hDcMem, ptPos.x, ptPos.y, bitInfo.bmWidth, bitInfo.bmHeight, dcTemp, 0, 0,
			bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));
	}

	//�ѻ���DC�ϵ��ڴ���Ƶ�hDC
	BitBlt(hDc, 0, 0, 900, 506, hDcMem, 0, 0, SRCCOPY);

	CheckCollise();	//�����ײ

	if (CheckGameSucceed())
	{
//		TCHAR path[MAX_PATH];
//		GetCurrentDirectory(MAX_PATH, path);
//		wsprintf(path, _T("%s\\..\\Debug\\music\\ֲ���ս��ʬʤ����Ч.mp3"), path);
		KillTimer(g_hWnd, Timer_CreateGold);
		MCI_OPEN_PARMS m_mciOpen;
		MCI_PLAY_PARMS m_mciPlay;
		UINT m_curDeviceID;
		m_mciOpen.lpstrDeviceType = _T("mpegvideo");
		m_mciOpen.lpstrElementName = _T("music/ֲ���ս��ʬʤ����Ч.mp3");
//		m_mciOpen.lpstrElementName = path;
		MCIERROR mcierror = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&m_mciOpen);
		m_curDeviceID = m_mciOpen.wDeviceID;
		mciSendCommand(m_curDeviceID, MCI_PLAY, MCI_NOTIFY,
			(DWORD)&m_mciPlay);
		LOGFONT font;
		memset(&font, 0, sizeof(LOGFONT));
		font.lfCharSet = HANGUL_CHARSET;
		font.lfWidth = 20;
		font.lfHeight = 40;
		font.lfWeight = 900;
		HFONT hFont = CreateFontIndirect(&font);
		SelectObject(hDc, hFont);
		TCHAR overText[40] = _T("��ϲ������Ϸʤ��������");
		SetBkMode(hDc, 0);
		SetTextColor(hDc, RGB(255, 0, 0));
		TextOut(hDc, 200, 100, overText, lstrlen(overText));
		DeleteObject(hFont);
		goto exit;
	}

exit:
	DeleteObject(hBrush);
	DeleteObject(hDcMem);
	DeleteObject(hBitmapBack);
	DeleteObject(dcTemp);
	ReleaseDC(g_hWnd, hDc);

}

void PreLoadBitmap()
{
	g_hBitmapBack = (HBITMAP)LoadImage(NULL, _T("images/map.BMP"), IMAGE_BITMAP, 
		0, 0, LR_LOADFROMFILE);
	g_hBitmapBullet = (HBITMAP)LoadImage(NULL, _T("images/bullet.BMP"), IMAGE_BITMAP,
		0, 0, LR_LOADFROMFILE);
	g_hBitmapFlower = (HBITMAP)LoadImage(NULL, _T("images/flower.BMP"), IMAGE_BITMAP,
		0, 0, LR_LOADFROMFILE);
	g_hBitmapGun = (HBITMAP)LoadImage(NULL, _T("images/gun.BMP"), IMAGE_BITMAP,
		0, 0, LR_LOADFROMFILE);
	g_hBitmapMonster = (HBITMAP)LoadImage(NULL, _T("images/monster.BMP"), IMAGE_BITMAP,
		0, 0, LR_LOADFROMFILE);
}

void CreateMonster()	//������ʬ
{
	if (g_MonsterCount >= MAXMONSTERSIZE)
	{
		return;
	}
	//��ȷ��λ��
	stObject obj;
	obj.ptPost.x = 900;
	obj.ptPost.y = (rand() % 3)*LATTICE_HEIGHT+START_Y_POS+10;
	obj.speed = rand() % 2+1;
	obj.hpValue = 100;
	obj.picIndex = 0;
	g_listMonster.push_back(obj);
	g_MonsterCount++;
}

void CreateBullet()	//�����ӵ�
{
	//����ĸ�ǹ���ڿ��Է����ӵ��ˣ��ͷ��䣬�����Է���ľͺ���
	DWORD timeNow = GetTickCount();
	for (stObject& obj:g_listGun)
	{
		if (timeNow >= obj.lastActionTime+GunFireTimeElapce)
		{
			//��Ҫ�����ӵ�
			stObject objBullet;
			objBullet.speed = 5;
			tagPOINT pt = obj.ptPost;
			pt.x = pt.x*LATTICE_WIDTH + START_X_POS;	//ǹ���ڸ��ӵ�X����
			pt.y = pt.y*LATTICE_HEIGHT + START_Y_POS;   //ǹ���ڸ��ӵ�Y����
			objBullet.ptPost.x = pt.x + 55;
			objBullet.ptPost.y = pt.y + 32;
			g_listBullet.push_back(objBullet);
		}
	}
}

void CheckCollise()
{
	//�ӵ��͹������ͻ��������ǹ
	for (list<stObject>::iterator itBullet = g_listBullet.begin();
		itBullet != g_listBullet.end();)
	{
		tagPOINT posBullet = itBullet->ptPost;
		bool bCollise = false;
		for (list<stObject>::iterator itMonster = g_listMonster.begin();
			itMonster != g_listMonster.end();++itMonster)
		{
			tagPOINT posMonster = itMonster->ptPost;
			if (posMonster.x <= posBullet.x+15 && posMonster.x > posBullet.x-55 &&
				posBullet.y-posMonster.y > 0 && posBullet.y-posMonster.y<50)
			{
				//��ײ����
				itMonster->hpValue -= BULLETATTACKVALUE;
				if (itMonster->hpValue <= 0)
				{
					g_listMonster.erase(itMonster);
					g_MonsterCount2--;
				}
				itBullet = g_listBullet.erase(itBullet);
				bCollise = true;
				break;
			}
		}
		if (!bCollise)
		{
			itBullet++;
		}
	}

	//����ͻ�
	for (list<stObject>::iterator itFlower = g_listFlower.begin();
		itFlower != g_listFlower.end();)
	{
		tagPOINT posFlower = itFlower->ptPost;
		tagPOINT pt = posFlower;
		pt.x = pt.x*LATTICE_WIDTH + START_X_POS;	//�����ڸ��ӵ�X����
		pt.y = pt.y*LATTICE_HEIGHT + START_Y_POS;   //�����ڸ��ӵ�Y����
		pt.x = pt.x + (LATTICE_WIDTH - 50) / 2;	//��������X����
		pt.y = pt.y + (LATTICE_HEIGHT - 62) / 2;	//��������Y����
		posFlower = pt;
		bool bCollise = false;
		for (list<stObject>::iterator itMonster = g_listMonster.begin();
			itMonster != g_listMonster.end();++itMonster)
		{
			tagPOINT posMonster = itMonster->ptPost;
			if (posMonster.x <= posFlower.x + 35 && posMonster.x > posFlower.x - 55 &&
				posFlower.y - posMonster.y > 0 && posFlower.y - posMonster.y < 50)
			{
				//��ײ����
				itFlower = g_listFlower.erase(itFlower);
				bCollise = true;
				break;
			}
		}
		if (!bCollise)
		{
			++itFlower;
		}
	}

	//�����ǹ
	for (list<stObject>::iterator itGun = g_listGun.begin();
		itGun != g_listGun.end();)
	{
		tagPOINT posGun = itGun->ptPost;
		tagPOINT pt = posGun;
		pt.x = pt.x*LATTICE_WIDTH + START_X_POS;	//ǹ���ڸ��ӵ�X����
		pt.y = pt.y*LATTICE_HEIGHT + START_Y_POS;   //ǹ���ڸ��ӵ�Y����
		pt.x = pt.x + (LATTICE_WIDTH - 50) / 2;	//ǹ������X����
		pt.y = pt.y + (LATTICE_HEIGHT - 62) / 2;	//ǹ������Y����
		posGun = pt;
		bool bCollise = false;
		for (list<stObject>::iterator itMonster = g_listMonster.begin();
			itMonster != g_listMonster.end(); ++itMonster)
		{
			tagPOINT posMonster = itMonster->ptPost;
			if (posMonster.x <= posGun.x + 40 && posMonster.x > posGun.x - 55 &&
				posGun.y - posMonster.y > 0 && posGun.y - posMonster.y < 50)
			{
				//��ײ����
				itGun = g_listGun.erase(itGun);
				bCollise = true;
				break;
			}
		}
		if (!bCollise)
		{
			++itGun;
		}
	}
}

void OnLButtonDown(LPARAM lParam)
{
	//������λ��
	tagPOINT posCursor;
	posCursor.x = GET_X_LPARAM(lParam);
	posCursor.y = GET_Y_LPARAM(lParam);

	//�ж�����������ڻ��л���ǹ��
	RECT rect;
	rect.left = 200;
	rect.top = 10;
	rect.right = 200 + 50;
	rect.bottom = 10 + 62;
	if (IsInRect(posCursor, rect))
	{
		//���ڻ���
		g_mouseItemType = 1;
		g_bMouseHaveItem = true; 
		return;
	}
	rect.left = 250;
	rect.top = 10;
	rect.right = 250 + 50;
	rect.bottom = 10 + 62;
	if (IsInRect(posCursor, rect))
	{
		//����ǹ��
		g_mouseItemType = 2;
		g_bMouseHaveItem = true;
	}
}

bool IsInRect(tagPOINT pt, RECT rect)
{
	if (pt.x >= rect.left && pt.x <= rect.right &&
		pt.y >= rect.top && pt.y <= rect.bottom)
	{
		return true;
	}
	return false;
}

void OnLButtonUp(LPARAM lParam)
{
	if (!g_bMouseHaveItem)
	{
		return;
	}
	if (g_mouseItemType == 1 && g_money < FlOWER_PRICE) 
	{
		return;
		//���ļ۸���100
	}
	if (g_mouseItemType == 2 && g_money < GUN_PRICE)
	{
		return;
	}
	tagPOINT posCursor;
	posCursor.x = GET_X_LPARAM(lParam);
	posCursor.y = GET_Y_LPARAM(lParam);
	RECT canCreateRect;
	canCreateRect.left = START_X_POS;
	canCreateRect.top = START_Y_POS;
	canCreateRect.right = canCreateRect.left + 10 * LATTICE_WIDTH;
	canCreateRect.bottom = canCreateRect.top + 3 * LATTICE_HEIGHT;
	if (!IsInRect(posCursor,canCreateRect))
	{
		return;
	}

	//���λ���Ƿ��Ѿ���ֲ�˻�����ǹ
	posCursor.x = (posCursor.x - START_X_POS) / LATTICE_WIDTH;
	posCursor.y = (posCursor.y - START_Y_POS) / LATTICE_HEIGHT;

	for (stObject obj:g_listFlower)
	{
		if (obj.ptPost.x == posCursor.x && obj.ptPost.y == posCursor.y)
		{
			//˵���Ѿ����˻��������棬���ܷ�
			return;
		}
	}
	for (stObject obj : g_listGun)
	{
		if (obj.ptPost.x == posCursor.x && obj.ptPost.y == posCursor.y)
		{
			//˵���Ѿ�����ǹ�������棬���ܷ�
			return;
		}
	}
	//��Ǯ
	g_money -= g_mouseItemType == 1 ? FlOWER_PRICE : GUN_PRICE;
	stObject obj;
	obj.ptPost = posCursor;
	obj.lastActionTime = GetTickCount();
	if (g_mouseItemType == 1)
	{
		//������
		g_listFlower.push_back(obj);
	}
	else
	{
		//����ǹ
		g_listGun.push_back(obj);
	}
}

void CreateGold()
{
	DWORD timeNow = GetTickCount();
	for (stObject& obj:g_listFlower)
	{
		if (timeNow >= obj.lastActionTime + FlOWER_CREATEGOLDTIME)
		{
			//��Ǯ
			g_money += 30;
			obj.lastActionTime = timeNow;
		}
	}
}

bool CheckGameOver()
{
	for (stObject &obj : g_listMonster)
	{
		tagPOINT pt = obj.ptPost;
		if (pt.x + 50 <= 0)
		{
			return true;
		}
	}
	return false;
}

bool CheckGameSucceed()
{
	if (g_MonsterCount2 <= 0)
	{
		return true;
	}
	return false;
}
