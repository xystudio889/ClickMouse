// ClickMouse.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "ClickMouse.h"
#include "shellapi.h"
#include "windows.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
UINT_PTR g_TimerId = 0;                // 定时器ID
bool g_IsPaused = false;               // 暂停状态
int g_CurrentClick = 0;                // 当前点击类型 0:无 1:左键 2:右键
int g_Interval = 0;                    // 点击间隔
WCHAR g_szInterval[100] = {  0 };         // 保存合法的间隔值文本

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void LeftClick() {
    // 创建一个INPUT结构体用于发送鼠标事件
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = 0;
    input.mi.dy = 0;
    input.mi.mouseData = 0;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN; // 鼠标左键按下
    input.mi.time = 0;
    input.mi.dwExtraInfo = 0;
    SendInput(1, &input, sizeof(INPUT));

    // 模拟鼠标左键释放
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP; // 鼠标左键释放
    SendInput(1, &input, sizeof(INPUT));
}

void RightClick() {
    // 创建一个INPUT结构体用于发送鼠标事件
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = 0;
    input.mi.dy = 0;
    input.mi.mouseData = 0;
    input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN; // 鼠标右键按下
    input.mi.time = 0;
    input.mi.dwExtraInfo = 0;
    SendInput(1, &input, sizeof(INPUT));

    // 模拟鼠标右键释放
    input.mi.dwFlags = MOUSEEVENTF_RIGHTUP; // 鼠标右键释放
    SendInput(1, &input, sizeof(INPUT));
}

// 修改后的对话框消息处理函数
INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(GetModuleHandle(NULL),
                MAKEINTRESOURCE(IDD_ABOUTBOX),
                hDlg, About);
            break;

        case IDM_EXIT:
            SendMessage(hDlg, WM_CLOSE, 0, 0);
            break;

        case IDC_LEFT:   // 左键连点按钮
        case IDC_RIGHT:  // 右键连点按钮
        {
            // 停止现有定时器
            if (g_TimerId) {
                KillTimer(hDlg, g_TimerId);
                g_TimerId = 0;
            }

            // 获取并验证间隔时间
            WCHAR szInterval[100];
            GetDlgItemText(hDlg, IDC_DELAY_INPUT_BOX, szInterval, 100);

            // 检查是否为纯数字
            bool valid = true;
            for (int i = 0; szInterval[i]; i++) {
                if (!iswdigit(szInterval[i])) {
                    valid = false;
                    break;
                }
            }

            if (!valid || wcslen(szInterval) == 0) {
                MessageBox(hDlg, L"请输入有效的数字间隔", L"错误", MB_ICONERROR);
                return TRUE;
            }

            // 保存合法值
            wcscpy_s(g_szInterval, szInterval);
            g_Interval = _wtoi(szInterval);

            // 设置新的点击类型
            g_CurrentClick = (wmId == IDC_LEFT) ? 1 : 2;

            // 启动定时器
            g_TimerId = SetTimer(hDlg, 1, g_Interval, NULL);
            SetDlgItemText(hDlg, IDC_PAUSE, L"暂停");
            break;
        }

        case IDC_PAUSE:  // 暂停/重启按钮
        {
            if (g_CurrentClick != 0) {
                if (g_IsPaused) {
                    // 重启定时器
                    g_TimerId = SetTimer(hDlg, 1, g_Interval, NULL);
                    if (g_TimerId) {
                        SetDlgItemText(hDlg, IDC_PAUSE, L"暂停");
                        g_IsPaused = false;
                    }
                }
                else {
                    // 暂停定时器
                    if (KillTimer(hDlg, 1)) {
                        g_IsPaused = true;
                        SetDlgItemText(hDlg, IDC_PAUSE, L"重启");
                    }
                }
            }
            break;
        }

        case IDC_STOP:  // 停止按钮
        {
            if (g_CurrentClick) {
                KillTimer(hDlg, 1);
                g_TimerId = 0;
                g_CurrentClick = 0;
                g_IsPaused = false;
                SetDlgItemText(hDlg, IDC_PAUSE, L"暂停");
            }
            break;
        }
        }
        break;
    }

    case WM_TIMER:
    {
        if (wParam == 1) {
            if (g_CurrentClick == 1) {
                LeftClick();
            }
            else if (g_CurrentClick == 2) {
                RightClick();
            }
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hDlg, &ps);
        EndPaint(hDlg, &ps);
        return TRUE;
    }

    case WM_CLOSE:
        if (g_TimerId) {
            KillTimer(hDlg, g_TimerId);
        }
        EndDialog(hDlg, 0);
        return TRUE;

    default:
        return FALSE;
    }
    return TRUE;
}


// 入口点
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    // 创建基于资源的主对话框
    DialogBoxParamW(
        hInstance,
        MAKEINTRESOURCEW(IDD_MAIN_WINDOW),
        NULL,
        MainDlgProc,  // 新的对话框过程函数
        (LPARAM)0
    );
    return 0;
}


//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = { 0 };

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = MainDlgProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLICKMOUSE);
    wcex.lpszClassName  = szWindowClass;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLICKMOUSE));
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLICKMOUSE));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDUPDATE)
        {
            ShellExecuteW(NULL, L"open", L"https://github.com/xystudio889/ClickMouse/releases/latest", NULL, NULL, SW_SHOWNORMAL);
        }
        break;
    }
    return (INT_PTR)FALSE;
}
