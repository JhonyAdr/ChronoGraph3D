#include "ChronoGraph.h"


void RenderScene(const AppState& st, HDC hdc, int W, int H);
extern AppState g_state;


static std::vector<Preset> g_presets = {
    {L"1. Gota en el estanque",  L"sin(sqrt(x^2 + z^2) - T*4)",         MODE_CART3D},
    {L"2. Ola sinusoidal",       L"sin(x - T*2) * cos(z)",               MODE_CART3D},
    {L"3. Silla de montar",      L"x^2/2 - z^2/2",                       MODE_CART3D},
    {L"4. Paraboloide",          L"x^2 + z^2",                           MODE_CART3D},
    {L"5. Toro ondulado",        L"sin(3*x)*cos(3*z)*0.5",               MODE_CART3D},
    {L"6. Espiral polar 3D",     L"sin(sqrt(x^2+z^2)*2 + atan2(z,x)*3 - T*2)", MODE_CART3D},
    {L"7. Cono senoidal",        L"sin(sqrt(x^2+z^2)*4)/( sqrt(x^2+z^2)+0.1)", MODE_CART3D},
    {L"8. Terreno fractal",      L"sin(x*2)*cos(z*2)*0.4 + sin(x*5+z*3)*0.15", MODE_CART3D},
    {L"9. Polar: Rosa",          L"cos(4*x)",                            MODE_POLAR2D},
    {L"10. Polar: Espiral",      L"x / (2*pi)",                          MODE_POLAR2D},
    {L"11. Polar: Cardiode",     L"1 + cos(x)",                          MODE_POLAR2D},
    {L"12. 2D: Senoidal animada",L"sin(x - T*3)",                        MODE_CART2D},
    {L"13. 2D: Suma armónicos",  L"sin(x)+sin(2*x)*0.5+sin(3*x)*0.33",  MODE_CART2D},
    {L"14. 2D: Onda cuadrada",   L"sign(sin(x*2))",                      MODE_CART2D},
    {L"15. Polar 3D: Hongo",     L"sin(sqrt(x^2+z^2))*exp(-0.3*sqrt(x^2+z^2)) - T*0.1", MODE_CART3D},
};


AppState g_state;
HWND g_hwnd     = nullptr;
HWND g_hCanvas  = nullptr; // ventana hija para renderizado
HWND g_hEdit    = nullptr;
HWND g_hCombo   = nullptr;
HWND g_hBtn     = nullptr;
HWND g_hR[4]    = {};
HWND g_hSlX=nullptr, g_hSlY=nullptr, g_hSlZ=nullptr;
HWND g_hChkRot=nullptr, g_hChkAnim=nullptr, g_hChk3D=nullptr;
HFONT g_hFont   = nullptr;
HFONT g_hFontSm = nullptr;

static const int TOOLBAR_H = 100; // altura del area superior de herramientas


static LRESULT CALLBACK CanvasProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        RenderScene(g_state, hdc, rc.right, rc.bottom);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_LBUTTONDOWN:
        g_state.dragging = true;
        g_state.panning  = false;
        g_state.lastMX = GET_X_LPARAM(lp);
        g_state.lastMY = GET_Y_LPARAM(lp);
        SetCapture(hwnd);
        return 0;
    case WM_RBUTTONDOWN:
        g_state.panning  = true;
        g_state.dragging = false;
        g_state.lastMX = GET_X_LPARAM(lp);
        g_state.lastMY = GET_Y_LPARAM(lp);
        SetCapture(hwnd);
        return 0;
    case WM_MOUSEMOVE:
        if (g_state.dragging && !g_state.panning) {
            int dx = GET_X_LPARAM(lp) - g_state.lastMX;
            int dy = GET_Y_LPARAM(lp) - g_state.lastMY;
            g_state.rotY += dx * 0.5;
            g_state.rotX += dy * 0.5;
            g_state.lastMX = GET_X_LPARAM(lp);
            g_state.lastMY = GET_Y_LPARAM(lp);
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        g_state.dragging = false;
        g_state.panning  = false;
        ReleaseCapture();
        return 0;
    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wp);
        g_state.zoom *= (delta > 0) ? 1.12 : (1.0/1.12);
        g_state.zoom = std::clamp(g_state.zoom, 0.05, 20.0);
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    }
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Crear controles de la interfaz
// ─────────────────────────────────────────────────────────────────────────────
static void CreateControls(HWND hwnd) {
    g_hFont   = CreateFontW(16,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
                    DEFAULT_PITCH,L"Segoe UI");
    g_hFontSm = CreateFontW(13,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
                    DEFAULT_PITCH,L"Segoe UI");

    // Campo de formula
    g_hEdit = CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",g_state.formula.c_str(),
        WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,
        10,10,390,26,hwnd,(HMENU)ID_EDIT_FORMULA,nullptr,nullptr);
    SendMessage(g_hEdit,WM_SETFONT,(WPARAM)g_hFont,TRUE);

    // Boton Graficar
    g_hBtn = CreateWindowW(L"BUTTON",L"Graficar",
        WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
        405,10,90,26,hwnd,(HMENU)ID_BTN_GRAPH,nullptr,nullptr);
    SendMessage(g_hBtn,WM_SETFONT,(WPARAM)g_hFont,TRUE);

    // Combo de presets
    g_hCombo = CreateWindowExW(0,L"COMBOBOX",nullptr,
        WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL,
        10,42,390,300,hwnd,(HMENU)ID_COMBO_PRESETS,nullptr,nullptr);
    SendMessage(g_hCombo,WM_SETFONT,(WPARAM)g_hFontSm,TRUE);
    for (auto& p : g_presets)
        SendMessageW(g_hCombo,CB_ADDSTRING,0,(LPARAM)p.name.c_str());
    SendMessage(g_hCombo,CB_SETCURSEL,0,0);

    // Botones de opcion: modo
    const wchar_t* rLabels[4] = {L"Cartesiano 2D",L"Polar 2D",L"Cartesiano 3D",L"Polar 3D"};
    int rIDs[4]={ID_RADIO_CART2D,ID_RADIO_POLAR2D,ID_RADIO_CART3D,ID_RADIO_POLAR3D};
    int rx=505;
    for (int i=0;i<4;i++) {
        int row=i/2, col=i%2;
        g_hR[i]=CreateWindowW(L"BUTTON",rLabels[i],
            WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON|(i==0?WS_GROUP:0),
            rx+col*120, 10+row*24, 118, 22,
            hwnd,(HMENU)rIDs[i],nullptr,nullptr);
        SendMessage(g_hR[i],WM_SETFONT,(WPARAM)g_hFontSm,TRUE);
    }
    // Por defecto: cartesiano 3D
    SendMessage(g_hR[2],BM_SETCHECK,BST_CHECKED,0);

    // Deslizadores X Y Z
    const wchar_t* slLabels[3]={L"X:",L"Y:",L"Z:"};
    HWND sliders[3];
    int sxL=760;
    for (int i=0;i<3;i++) {
        CreateWindowW(L"STATIC",slLabels[i],WS_CHILD|WS_VISIBLE,
            sxL,12+i*24,18,18,hwnd,nullptr,nullptr,nullptr);
        sliders[i]=CreateWindowW(TRACKBAR_CLASSW,nullptr,
            WS_CHILD|WS_VISIBLE|TBS_HORZ|TBS_NOTICKS,
            sxL+20,10+i*24,120,20,hwnd,(HMENU)(ID_SLIDER_X+i),nullptr,nullptr);
        SendMessage(sliders[i],TBM_SETRANGE,TRUE,MAKELPARAM(-100,100));
        SendMessage(sliders[i],TBM_SETPOS,TRUE,0);
    }
    g_hSlX=sliders[0]; g_hSlY=sliders[1]; g_hSlZ=sliders[2];

    // Casillas de verificacion
    int cxL=900;
    g_hChkRot=CreateWindowW(L"BUTTON",L"Auto-Rotación (Y)",
        WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
        cxL,10,170,20,hwnd,(HMENU)ID_CHECK_AUTOROT,nullptr,nullptr);
    g_hChkAnim=CreateWindowW(L"BUTTON",L"Animar Tiempo (T)",
        WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
        cxL,34,170,20,hwnd,(HMENU)ID_CHECK_ANIMTIME,nullptr,nullptr);
    g_hChk3D=CreateWindowW(L"BUTTON",L"Gafas 3D (Rojo/Cian)",
        WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
        cxL,58,180,20,hwnd,(HMENU)ID_CHECK_3DGLASS,nullptr,nullptr);
    SendMessage(g_hChkRot,WM_SETFONT,(WPARAM)g_hFontSm,TRUE);
    SendMessage(g_hChkAnim,WM_SETFONT,(WPARAM)g_hFontSm,TRUE);
    SendMessage(g_hChk3D,WM_SETFONT,(WPARAM)g_hFontSm,TRUE);

    // Ventana hija de dibujo (canvas)
    WNDCLASSEXW wcc = {};
    wcc.cbSize        = sizeof(wcc);
    wcc.lpszClassName = L"ChronoCanvas";
    wcc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcc.lpfnWndProc   = CanvasProc;
    wcc.hCursor       = LoadCursor(nullptr, IDC_CROSS);
    RegisterClassExW(&wcc);

    RECT rc; GetClientRect(hwnd,&rc);
    g_hCanvas=CreateWindowExW(WS_EX_CLIENTEDGE,L"ChronoCanvas",nullptr,
        WS_CHILD|WS_VISIBLE,
        0,TOOLBAR_H,rc.right,rc.bottom-TOOLBAR_H,
        hwnd,nullptr,nullptr,nullptr);

    // Temporizadores
    SetTimer(hwnd,ID_TIMER_ANIM,33,nullptr); // ~30fps
    SetTimer(hwnd,ID_TIMER_ROT,33,nullptr);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Disparar re-render de la grafica
// ─────────────────────────────────────────────────────────────────────────────
static void DoGraph(HWND /*hwnd*/) {
    wchar_t buf[512]={};
    GetWindowTextW(g_hEdit,buf,511);
    g_state.formula = buf;
    InvalidateRect(g_hCanvas,nullptr,FALSE);
}

static void ApplyPreset(int idx) {
    if (idx<0||idx>=(int)g_presets.size()) return;
    auto& p=g_presets[idx];
    g_state.formula = p.formula;
    g_state.mode    = p.mode;
    SetWindowTextW(g_hEdit,p.formula.c_str());
    // ajustar radio segun modo del preset
    int modeMap[4]={MODE_CART2D,MODE_POLAR2D,MODE_CART3D,MODE_POLAR3D};
    for (int i=0;i<4;i++)
        SendMessage(g_hR[i],BM_SETCHECK,(modeMap[i]==p.mode)?BST_CHECKED:BST_UNCHECKED,0);
    InvalidateRect(g_hCanvas,nullptr,FALSE);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        InitCommonControls();
        CreateControls(hwnd);
        ApplyPreset(0);
        return 0;

    case WM_SIZE: {
        RECT rc; GetClientRect(hwnd,&rc);
        if (g_hCanvas)
            MoveWindow(g_hCanvas,0,TOOLBAR_H,rc.right,rc.bottom-TOOLBAR_H,TRUE);
        return 0;
    }

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORSTATIC: {
        HDC hdc=(HDC)wp;
        SetTextColor(hdc,RGB(200,210,220));
        SetBkColor(hdc,RGB(28,30,38));
        static HBRUSH hbr=CreateSolidBrush(RGB(28,30,38));
        return (LRESULT)hbr;
    }
    case WM_CTLCOLOREDIT: {
        HDC hdc=(HDC)wp;
        SetTextColor(hdc,RGB(50,230,120));
        SetBkColor(hdc,RGB(20,22,30));
        static HBRUSH hbr=CreateSolidBrush(RGB(20,22,30));
        return (LRESULT)hbr;
    }
    case WM_CTLCOLORLISTBOX: {
        HDC hdc=(HDC)wp;
        SetTextColor(hdc,RGB(200,210,220));
        SetBkColor(hdc,RGB(25,27,35));
        static HBRUSH hbr=CreateSolidBrush(RGB(25,27,35));
        return (LRESULT)hbr;
    }

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case ID_BTN_GRAPH:
            DoGraph(hwnd);
            break;
        case ID_RADIO_CART2D:  g_state.mode=MODE_CART2D;  InvalidateRect(g_hCanvas,nullptr,FALSE); break;
        case ID_RADIO_POLAR2D: g_state.mode=MODE_POLAR2D; InvalidateRect(g_hCanvas,nullptr,FALSE); break;
        case ID_RADIO_CART3D:  g_state.mode=MODE_CART3D;  InvalidateRect(g_hCanvas,nullptr,FALSE); break;
        case ID_RADIO_POLAR3D: g_state.mode=MODE_POLAR3D; InvalidateRect(g_hCanvas,nullptr,FALSE); break;
        case ID_CHECK_AUTOROT:
            g_state.autoRot=SendMessage(g_hChkRot,BM_GETCHECK,0,0)==BST_CHECKED;
            break;
        case ID_CHECK_ANIMTIME:
            g_state.animTime=SendMessage(g_hChkAnim,BM_GETCHECK,0,0)==BST_CHECKED;
            break;
        case ID_CHECK_3DGLASS:
            g_state.glass3D=SendMessage(g_hChk3D,BM_GETCHECK,0,0)==BST_CHECKED;
            InvalidateRect(g_hCanvas,nullptr,FALSE);
            break;
        case ID_COMBO_PRESETS:
            if (HIWORD(wp)==CBN_SELCHANGE)
                ApplyPreset((int)SendMessage(g_hCombo,CB_GETCURSEL,0,0));
            break;

        case ID_EDIT_FORMULA:
            if (HIWORD(wp)==EN_CHANGE) {} // actualizacion en vivo (opcional)
            break;
        }
        return 0;

    case WM_KEYDOWN:
        if (wp==VK_RETURN) DoGraph(hwnd);
        return 0;

    case WM_HSCROLL: {
        // Deslizadores
        HWND hSlider=(HWND)lp;
        auto getSlVal=[&](HWND h){ return (double)SendMessage(h,TBM_GETPOS,0,0)/10.0; };
        if (hSlider==g_hSlX) g_state.rotX = getSlVal(g_hSlX)*18.0;
        if (hSlider==g_hSlY) g_state.rotY = getSlVal(g_hSlY)*18.0;
        if (hSlider==g_hSlZ) g_state.rotZ = getSlVal(g_hSlZ)*18.0;
        InvalidateRect(g_hCanvas,nullptr,FALSE);
        return 0;
    }

    case WM_TIMER:
        if (wp==ID_TIMER_ANIM) {
            bool dirty=false;
            if (g_state.animTime) { g_state.T+=0.05; dirty=true; }
            if (g_state.autoRot)  { g_state.rotY+=0.8; dirty=true; }
            if (dirty) InvalidateRect(g_hCanvas,nullptr,FALSE);
        }
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc=BeginPaint(hwnd,&ps);
        // Dibujar fondo oscuro de la barra superior
        RECT toolbarRc={0,0,10000,TOOLBAR_H};
        HBRUSH hbr=CreateSolidBrush(RGB(22,24,32));
        FillRect(hdc,&toolbarRc,hbr);
        DeleteObject(hbr);
        // Borde superior
        HPEN linePen=CreatePen(PS_SOLID,1,RGB(50,55,70));
        HPEN old=(HPEN)SelectObject(hdc,linePen);
        MoveToEx(hdc,0,TOOLBAR_H-1,nullptr);
        LineTo(hdc,10000,TOOLBAR_H-1);
        SelectObject(hdc,old);
        DeleteObject(linePen);
        // Etiquetas de deslizadores
        SetTextColor(hdc,RGB(140,150,170)); SetBkMode(hdc,TRANSPARENT);
        SelectObject(hdc,(HGDIOBJ)g_hFontSm);
        // el pintado de etiquetas estaticas lo manejan los controles hijos
        EndPaint(hwnd,&ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd,ID_TIMER_ANIM);
        KillTimer(hwnd,ID_TIMER_ROT);
        if (g_hFont)   DeleteObject(g_hFont);
        if (g_hFontSm) DeleteObject(g_hFontSm);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}
