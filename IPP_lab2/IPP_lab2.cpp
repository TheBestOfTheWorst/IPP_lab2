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

//all global declarations
HINSTANCE hInst;
HANDLE semaphore;

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
	int MaxCounter; //how many apps can be opened at once
	wchar_t buffer[45]{};
	hInst = hInstance;

	srand(time(NULL));

	//trying to open existing semaphore
	semaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, L"MySemaphore");
	if (semaphore == NULL)
	{
		//there is none yet
		MaxCounter = rand() % 9 + 2;
		wsprintf(buffer, L"You can open up to %d window copies", MaxCounter);
		MessageBox(NULL, buffer, L"Info", MB_OK | MB_ICONINFORMATION);

		semaphore = CreateSemaphore(NULL, MaxCounter - 1, MaxCounter, L"MySemaphore");
	}

	HWND hWindow = CreateWindow(L"IPP2_LYSENKO",//window class name
		L"Second lab",				//window name
		WS_OVERLAPPED | WS_SYSMENU, //window style
		rand() % 1000,				//x position on the screen
		rand() % 600, 				//y position on the screen
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

	ShowWindow(hWindow, SW_SHOW);	//show a window

	return TRUE;
}
//message processing function
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static STARTUPINFO si;
	static PROCESS_INFORMATION pi;
	DWORD dwWaitResult;

	//process messages of the window
	switch (message)
	{
	//there is only one button, no ifs are needed
	case WM_COMMAND:
		dwWaitResult = WaitForSingleObject(semaphore, 0L);

		switch (dwWaitResult)
		{
		// The semaphore object was signaled
		case WAIT_OBJECT_0:
			CreateProcess(L"IPP_lab2.exe", // Module name
				NULL,			// Command line
				NULL,           // Process handle not inheritable
				NULL,           // Thread handle not inheritable
				FALSE,          // Set handle inheritance to FALSE
				NULL,           // No creation flags
				NULL,           // Use parent's environment block
				NULL,           // Use parent's starting directory 
				&si,            // Pointer to STARTUPINFO structure
				&pi);			// Pointer to PROCESS_INFORMATION structure
		break;
		// The semaphore object was non-signaled
		case WAIT_TIMEOUT: return 0;
		break;
		}
	break;
	case WM_DESTROY:
		//ending my application and releasing semaphore one resource
		ReleaseSemaphore(semaphore, 1, NULL);
		PostQuitMessage(0);
	break;
	default:
		//let Windows handle the rest
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

