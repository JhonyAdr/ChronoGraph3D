#include "ChronoGraph.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Utilidades de matriz / vector
// ─────────────────────────────────────────────────────────────────────────────
struct Mat4 {
    double m[4][4] = {};
    static Mat4 Identity() {
        Mat4 r; r.m[0][0]=r.m[1][1]=r.m[2][2]=r.m[3][3]=1; return r;
    }
    static Mat4 RotX(double deg) {
        double rad = deg * M_PI / 180.0;
        Mat4 r = Identity();
        r.m[1][1] =  std::cos(rad); r.m[1][2] = -std::sin(rad);
        r.m[2][1] =  std::sin(rad); r.m[2][2] =  std::cos(rad);
        return r;
    }
    static Mat4 RotY(double deg) {
        double rad = deg * M_PI / 180.0;
        Mat4 r = Identity();
        r.m[0][0] =  std::cos(rad); r.m[0][2] =  std::sin(rad);
        r.m[2][0] = -std::sin(rad); r.m[2][2] =  std::cos(rad);
        return r;
    }
    static Mat4 RotZ(double deg) {
        double rad = deg * M_PI / 180.0;
        Mat4 r = Identity();
        r.m[0][0] =  std::cos(rad); r.m[0][1] = -std::sin(rad);
        r.m[1][0] =  std::sin(rad); r.m[1][1] =  std::cos(rad);
        return r;
    }
    Mat4 operator*(const Mat4& o) const {
        Mat4 r;
        for (int i=0;i<4;i++) for (int j=0;j<4;j++)
            for (int k=0;k<4;k++) r.m[i][j] += m[i][k]*o.m[k][j];
        return r;
    }
    Vec3 transform(Vec3 v) const {
        double x = m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z + m[0][3];
        double y = m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z + m[1][3];
        double z = m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z + m[2][3];
        return {x,y,z};
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  Renderizador
// ─────────────────────────────────────────────────────────────────────────────
static COLORREF LerpColor(COLORREF a, COLORREF b, double t) {
    int r = (int)(GetRValue(a)+(GetRValue(b)-GetRValue(a))*t);
    int g = (int)(GetGValue(a)+(GetGValue(b)-GetGValue(a))*t);
    int bv= (int)(GetBValue(a)+(GetBValue(b)-GetBValue(a))*t);
    return RGB(r,g,bv);
}

// Gradiente de color: oscuro->brillante segun la altura en Y
static COLORREF HeightColor(double norm, bool glass3D, bool isRight) {
    // norm en [0,1]
    COLORREF dark  = glass3D ? (isRight ? RGB(0,60,120)  : RGB(120,0,0))
                             : RGB(0, 60, 0);
    COLORREF mid   = glass3D ? (isRight ? RGB(0,120,220) : RGB(200,0,0))
                             : RGB(0, 160, 0);
    COLORREF light = glass3D ? (isRight ? RGB(100,200,255): RGB(255,100,100))
                             : RGB(80, 255, 80);
    if (norm < 0.5) return LerpColor(dark,  mid,   norm*2.0);
    else            return LerpColor(mid,   light, (norm-0.5)*2.0);
}

struct Renderer {
    HBITMAP  hbm  = nullptr;
    HDC      hdc  = nullptr;
    int      W=0, H=0;
    HPEN     hpen = nullptr;
    HPEN     hpenGrid = nullptr;
    HBRUSH   hbrBg = nullptr;

    void Init(HDC hdcScreen, int w, int h) {
        if (hbm) { DeleteObject(hbm); DeleteDC(hdc); }
        W=w; H=h;
        hdc  = CreateCompatibleDC(hdcScreen);
        hbm  = CreateCompatibleBitmap(hdcScreen, w, h);
        SelectObject(hdc, hbm);
        if (hpen)     DeleteObject(hpen);
        if (hpenGrid) DeleteObject(hpenGrid);
        if (hbrBg)    DeleteObject(hbrBg);
        hpen     = CreatePen(PS_SOLID, 1, RGB(0,200,0));
        hpenGrid = CreatePen(PS_SOLID, 1, RGB(50,50,60));
        hbrBg    = CreateSolidBrush(RGB(18,18,24));
    }
    void Clear() {
        RECT rc = {0,0,W,H};
        FillRect(hdc, &rc, hbrBg);
    }
    void Present(HDC dst) {
        BitBlt(dst,0,0,W,H,hdc,0,0,SRCCOPY);
    }
    ~Renderer() {
        if (hbm)      DeleteObject(hbm);
        if (hdc)      DeleteDC(hdc);
        if (hpen)     DeleteObject(hpen);
        if (hpenGrid) DeleteObject(hpenGrid);
        if (hbrBg)    DeleteObject(hbrBg);
    }
};

static Renderer g_renderer;

// ─────────────────────────────────────────────────────────────────────────────
//  Proyeccion 3D->2D (perspectiva simple)
// ─────────────────────────────────────────────────────────────────────────────
static POINT Project(Vec3 v, int W, int H, double zoom) {
    double fov = 400.0 * zoom;
    double zOff = 5.0;
    double px = v.x * fov / (v.z + zOff) + W*0.5;
    double py = -v.y * fov / (v.z + zOff) + H*0.5;
    return {(LONG)px, (LONG)py};
}

// ─────────────────────────────────────────────────────────────────────────────
//  Dibuja ejes de referencia en 3D
// ─────────────────────────────────────────────────────────────────────────────
static void DrawAxes3D(HDC hdc, const Mat4& mvp, int W, int H, double zoom) {
    auto proj = [&](double x, double y, double z) {
        return Project(mvp.transform({x,y,z}), W, H, zoom);
    };
    struct AxisDef { Vec3 from, to; COLORREF col; const wchar_t* label; };
    AxisDef axes[] = {
        {{0,0,0},{2,0,0}, RGB(220,60,60),  L"X"},
        {{0,0,0},{0,2,0}, RGB(60,220,60),  L"Y"},
        {{0,0,0},{0,0,2}, RGB(60,120,220), L"Z"},
    };
    for (auto& ax : axes) {
        HPEN p = CreatePen(PS_SOLID, 2, ax.col);
        HPEN old = (HPEN)SelectObject(hdc, p);
        POINT a = proj(ax.from.x,ax.from.y,ax.from.z);
        POINT b = proj(ax.to.x,  ax.to.y,  ax.to.z);
        MoveToEx(hdc,a.x,a.y,nullptr);
        LineTo(hdc,b.x,b.y);
        SelectObject(hdc,old);
        DeleteObject(p);
        // etiqueta
        SetTextColor(hdc, ax.col);
        SetBkMode(hdc, TRANSPARENT);
        TextOutW(hdc, b.x+4, b.y-8, ax.label, 1);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Render de superficie cartesiana 3D: y = f(x, z, T)
// ─────────────────────────────────────────────────────────────────────────────
void Render3DCartesian(const AppState& st, int W, int H) {
    HDC hdc = g_renderer.hdc;
    int N = 60; // resolucion de malla
    double range = 3.14159;
    double step  = range*2.0 / N;

    Mat4 mvp = Mat4::RotX(st.rotX) * Mat4::RotY(st.rotY) * Mat4::RotZ(st.rotZ);

    // construir malla de alturas
    std::vector<std::vector<double>> grid(N+1, std::vector<double>(N+1));
    double ymin=1e18, ymax=-1e18;
    for (int iz=0; iz<=N; iz++) {
        double z = -range + iz*step;
        for (int ix=0; ix<=N; ix++) {
            double x = -range + ix*step;
            double y = EvaluateExpression(st.formula, x, 0, z, st.T);
            if (!std::isfinite(y)) y = 0;
            grid[iz][ix] = y;
            ymin = std::min(ymin, y);
            ymax = std::max(ymax, y);
        }
    }
    double yrange = ymax - ymin;
    if (yrange < 1e-9) yrange = 1.0;

    auto colorOf = [&](double y, bool right) -> COLORREF {
        double norm = (y - ymin) / yrange;
        return HeightColor(std::clamp(norm,0.0,1.0), st.glass3D, right);
    };
    auto proj = [&](double x, double y, double z) {
        Vec3 v = mvp.transform({x, y, z});
        return Project(v, W, H, st.zoom);
    };

    // dibujar cuadrilateros de la malla (wireframe coloreado por altura)
    for (int iz=0; iz<N; iz++) {
        for (int ix=0; ix<N; ix++) {
            double x0 = -range + ix*step,     x1 = x0+step;
            double z0 = -range + iz*step,     z1 = z0+step;
            double y00=grid[iz][ix],   y10=grid[iz][ix+1];
            double y01=grid[iz+1][ix], y11=grid[iz+1][ix+1];
            double yAvg = (y00+y10+y01+y11)*0.25;

            COLORREF col = colorOf(yAvg, false);
            if (st.glass3D) {
                // desplazamiento para ojo izquierdo (canal rojo)
                // ojo derecho (cian) dibujado en segunda pasada con desplazamiento
                col = colorOf(yAvg, false);
            }

            HPEN pen = CreatePen(PS_SOLID, 1, col);
            HPEN old = (HPEN)SelectObject(hdc, pen);

            POINT p00 = proj(x0, y00, z0);
            POINT p10 = proj(x1, y10, z0);
            POINT p01 = proj(x0, y01, z1);
            POINT p11 = proj(x1, y11, z1);

            // dibujar dos triangulos como bordes wireframe
            MoveToEx(hdc, p00.x, p00.y, nullptr);
            LineTo(hdc,   p10.x, p10.y);
            LineTo(hdc,   p11.x, p11.y);
            LineTo(hdc,   p01.x, p01.y);
            LineTo(hdc,   p00.x, p00.y);
            LineTo(hdc,   p11.x, p11.y);

            SelectObject(hdc, old);
            DeleteObject(pen);

            // segunda pasada para efecto gafas 3D (desplazamiento cian)
            if (st.glass3D) {
                COLORREF colR = colorOf(yAvg, true);
                HPEN penR = CreatePen(PS_SOLID, 1, colR);
                old = (HPEN)SelectObject(hdc, penR);
                int off = 4;
                POINT q00={p00.x+off,p00.y}, q10={p10.x+off,p10.y};
                POINT q01={p01.x+off,p01.y}, q11={p11.x+off,p11.y};
                MoveToEx(hdc, q00.x, q00.y, nullptr);
                LineTo(hdc, q10.x, q10.y);
                LineTo(hdc, q11.x, q11.y);
                LineTo(hdc, q01.x, q01.y);
                LineTo(hdc, q00.x, q00.y);
                LineTo(hdc, q11.x, q11.y);
                SelectObject(hdc, old);
                DeleteObject(penR);
            }
        }
    }
    DrawAxes3D(hdc, mvp, W, H, st.zoom);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Render de superficie polar 3D: y = f(r, theta, T)
// ─────────────────────────────────────────────────────────────────────────────
void Render3DPolar(const AppState& st, int W, int H) {
    HDC hdc = g_renderer.hdc;
    int NR=40, NT=80;
    double rMax = 3.14159;
    Mat4 mvp = Mat4::RotX(st.rotX) * Mat4::RotY(st.rotY);

    std::vector<std::vector<double>> grid(NR+1, std::vector<double>(NT+1));
    std::vector<std::vector<double>> gx(NR+1, std::vector<double>(NT+1));
    std::vector<std::vector<double>> gz(NR+1, std::vector<double>(NT+1));
    double ymin=1e18, ymax=-1e18;
    for (int ir=0; ir<=NR; ir++) {
        double r = ir*(rMax/NR);
        for (int it=0; it<=NT; it++) {
            double theta = it*(2*M_PI/NT);
            double x = r*std::cos(theta), z = r*std::sin(theta);
            double y = EvaluateExpression(st.formula, x, 0, z, st.T);
            if (!std::isfinite(y)) y=0;
            grid[ir][it]=y; gx[ir][it]=x; gz[ir][it]=z;
            ymin=std::min(ymin,y); ymax=std::max(ymax,y);
        }
    }
    double yrange = ymax-ymin; if (yrange<1e-9) yrange=1.0;
    auto proj = [&](double x, double y, double z) {
        return Project(mvp.transform({x,y,z}), W, H, st.zoom);
    };
    for (int ir=0; ir<NR; ir++) for (int it=0; it<NT; it++) {
        double yAvg=(grid[ir][it]+grid[ir][it+1]+grid[ir+1][it]+grid[ir+1][it+1])*0.25;
        double norm=std::clamp((yAvg-ymin)/yrange,0.0,1.0);
        COLORREF col = HeightColor(norm, st.glass3D, false);
        HPEN pen=CreatePen(PS_SOLID,1,col), old=(HPEN)SelectObject(hdc,pen);
        POINT p00=proj(gx[ir][it],  grid[ir][it],   gz[ir][it]);
        POINT p10=proj(gx[ir][it+1],grid[ir][it+1], gz[ir][it+1]);
        POINT p01=proj(gx[ir+1][it],grid[ir+1][it], gz[ir+1][it]);
        POINT p11=proj(gx[ir+1][it+1],grid[ir+1][it+1],gz[ir+1][it+1]);
        MoveToEx(hdc,p00.x,p00.y,nullptr);
        LineTo(hdc,p10.x,p10.y); LineTo(hdc,p11.x,p11.y);
        LineTo(hdc,p01.x,p01.y); LineTo(hdc,p00.x,p00.y);
        SelectObject(hdc,old); DeleteObject(pen);
    }
    DrawAxes3D(hdc, mvp, W, H, st.zoom);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Render cartesiano 2D: y = f(x, T)
// ─────────────────────────────────────────────────────────────────────────────
void Render2DCartesian(const AppState& st, int W, int H) {
    HDC hdc = g_renderer.hdc;
    double xMin=-6.28, xMax=6.28;
    double yMin=-3, yMax=3;
    int N = W;

    // dibujar cuadricula
    {
        HPEN gpen = CreatePen(PS_SOLID,1,RGB(40,45,60));
        HPEN old=(HPEN)SelectObject(hdc,gpen);
        for (int gx=-6;gx<=6;gx++) {
            int px=(int)((gx-xMin)/(xMax-xMin)*W);
            MoveToEx(hdc,px,0,nullptr); LineTo(hdc,px,H);
        }
        for (int gy=-3;gy<=3;gy++) {
            int py=(int)((1-(gy-yMin)/(yMax-yMin))*H);
            MoveToEx(hdc,0,py,nullptr); LineTo(hdc,W,py);
        }
        // ejes
        SelectObject(hdc,old); DeleteObject(gpen);
        HPEN apen=CreatePen(PS_SOLID,2,RGB(80,90,110));
        old=(HPEN)SelectObject(hdc,apen);
        int ax0=(int)((0-xMin)/(xMax-xMin)*W);
        int ay0=(int)((1-(0-yMin)/(yMax-yMin))*H);
        MoveToEx(hdc,ax0,0,nullptr); LineTo(hdc,ax0,H);
        MoveToEx(hdc,0,ay0,nullptr); LineTo(hdc,W,ay0);
        SelectObject(hdc,old); DeleteObject(apen);
    }

    // dibujar curva
    HPEN cpen=CreatePen(PS_SOLID,2,RGB(0,220,120));
    HPEN old=(HPEN)SelectObject(hdc,cpen);
    bool first=true;
    for (int i=0;i<N;i++) {
        double x=xMin+(xMax-xMin)*i/N;
        double y=EvaluateExpression(st.formula, x, 0, 0, st.T);
        if (!std::isfinite(y)) { first=true; continue; }
        int px=i;
        int py=(int)((1-(y-yMin)/(yMax-yMin))*H);
        if (first) { MoveToEx(hdc,px,py,nullptr); first=false; }
        else LineTo(hdc,px,py);
    }
    SelectObject(hdc,old); DeleteObject(cpen);

    // etiquetas
    SetTextColor(hdc,RGB(100,110,130)); SetBkMode(hdc,TRANSPARENT);
    SetTextColor(hdc, RGB(80,200,255));
    std::wstring info = L"y = f(x,T)   T=" + std::to_wstring(st.T).substr(0,5);
    TextOutW(hdc,8,8,info.c_str(),(int)info.size());
}

// ─────────────────────────────────────────────────────────────────────────────
//  Render polar 2D: r = f(theta, T)
// ─────────────────────────────────────────────────────────────────────────────
void Render2DPolar(const AppState& st, int W, int H) {
    HDC hdc = g_renderer.hdc;
    int cx=W/2, cy=H/2;
    double scale=std::min(W,H)*0.35;

    // circulos de referencia
    HPEN gpen=CreatePen(PS_SOLID,1,RGB(40,45,60));
    HPEN old=(HPEN)SelectObject(hdc,gpen);
    for (int r=1;r<=4;r++) {
        int rr=(int)(r*scale*0.25);
        Ellipse(hdc,cx-rr,cy-rr,cx+rr,cy+rr);
    }
    // lineas de ejes radiales
    for (int a=0;a<360;a+=30) {
        double rad=a*M_PI/180.0;
        MoveToEx(hdc,cx,cy,nullptr);
        LineTo(hdc,(int)(cx+std::cos(rad)*scale),(int)(cy-std::sin(rad)*scale));
    }
    SelectObject(hdc,old); DeleteObject(gpen);

    int N=720;
    HPEN cpen=CreatePen(PS_SOLID,2,RGB(0,220,120));
    old=(HPEN)SelectObject(hdc,cpen);
    bool first=true;
    for (int i=0;i<=N;i++) {
        double theta=i*(2*M_PI/N);
        double r=EvaluateExpression(st.formula, theta, 0, 0, st.T);
        if (!std::isfinite(r)) { first=true; continue; }
        int px=(int)(cx+r*std::cos(theta)*scale*0.25);
        int py=(int)(cy-r*std::sin(theta)*scale*0.25);
        if (first) { MoveToEx(hdc,px,py,nullptr); first=false; }
        else LineTo(hdc,px,py);
    }
    SelectObject(hdc,old); DeleteObject(cpen);

    SetTextColor(hdc,RGB(80,200,255)); SetBkMode(hdc,TRANSPARENT);
    std::wstring info=L"r = f(θ,T)   T="+std::to_wstring(st.T).substr(0,5);
    TextOutW(hdc,8,8,info.c_str(),(int)info.size());
}

// ─────────────────────────────────────────────────────────────────────────────
//  Despachador principal de render
// ─────────────────────────────────────────────────────────────────────────────
void RenderScene(const AppState& st, HDC hdcScreen, int W, int H) {
    g_renderer.Init(hdcScreen, W, H);
    g_renderer.Clear();
    switch (st.mode) {
        case MODE_CART2D:  Render2DCartesian(st, W, H); break;
        case MODE_POLAR2D: Render2DPolar(st, W, H);     break;
        case MODE_CART3D:  Render3DCartesian(st, W, H); break;
        case MODE_POLAR3D: Render3DPolar(st, W, H);     break;
    }
    g_renderer.Present(hdcScreen);
}
