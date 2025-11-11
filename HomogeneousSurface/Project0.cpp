#include <windows.h> 
#include <GL/gl.h> 
#pragma comment(lib,"OpenGL32.lib") 
#include <GL/glu.h> 
#pragma comment(lib,"Glu32.lib") 
#pragma comment (lib, "legacy_stdio_definitions.lib") 
#include "GL/glaux.h" 
#pragma comment(lib,"Glaux.lib") 

#include "CoordinateAxes.hpp"
#include "Background.hpp"

#include "LineSegmentConeLit.hpp"
#include "CircleConeLit.hpp"
#include "ParabolaConeLit.hpp"

static CoordinateAxes gAxes;
static Background gBackground;

static LineSegmentConeLit gLineCone(12, 8, 3.0f); // levels, segments, depth
static CircleConeLit gCircle(12, 24, 3.0f);
static ParabolaConeLit gParabola(12, 8, 3.0f);

// ==== [globals for Subiectul 2] ==============================================
struct P2 { float y, z; };        // we'll place curve in plane X = const -> (X, Y, Z) = (xPlane, y, z)
static const P2 B0 = { -3.f,  1.f };
static const P2 B1 = { 1.f, -1.f };
static const P2 B2 = { 1.f,  2.f };
static const P2 B3 = { -2.f, -1.f };

int   gNSeg = 80;                 // segments along curve
int   gNLev = 48;                 // levels along "height" (extrusion direction)
float gXPlane = 2.5f;             // plane X = 2.5
float gHeight = 6.0f;             // total length of the cylindrical surface along X

static inline P2 bezier(float t) {
    float u = 1.0f - t;
    float b0 = u * u * u;
    float b1 = 3.0f * u * u * t;
    float b2 = 3.0f * u * t * t;
    float b3 = t * t * t;
    return { b0 * B0.y + b1 * B1.y + b2 * B2.y + b3 * B3.y,
             b0 * B0.z + b1 * B1.z + b2 * B2.z + b3 * B3.z };
}

static inline P2 bezierDeriv(float t) {           // derivative for lighting
    float u = 1.0f - t;
    // 3[(P1-P0)u^2 + 2(P2-P1)ut + (P3-P2)t^2]
    float dy = 3.0f * ((B1.y - B0.y) * u * u + 2.0f * (B2.y - B1.y) * u * t + (B3.y - B2.y) * t * t);
    float dz = 3.0f * ((B1.z - B0.z) * u * u + 2.0f * (B2.z - B1.z) * u * t + (B3.z - B2.z) * t * t);
    return { dy, dz };
}

static inline void norm3f(float& x, float& y, float& z) {    // normalize
    float l = std::sqrt(x * x + y * y + z * z); if (l < 1e-6f) { x = 0;y = 0;z = 1; return; }
    x /= l; y /= l; z /= l;
}

void drawBezierCurve_Xconst()
{
    glDisable(GL_LIGHTING);
    glLineWidth(2.f);
    glColor3f(0.f, 0.f, 0.f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= gNSeg; ++i) {
        float t = (float)i / gNSeg;
        P2 p = bezier(t);
        glVertex3f(gXPlane, p.y, p.z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawCylSurfaceFromBezier()
{
    // S(s,t) = (xPlane + s*H - H/2,  By(t),  Bz(t)),  s∈[0,1], t∈[0,1]
    glColor4f(0.75f, 0.75f, 0.9f, 0.95f);

    for (int h = 0; h < gNLev; ++h) {
        float s0 = (float)h / gNLev;
        float s1 = (float)(h + 1) / gNLev;
        float x0 = gXPlane + s0 * gHeight - gHeight * 0.5f;
        float x1 = gXPlane + s1 * gHeight - gHeight * 0.5f;

        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= gNSeg; ++i) {
            float t = (float)i / gNSeg;
            P2 p = bezier(t);
            P2 d = bezierDeriv(t);               // tangent along curve (0, dy, dz)
            // normal = cross( (1,0,0), (0,dy,dz) ) = (0, -dz, dy)
            float nx = 0.f, ny = -d.z, nz = d.y; norm3f(nx, ny, nz);
            glNormal3f(nx, ny, nz);
            glVertex3f(x0, p.y, p.z);
            glVertex3f(x1, p.y, p.z);
        }
        glEnd();
    }
}

// ---- анимация вращения сцены ----
static bool   gSpinEnabled = true;     // включить/выключить авто-вращение
static float  gYawDeg = -35.0f;   // поворот вокруг Y (горизонт)
static float  gPitchDeg = 35.0f;    // поворот вокруг X (наклон)
static float  gYawSpeedDps = 20.0f;    // скорость по Y, градусов в секунду
static DWORD  gLastTick = 0;        // для дельта-времени
static float gKeyYawSpeedDps = 90.0f;   // скорость поворота по Y от стрелок
static float gKeyPitchSpeedDps = 90.0f;   // скорость поворота по X от стрелок
static float gCamDist = 6.0f;    // дистанция "камеры" (Translate z)

static float gZoom = 1.0f;      // 1.0 — без зума, >1 — отдаление, <1 — приближение
static int   gWinW = 500, gWinH = 500; // запомним размер окна

static void applyProjection()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // уменьшаем/увеличиваем объём видимости по XY пропорционально gZoom
    const float base = 6.2f;
    const float s = gZoom; // zoom scale
    glOrtho(-base / s, base / s, -base / s, base / s, 2.0, 12.0);

    glMatrixMode(GL_MODELVIEW);
}

void initScene() {
	gAxes.setLength(5.5f);
	gAxes.setArrow(0.1f, 0.2f);
	gAxes.setLineWidth(1.5f);
	gAxes.setShowOriginPoint(true);
	gAxes.setLineSmooth(true);

	if (!gBackground.loadFromExeDir("background_lion.bmp")) {
		MessageBoxA(nullptr, "Can't load background_lion.bmp", "Warning", MB_OK | MB_ICONWARNING);
	}

	gLineCone.build();
    gLineCone.setWireframe(true);
    gLineCone.loadTextures("wood.bmp", "asphalt.bmp");

	gCircle.build();
    gCircle.setWireframe(true);

	gParabola.build();
    gParabola.setWireframe(true);

    gLastTick = GetTickCount();
}

void CALLBACK resize(int width, int height)
{
	// Здесь указывается часть окна в пределах которой 
	// будут рисовать функции OpenGL. 
	GLuint wp = width < height ? width - 20 : height - 20;
	glViewport(10, 10, wp, wp);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Устанавливаем тип проекции 
	// glOrtho - параллельная 
	// glFrustum - перспектива 
	// Параметры о обеих функции идентичны: 
	// они задают объем видимости 
	glOrtho(-6.2, 6.2, -6.2, 6.2, 2.0, 12.0);
	//   glFrustum(-5.0, 5.0, -5.0, 5.0, 2.0, 12.0);  

	glMatrixMode(GL_MODELVIEW);

    applyProjection(); // проекция теперь зависит от gZoom
}

void CALLBACK display(void)
{
    // очистка
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- 1) фон (в экранных координатах, сам чинит свои стейты) ---
    gBackground.draw();

    // --- 2) тайминг / анимация ---
    DWORD now = GetTickCount();
    float dt = (now - gLastTick) * 0.001f; // сек
    gLastTick = now;

    if (gSpinEnabled) {
        gYawDeg += gYawSpeedDps * dt;
        if (gYawDeg > 360.f)  gYawDeg -= 360.f;
        if (gYawDeg < -360.f) gYawDeg += 360.f;
    }

    // --- 3) обработка клавиш (стрелки, PgUp/PgDn, Space, R) ---
    // Yaw (влево/вправо)
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) gYawDeg -= gKeyYawSpeedDps * dt;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) gYawDeg += gKeyYawSpeedDps * dt;
    // Pitch (вверх/вниз)
    if (GetAsyncKeyState(VK_UP) & 0x8000) gPitchDeg -= gKeyPitchSpeedDps * dt;
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) gPitchDeg += gKeyPitchSpeedDps * dt;
    if (gPitchDeg > 89.f) gPitchDeg = 89.f;
    if (gPitchDeg < -89.f) gPitchDeg = -89.f;
    // Зум ( + / - ) : VK_OEM_PLUS/ADD и VK_OEM_MINUS/SUBTRACT
    bool plusDown = (GetAsyncKeyState(VK_OEM_PLUS) & 0x8000) || (GetAsyncKeyState(VK_ADD) & 0x8000);
    bool minusDown = (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000) || (GetAsyncKeyState(VK_SUBTRACT) & 0x8000);

    // экспоненциальный, плавный зум: 5% в сек/клавиша
    if (plusDown)  gZoom *= (1.0f + 0.05f * dt * 60.0f / 60.0f);   // приблизить (меньше фрустум)
    if (minusDown) gZoom /= (1.0f + 0.05f * dt * 60.0f / 60.0f);   // отдалить  (больше фрустум)

    // ограничения
    if (gZoom < 0.3f) gZoom = 0.3f;
    if (gZoom > 4.0f) gZoom = 4.0f;

    // применим проекцию с новым зумом (фон уже отрисован и восстановил матрицы)
    applyProjection();
    // Пауза автоворота
    if (GetAsyncKeyState(VK_SPACE) & 0x0001) gSpinEnabled = !gSpinEnabled;
    // Сброс ориентации
    if (GetAsyncKeyState('R') & 0x0001) { gYawDeg = -35.f; gPitchDeg = 35.f; }

    // --- 4) 3D-сцена ---
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // камера с учётом зума
    glTranslated(0.0, 0.0, -gCamDist);

    // вращение всей сцены
    glRotatef(gPitchDeg, 1.0f, 0.0f, 0.0f);
    glRotatef(gYawDeg, 0.0f, 1.0f, 0.0f);

    // позиции источников света (после установки MODELVIEW!)
    {
        GLfloat pos0[4] = { 3.f,  3.f,  3.f, 1.f };
        GLfloat pos1[4] = { -3.f,  3.f, -3.f, 1.f };
        GLfloat pos2[4] = { 0.f, -3.f,  3.f, 1.f };
        glLightfv(GL_LIGHT0, GL_POSITION, pos0);
        glLightfv(GL_LIGHT1, GL_POSITION, pos1);
        glLightfv(GL_LIGHT2, GL_POSITION, pos2);
    }

    // ключевые состояния на всякий случай (фон мог менять)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // рисуем оси и фигуры
    gAxes.draw();
    //gLineCone.draw();
    //gCircle.draw();
    //gParabola.draw();
    drawCylSurfaceFromBezier();   // поверхность B
    drawBezierCurve_Xconst();     // отображение сечения (кривая) в плоскости X=2.5

    glFlush();
    glPopMatrix();

    // двойная буферизация
    auxSwapBuffers();
}


int main()
{
    // Окно 500x500 в левом верхнем углу
    auxInitPosition(0, 0, 500, 500);

    // Контекст: RGB + Z-буфер + двойная буферизация
    auxInitDisplayMode(AUX_RGB | AUX_DEPTH | AUX_DOUBLE);

    // Создать окно
    auxInitWindow(L"OpenGL");

    // Инициализация сцены (фон, оси, геометрия и т.д.)
    initScene();

    // Коллбэки
    auxIdleFunc(display);
    auxReshapeFunc(resize);

    // Глобальные GL-состояния
    glEnable(GL_DEPTH_TEST);           // Z-тест
    glEnable(GL_BLEND);                // прозрачность в фигурах
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_COLOR_MATERIAL);       // материал берём из glColor
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    glShadeModel(GL_SMOOTH);           // Gouraud
    glEnable(GL_NORMALIZE);            // корректные нормали при масштабах

    glDisable(GL_CULL_FACE);                          // рисуем обе стороны
    glShadeModel(GL_SMOOTH);

    glClearColor(1.f, 1.f, 1.f, 1.f);  // фон окна (на случай, если нет фоновой текстуры)

    // --- Освещение ---
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);

    // Глобальный «окружающий» свет
    {
        GLfloat globalAmbient[4] = { 0.2f, 0.2f, 0.2f, 1.f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    }

    // Начальные параметры источников (позиции лучше ставить каждый кадр в display())
    {
        // LIGHT0 — тёплый сверху справа
        GLfloat dif0[4] = { 0.9f, 0.85f, 0.8f, 1.f };
        GLfloat amb0[4] = { 0.10f, 0.10f, 0.10f, 1.f };
        GLfloat spc0[4] = { 0.7f, 0.7f, 0.7f, 1.f };
        glLightfv(GL_LIGHT0, GL_DIFFUSE, dif0);
        glLightfv(GL_LIGHT0, GL_AMBIENT, amb0);
        glLightfv(GL_LIGHT0, GL_SPECULAR, spc0);

        // LIGHT1 — холодный слева
        GLfloat dif1[4] = { 0.6f, 0.7f, 0.9f, 1.f };
        GLfloat amb1[4] = { 0.05f, 0.05f, 0.08f, 1.f };
        GLfloat spc1[4] = { 0.3f, 0.3f, 0.4f, 1.f };
        glLightfv(GL_LIGHT1, GL_DIFFUSE, dif1);
        glLightfv(GL_LIGHT1, GL_AMBIENT, amb1);
        glLightfv(GL_LIGHT1, GL_SPECULAR, spc1);

        // LIGHT2 — сзади
        GLfloat dif2[4] = { 0.5f, 0.5f, 0.5f, 1.f };
        GLfloat amb2[4] = { 0.03f, 0.03f, 0.03f, 1.f };
        GLfloat spc2[4] = { 0.6f, 0.6f, 0.6f, 1.f };
        glLightfv(GL_LIGHT2, GL_DIFFUSE, dif2);
        glLightfv(GL_LIGHT2, GL_AMBIENT, amb2);
        glLightfv(GL_LIGHT2, GL_SPECULAR, spc2);
    }

    // Главный цикл
    auxMainLoop(display);
    return 0;
}
