// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <time.h>
#include <stdlib.h>

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

//Завдання:
//Організувати синхронізацію за допомогою семафорів для наступної задачі.
//Після запуску основної програми користувач задає максимальне число можливих копій цієї програми.
//Після чого, починає процес запуску програм кілька разів. Якщо ліміт програм вичерпаний, 
//наступна програма автоматично завершує роботу.

//my semaphore class
class Semaphore
{
private:
	//maximum number of resources available at once
	int MaxCounter;
	//current number of resources occupied
	int CurrentCounter = 0;

public:
	Semaphore()
	{
		srand(time(NULL));
		MaxCounter = rand() % 13 + 3;
		CurrentCounter = 0;
	}
	//check if semaphore can accept more resources
	bool IsSemaphoreOpen() { return CurrentCounter < MaxCounter; }
	//check if semaphore is empty
	bool IsSemaphoreEmpty() { return !CurrentCounter; }

	int GetMaxCounter() { return MaxCounter; }
	int GetCurrentCounter() { return CurrentCounter; }
	BOOL OccupyResource()
	{
		//semaphore is blocked
		if (CurrentCounter == MaxCounter)
			return FALSE;

		//take up one resource
		CurrentCounter++;
		return TRUE;
	}
	BOOL ReleaseResource()
	{
		//semaphore is free
		if (CurrentCounter == 0)
			return FALSE;

		CurrentCounter--;
		return TRUE;
	}
};

//all global declarations
HINSTANCE hInst;
Semaphore semaphore;

ATOM MyRegisterClass(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI ThreadProc(LPVOID);

//main function
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	MyRegisterClass(hInstance);
	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
//registering window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_INFORMATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(255, 255, 204));
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"IPP2_LYSENKO"; //window class name
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}
//creating window program
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	wchar_t buffer[45]{};
	hInst = hInstance;

	HWND hWindow = CreateWindow(L"IPP2_LYSENKO",//window class name
		L"Second lab",				//window name
		WS_OVERLAPPED | WS_SYSMENU, //window style
		530,						//x position on the screen
		275, 						//y position on the screen
		500,						//x-coordinate size
		300,						//y-coordinate size
		NULL,
		NULL,
		hInstance,
		NULL);
	
	HWND hButton = CreateWindow(
		L"BUTTON",			// Predefined class; Unicode assumed 
		L"Make a copy",     // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		165,				// x position 
		110,				// y position 
		150,				// Button width
		30,					// Button height
		hWindow,			// Parent window
		NULL,				// No menu.
		hInst,
		NULL);

	if (!hWindow || !hButton)
		return FALSE;

	semaphore.OccupyResource();
	ShowWindow(hWindow, SW_SHOW);	//show a window

	wsprintf(buffer, L"You can open up to %d window copies", semaphore.GetMaxCounter());
	MessageBox(hWindow, buffer, L"Info", MB_OK | MB_ICONINFORMATION);

	return TRUE;
}
//message processing function
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//process messages of the window
	switch (message)
	{
	//there is only one button, no ifs are needed
	case WM_COMMAND:
	{   
		//checking if the semaphore is blocked
		if (!semaphore.IsSemaphoreOpen()) break;

		HWND hWindow = CreateWindow(L"IPP2_LYSENKO", L"Second lab", WS_OVERLAPPED | WS_SYSMENU,
			rand() % 1000, rand() % 500, 500, 300, NULL, NULL, hInst, NULL);

		HWND hButton = CreateWindow(L"BUTTON", L"Make a copy", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			165, 110, 150, 30, hWindow, NULL, hInst, NULL);

		ShowWindow(hWindow, SW_SHOW);

		//taking up a resource when window is created
		semaphore.OccupyResource();
	}
	break;
	case WM_CLOSE:
		//releasing a resource when window is closed
		semaphore.ReleaseResource();
		DestroyWindow(hWnd);
	break;
	case WM_DESTROY:
		//ending my application if no windows are left
		if (semaphore.IsSemaphoreEmpty())
			PostQuitMessage(0);
	break;
	default:
		//let Windows handle the rest
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

