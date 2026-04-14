#include "ChronoGraph.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {

    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_WIN95_CLASSES | ICC_BAR_CLASSES };
    InitCommonControlsEx(&icc);


    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = L"ChronoGraph3D";
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm       = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);


    HWND hwnd = CreateWindowExW(
        0,
        L"ChronoGraph3D",
        L"ChronoGraph 3D: Motor de Renderizado Matemático en Tiempo Real",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1280, 800,
        nullptr, nullptr, hInst, nullptr
    );
    if (!hwnd) return -1;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {

        if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
            SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(1001, BN_CLICKED), (LPARAM)nullptr);
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
