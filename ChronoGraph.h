#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <map>
#include <memory>


#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

// ─── IDs de controles ────────────────────────────────────────────────────────
#define ID_BTN_GRAPH        1001
#define ID_EDIT_FORMULA     1002
#define ID_COMBO_PRESETS    1003
#define ID_RADIO_CART2D     1004
#define ID_RADIO_POLAR2D    1005
#define ID_RADIO_CART3D     1006
#define ID_RADIO_POLAR3D    1007
#define ID_SLIDER_X         1008
#define ID_SLIDER_Y         1009
#define ID_SLIDER_Z         1010
#define ID_CHECK_AUTOROT    1011
#define ID_CHECK_ANIMTIME   1012
#define ID_CHECK_3DGLASS    1013
#define ID_TIMER_ANIM       2001
#define ID_TIMER_ROT        2002

// ─── Modo de grafica ─────────────────────────────────────────────────────────
enum GraphMode { MODE_CART2D, MODE_POLAR2D, MODE_CART3D, MODE_POLAR3D };

// ─── Punto 3D ────────────────────────────────────────────────────────────────
struct Vec3 { double x, y, z; };
struct Vec2 { double x, y; };

// ─── Preset ──────────────────────────────────────────────────────────────────
struct Preset {
    std::wstring name;
    std::wstring formula;
    GraphMode    mode;
};

// ─── Declaracion adelantada del parser/evaluador ─────────────────────────────
double EvaluateExpression(const std::wstring& expr, double x, double y, double z, double t);

// ─── Estado de la aplicacion ─────────────────────────────────────────────────
struct AppState {
    GraphMode   mode      = MODE_CART3D;
    std::wstring formula  = L"sin(sqrt(x^2 + z^2) - T*4)";
    double       T        = 0.0;
    double       rotX     = 25.0;   // grados
    double       rotY     = -35.0;
    double       rotZ     = 0.0;
    double       zoom     = 1.0;
    bool         autoRot  = false;
    bool         animTime = false;
    bool         glass3D  = false;
    // desplazamiento (pan)
    double       panX     = 0.0;
    double       panY     = 0.0;
    // arrastre con raton
    bool         dragging = false;
    int          lastMX   = 0, lastMY = 0;
    bool         panning  = false;
};

// ─── Procedimiento de ventana ────────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
