#include <windows.h>
#include <assert.h>
#include <string>
#include <WindowsX.h>
#include "device.h"
#include "rmathf.h"
#include "mesh.h"
#include "camera.h"
#include <iostream>
#include "gameobject.h"

using namespace std;
const int SCREEN_WIDTH = Screen::Width;
const int SCREEN_HEIGHT = Screen::Height;

//��Ļ��Ⱥ͸߶�   
const int BITS = 32;				//ÿ�����ص�λ��  
HDC screen_hdc;
HWND screen_hwnd;
HDC hCompatibleDC; //����HDC  
HBITMAP hCompatibleBitmap; //����BITMAP    
BITMAPINFO binfo; //BITMAPINFO�ṹ��  
HINSTANCE ghInstance;


BYTE Buffer[SCREEN_WIDTH*SCREEN_HEIGHT * 4];
//��Ⱦװ��
Device *device;
Cube *cube;
Texture *tex;
float deltaTime;

void Display();
void SrcInit();
HWND Start(HINSTANCE hInstance, int nShowCmd, string wcName, string title);
void Update(HWND hwnd);
void Destroy(string wcName, HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lParam);






int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	//1.��������
	string windowClassName = "MyWindow";
	string title = "3DRender";
	HWND hwnd = Start(hInstance, nShowCmd, windowClassName, title);

	//ʱ���ʼ��
	DWORD curTime = GetTickCount();
	DWORD preTime = GetTickCount();
	//2.��Ϣѭ��
	MSG msg = { 0 };
	SrcInit();
	while (msg.message != WM_QUIT)
	{
		//��ȡ��Ϣ
		if (PeekMessage(&msg, 0, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			curTime = GetTickCount();
			if (curTime - preTime > 30)
			{
				deltaTime = curTime - preTime;
				preTime = curTime;
				Update(hwnd);
			}
		}
	}

	//3.��Ϸ����
	Destroy(windowClassName, hInstance);
	return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
	{
	}
	break;
	default:
		return DefWindowProc(hwnd, message, wparam, lParam);
	}
	return 0;
}
HWND Start(HINSTANCE hInstance, int nShowCmd, string wcName, string title)
{

	ghInstance = hInstance;
	//1.����������
	WNDCLASSEX wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = wcName.c_str();

	//2.ע�ᴰ����
	assert(RegisterClassEx(&wndClass));

	//3.��������
	HWND hwnd = CreateWindow(wcName.c_str(), title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, NULL, hInstance, NULL);

	//4.������С���ƶ�����ʾ������
	if (!hwnd)
	{
		return FALSE;
	}

	RECT  rectProgram, rectClient;
	GetWindowRect(hwnd, &rectProgram);   //��ó��򴰿�λ����Ļ����
	GetClientRect(hwnd, &rectClient);      //��ÿͻ�������
										   //�ǿͻ�����,��
	int nWidth = rectProgram.right - rectProgram.left - (rectClient.right - rectClient.left);
	int nHeiht = rectProgram.bottom - rectProgram.top - (rectClient.bottom - rectClient.top);
	nWidth += SCREEN_WIDTH;
	nHeiht += SCREEN_HEIGHT;
	rectProgram.right = nWidth;
	rectProgram.bottom = nHeiht;
	int showToScreenx = GetSystemMetrics(SM_CXSCREEN) / 2 - nWidth / 2;    //���д���
	int showToScreeny = GetSystemMetrics(SM_CYSCREEN) / 2 - nHeiht / 2;
	MoveWindow(hwnd, showToScreenx, showToScreeny, rectProgram.right, rectProgram.bottom, false);

	memset(&binfo, 0, sizeof(BITMAPINFO));
	binfo.bmiHeader.biBitCount = BITS;      //ÿ�����ض���λ��Ҳ��ֱ��д24(RGB)����32(RGBA)  
	binfo.bmiHeader.biCompression = BI_RGB;
	binfo.bmiHeader.biHeight = -SCREEN_HEIGHT;
	binfo.bmiHeader.biPlanes = 1;
	binfo.bmiHeader.biSizeImage = 0;
	binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	binfo.bmiHeader.biWidth = SCREEN_WIDTH;



	//��ȡ��ĻHDC  
	screen_hwnd = hwnd;
	screen_hdc = GetDC(screen_hwnd);

	//��ȡ����HDC�ͼ���Bitmap,����Bitmapѡ�����HDC(ÿ��HDC�ڴ�ÿʱ�̽���ѡ��һ��GDI��Դ,GDI��ԴҪѡ��HDC���ܽ��л���)  
	hCompatibleDC = CreateCompatibleDC(screen_hdc);
	hCompatibleBitmap = CreateCompatibleBitmap(screen_hdc, SCREEN_WIDTH, SCREEN_HEIGHT);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hCompatibleDC, hCompatibleBitmap);

	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	return hwnd;
}


void SrcInit()
{
	device = new Device(SCREEN_WIDTH, SCREEN_HEIGHT, Buffer, Color(1.0f, 0, 0, 1.0f));
	cube = new Cube;

	cube->transform->Scale(2, 2, 2);
	cube->transform->Rotate(0.0, 45.0, 0.0);

	tex = new Texture("102.bmp", 148, 149);
	Texture::LoadTexture(screen_hdc, tex);
}
void Update(HWND hwnd)
{
	//cube->transform->Rotate(-1.f, Axis::Y);
	device->Render(cube, tex);
	device->RenderTexture(tex, tex->width, tex->height);
	Display();
}
void Display()
{
	////����ɫ���ݴ�ӡ����Ļ�ϣ���������������ÿ֡���õ���  
	SetDIBits(screen_hdc, hCompatibleBitmap, 0, SCREEN_HEIGHT, Buffer, (BITMAPINFO*)&binfo, DIB_RGB_COLORS);
	BitBlt(screen_hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hCompatibleDC, 0, 0, SRCCOPY);
}
void Destroy(string wcName, HINSTANCE hInstance)
{
	/*delete device;
	device = nullptr;*/
	delete cube;
	cube = nullptr;
	//5.ע��������
	UnregisterClass(wcName.c_str(), hInstance);
}