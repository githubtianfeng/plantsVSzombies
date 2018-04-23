// plantsVSmonster.cpp : 定义应用程序的入口点。
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

// 全局变量: 
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

//宏定义
#define START_X_POS 60			//格子的起始X坐标
#define START_Y_POS 90			//格子的起始Y坐标
#define LATTICE_WIDTH 80		//每个格子的宽
#define LATTICE_HEIGHT 110      //每个格子的高
#define MONSTER_WIDTH 55		//怪的宽
#define MONSTER_HEIGHT 106      //怪的高
#define GunFireTimeElapce 1500  //枪发射子弹的时间间隔 
#define BULLETATTACKVALUE 8		//子弹攻击力
#define GUN_PRICE 70	//枪的价格
#define FlOWER_PRICE 100	//花的价格
#define FlOWER_CREATEGOLDTIME 6000	//花多久创建一个金币（30）
#define HPITEM_H	10	//血条高
#define HPITEM_W	50	//血条宽度
#define MAXMONSTERSIZE 25	//僵尸最大数量

//数据类型
enum TimerType
{
	Timer_Draw,
	Timer_CreateMonster,
	Timer_CreateBullet,
	Timer_CreateGold
};

struct stObject
{
	tagPOINT ptPost;	//对象的坐标，对于花和枪是格子坐标，子弹和怪是实际坐标
	int hpValue;		//怪物血量
	UINT speed;			//速度
	UINT picIndex;		//僵尸显示的当前图片
	//枪和花
	DWORD lastActionTime;	//对于枪上次发射子弹的时间，对于花上次果实成熟时间
};

//函数
void DrawAll();		//绘制所有内容
void PreLoadBitmap();	//预加载所有资源
void CreateMonster();	//创建僵尸
void CreateBullet();	//创建子弹
void CheckCollise();	//检测碰撞
void OnLButtonDown(LPARAM lParam);	//左键点击
void OnLButtonUp(LPARAM lParam);  //左键弹起
void CreateGold();	//创建金币
bool IsInRect(tagPOINT pt, RECT rect);	//判断一个点是否在一个矩形区域中
bool CheckGameOver();		//检测游戏是否结束
bool CheckGameSucceed();	//检测是否游戏过关
list<stObject> g_listFlower;	//花
list<stObject> g_listGun;		//枪
list<stObject> g_listMonster;	//怪
list<stObject> g_listBullet;	//子弹

//变量
HWND g_hWnd = NULL;	//窗口句柄
HBITMAP g_hBitmapBack = NULL;	//背景
HBITMAP g_hBitmapGun = NULL;	//枪
HBITMAP g_hBitmapFlower = NULL;	//花
HBITMAP g_hBitmapBullet = NULL;	//子弹
HBITMAP g_hBitmapMonster = NULL;//怪物
int g_mouseItemType = 1;		//如果为1就是花，如果为2就是枪
bool g_bMouseHaveItem= false;		//标识鼠标是否有物体
tagPOINT g_mouseItemCenterPos;	//鼠标位置
int g_money = 600;					//金币数
static int g_MonsterCount = 0;		//僵尸目前数量
static int g_MonsterCount2 = MAXMONSTERSIZE;	//僵尸目前数量

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PLANTSVSMONSTER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PLANTSVSMONSTER));

	// 主消息循环: 
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

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME &
	   ~WS_MAXIMIZEBOX,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   g_hWnd = hWnd;
   PreLoadBitmap();	//加载资源

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
	//		wsprintf(path, _T("%s\\..\\Debug\\music\\植物大战僵尸背景音乐.mp3"), path);
			MCI_OPEN_PARMS m_mciOpen;
			MCI_PLAY_PARMS m_mciPlay;
			UINT m_curDeviceID;
			m_mciOpen.lpstrDeviceType = _T("mpegvideo");
	 	    m_mciOpen.lpstrElementName = _T("music/植物大战僵尸背景音乐.mp3");
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
		// TODO:  在此添加任意绘图代码...
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
		//确定点击的位置是花还是枪
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
			//如果此是鼠标没有按下，不要做处理
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
	//最下面的先绘制
	//背景层、对象层、UI层
	//背景层
	HDC hDc = GetDC(g_hWnd);

	//缓存DC
	HDC hDcMem = CreateCompatibleDC(hDc);
	HBITMAP hBitmapBack = CreateCompatibleBitmap(hDc, 900, 506);
	SelectObject(hDcMem, hBitmapBack);

	HDC dcTemp = CreateCompatibleDC(hDc);
	SelectObject(dcTemp, g_hBitmapBack);
	BITMAP bitInfo;
	GetObject(g_hBitmapBack, sizeof(BITMAP), &bitInfo);
	BitBlt(hDcMem, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight, dcTemp, 0, 0, SRCCOPY);
	//绘制对象（花，枪，子弹，怪）
	//花
	for (stObject obj:g_listFlower)
	{
		SelectObject(dcTemp, g_hBitmapFlower);
		GetObject(g_hBitmapFlower, sizeof(BITMAP), &bitInfo);
		tagPOINT pt = obj.ptPost;
		pt.x = pt.x*LATTICE_WIDTH + START_X_POS;	//花所在格子的X坐标
		pt.y = pt.y*LATTICE_HEIGHT + START_Y_POS;   //花所在格子的Y坐标
		pt.x = pt.x + (LATTICE_WIDTH - bitInfo.bmWidth) / 2;	//花真正的X坐标
		pt.y = pt.y + (LATTICE_HEIGHT - bitInfo.bmHeight) / 2;	//花真正的Y坐标
		TransparentBlt(hDcMem, pt.x, pt.y, bitInfo.bmWidth, bitInfo.bmHeight,
			dcTemp, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight,RGB(255,0,255));
	}
	//枪
	for (stObject obj:g_listGun)
	{
		SelectObject(dcTemp, g_hBitmapGun);
		GetObject(g_hBitmapGun, sizeof(BITMAP), &bitInfo);
		tagPOINT pt = obj.ptPost;
		pt.x = pt.x*LATTICE_WIDTH + START_X_POS;	//枪所在格子的X坐标
		pt.y = pt.y*LATTICE_HEIGHT + START_Y_POS;   //枪所在格子的Y坐标
		pt.x = pt.x + (LATTICE_WIDTH - bitInfo.bmWidth) / 2;	//枪真正的X坐标
		pt.y = pt.y + (LATTICE_HEIGHT - bitInfo.bmHeight) / 2;	//枪真正的Y坐标
		TransparentBlt(hDcMem, pt.x, pt.y, bitInfo.bmWidth, bitInfo.bmHeight,
			dcTemp, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));
	}

	//子弹
	for (stObject& obj:g_listBullet)
	{
		SelectObject(dcTemp, g_hBitmapBullet);
		GetObject(g_hBitmapBullet, sizeof(BITMAP), &bitInfo);
		TransparentBlt(hDcMem, obj.ptPost.x, obj.ptPost.y, bitInfo.bmWidth, bitInfo.bmHeight,
			dcTemp, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));
		obj.ptPost.x += obj.speed;
	}
	//怪
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
	//		wsprintf(path, _T("%s\\..\\Debug\\music\\植物大战僵尸失败音效.mp3"), path);
			MCI_OPEN_PARMS m_mciOpen;
			MCI_PLAY_PARMS m_mciPlay;
			UINT m_curDeviceID;
			m_mciOpen.lpstrDeviceType = _T("mpegvideo");
			m_mciOpen.lpstrElementName = _T("music/植物大战僵尸失败音效.mp3");
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
			TCHAR overText[40] = _T("僵尸吃掉了你的脑子!");
			SetBkMode(hDc, 0);
			SetTextColor(hDc, RGB(255, 0, 0));
			TextOut(hDc, 200, 100, overText, lstrlen(overText));
			DeleteObject(hFont);
 		//	sndPlaySound(_T("videos/植物大战僵尸失败音效.wav"), SND_FILENAME | SND_ASYNC);
			goto exit;
		}
	}

	//绘制花和枪的图标
	SelectObject(dcTemp, g_hBitmapFlower);
	GetObject(g_hBitmapFlower, sizeof(BITMAP), &bitInfo);
	TransparentBlt(hDcMem, 200, 10, bitInfo.bmWidth, bitInfo.bmHeight, dcTemp, 0, 0,
		bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));
	SelectObject(dcTemp, g_hBitmapGun);
	GetObject(g_hBitmapGun, sizeof(BITMAP), &bitInfo);
	TransparentBlt(hDcMem, 250, 10, bitInfo.bmWidth, bitInfo.bmHeight, dcTemp, 0, 0,
		bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));

	//绘制钱
	SetBkMode(hDcMem, 0);
	SetTextColor(hDcMem, RGB(235, 235, 31));
	wchar_t moneyText[20];
	wsprintf(moneyText, _T("gold: %d"), g_money);
	TextOut(hDcMem, 100, 25, moneyText, lstrlen(moneyText));

	//UI层，鼠标上的物体
	HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
	for (stObject& obj : g_listMonster)
	{
		int hpValue = obj.hpValue;
		int maxHp = 100;
		float f = hpValue / (float)maxHp;
		//先绘制底图，透明填充的矩形框
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
			//花
			SelectObject(dcTemp, g_hBitmapFlower);
			GetObject(g_hBitmapFlower, sizeof(BITMAP), &bitInfo);
		}
		else
		{
			//枪
			SelectObject(dcTemp, g_hBitmapGun);
			GetObject(g_hBitmapGun, sizeof(BITMAP), &bitInfo);
		}
		tagPOINT ptPos;
		ptPos.x = g_mouseItemCenterPos.x - bitInfo.bmWidth/2;
		ptPos.y = g_mouseItemCenterPos.y - bitInfo.bmHeight/2;
		TransparentBlt(hDcMem, ptPos.x, ptPos.y, bitInfo.bmWidth, bitInfo.bmHeight, dcTemp, 0, 0,
			bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));
	}

	//把缓存DC上的内存绘制到hDC
	BitBlt(hDc, 0, 0, 900, 506, hDcMem, 0, 0, SRCCOPY);

	CheckCollise();	//检测碰撞

	if (CheckGameSucceed())
	{
//		TCHAR path[MAX_PATH];
//		GetCurrentDirectory(MAX_PATH, path);
//		wsprintf(path, _T("%s\\..\\Debug\\music\\植物大战僵尸胜利音效.mp3"), path);
		KillTimer(g_hWnd, Timer_CreateGold);
		MCI_OPEN_PARMS m_mciOpen;
		MCI_PLAY_PARMS m_mciPlay;
		UINT m_curDeviceID;
		m_mciOpen.lpstrDeviceType = _T("mpegvideo");
		m_mciOpen.lpstrElementName = _T("music/植物大战僵尸胜利音效.mp3");
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
		TCHAR overText[40] = _T("恭喜您，游戏胜利！！！");
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

void CreateMonster()	//创建僵尸
{
	if (g_MonsterCount >= MAXMONSTERSIZE)
	{
		return;
	}
	//先确定位置
	stObject obj;
	obj.ptPost.x = 900;
	obj.ptPost.y = (rand() % 3)*LATTICE_HEIGHT+START_Y_POS+10;
	obj.speed = rand() % 2+1;
	obj.hpValue = 100;
	obj.picIndex = 0;
	g_listMonster.push_back(obj);
	g_MonsterCount++;
}

void CreateBullet()	//创建子弹
{
	//检测哪个枪现在可以发射子弹了，就发射，不可以发射的就忽略
	DWORD timeNow = GetTickCount();
	for (stObject& obj:g_listGun)
	{
		if (timeNow >= obj.lastActionTime+GunFireTimeElapce)
		{
			//需要发射子弹
			stObject objBullet;
			objBullet.speed = 5;
			tagPOINT pt = obj.ptPost;
			pt.x = pt.x*LATTICE_WIDTH + START_X_POS;	//枪所在格子的X坐标
			pt.y = pt.y*LATTICE_HEIGHT + START_Y_POS;   //枪所在格子的Y坐标
			objBullet.ptPost.x = pt.x + 55;
			objBullet.ptPost.y = pt.y + 32;
			g_listBullet.push_back(objBullet);
		}
	}
}

void CheckCollise()
{
	//子弹和怪物、怪物和花、怪物和枪
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
				//碰撞到了
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

	//怪物和花
	for (list<stObject>::iterator itFlower = g_listFlower.begin();
		itFlower != g_listFlower.end();)
	{
		tagPOINT posFlower = itFlower->ptPost;
		tagPOINT pt = posFlower;
		pt.x = pt.x*LATTICE_WIDTH + START_X_POS;	//花所在格子的X坐标
		pt.y = pt.y*LATTICE_HEIGHT + START_Y_POS;   //花所在格子的Y坐标
		pt.x = pt.x + (LATTICE_WIDTH - 50) / 2;	//花真正的X坐标
		pt.y = pt.y + (LATTICE_HEIGHT - 62) / 2;	//花真正的Y坐标
		posFlower = pt;
		bool bCollise = false;
		for (list<stObject>::iterator itMonster = g_listMonster.begin();
			itMonster != g_listMonster.end();++itMonster)
		{
			tagPOINT posMonster = itMonster->ptPost;
			if (posMonster.x <= posFlower.x + 35 && posMonster.x > posFlower.x - 55 &&
				posFlower.y - posMonster.y > 0 && posFlower.y - posMonster.y < 50)
			{
				//碰撞到了
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

	//怪物和枪
	for (list<stObject>::iterator itGun = g_listGun.begin();
		itGun != g_listGun.end();)
	{
		tagPOINT posGun = itGun->ptPost;
		tagPOINT pt = posGun;
		pt.x = pt.x*LATTICE_WIDTH + START_X_POS;	//枪所在格子的X坐标
		pt.y = pt.y*LATTICE_HEIGHT + START_Y_POS;   //枪所在格子的Y坐标
		pt.x = pt.x + (LATTICE_WIDTH - 50) / 2;	//枪真正的X坐标
		pt.y = pt.y + (LATTICE_HEIGHT - 62) / 2;	//枪真正的Y坐标
		posGun = pt;
		bool bCollise = false;
		for (list<stObject>::iterator itMonster = g_listMonster.begin();
			itMonster != g_listMonster.end(); ++itMonster)
		{
			tagPOINT posMonster = itMonster->ptPost;
			if (posMonster.x <= posGun.x + 40 && posMonster.x > posGun.x - 55 &&
				posGun.y - posMonster.y > 0 && posGun.y - posMonster.y < 50)
			{
				//碰撞到了
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
	//获得鼠标位置
	tagPOINT posCursor;
	posCursor.x = GET_X_LPARAM(lParam);
	posCursor.y = GET_Y_LPARAM(lParam);

	//判断这个坐标是在花中还是枪中
	RECT rect;
	rect.left = 200;
	rect.top = 10;
	rect.right = 200 + 50;
	rect.bottom = 10 + 62;
	if (IsInRect(posCursor, rect))
	{
		//是在花中
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
		//是在枪中
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
		//花的价格是100
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

	//这个位置是否已经种植了花或者枪
	posCursor.x = (posCursor.x - START_X_POS) / LATTICE_WIDTH;
	posCursor.y = (posCursor.y - START_Y_POS) / LATTICE_HEIGHT;

	for (stObject obj:g_listFlower)
	{
		if (obj.ptPost.x == posCursor.x && obj.ptPost.y == posCursor.y)
		{
			//说明已经有了花了在里面，不能放
			return;
		}
	}
	for (stObject obj : g_listGun)
	{
		if (obj.ptPost.x == posCursor.x && obj.ptPost.y == posCursor.y)
		{
			//说明已经有了枪了在里面，不能放
			return;
		}
	}
	//减钱
	g_money -= g_mouseItemType == 1 ? FlOWER_PRICE : GUN_PRICE;
	stObject obj;
	obj.ptPost = posCursor;
	obj.lastActionTime = GetTickCount();
	if (g_mouseItemType == 1)
	{
		//创建花
		g_listFlower.push_back(obj);
	}
	else
	{
		//创建枪
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
			//加钱
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
