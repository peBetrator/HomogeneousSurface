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
}

void CALLBACK display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//   glClear(GL_COLOR_BUFFER_BIT); 
	//   glClear(GL_DEPTH_BUFFER_BIT); 

	gBackground.draw();

	glPushMatrix();
	glTranslated(0.0, 0.0, -6.0);
	glRotated(35.0, 1.0, 0.0, 0.0);
	glRotated(-35.0, 0.0, 1.0, 0.0);

    // === Позиции источников света (устанавливаются каждый кадр) ===
    GLfloat pos0[4] = { 3.f,  3.f,  3.f, 1.f }; // точечный источник справа сверху спереди
    GLfloat pos1[4] = { -3.f,  3.f, -3.f, 1.f }; // слева сверху сзади
    GLfloat pos2[4] = { 0.f, -3.f,  3.f, 1.f }; // снизу спереди

    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glLightfv(GL_LIGHT1, GL_POSITION, pos1);
    glLightfv(GL_LIGHT2, GL_POSITION, pos2);

    // Можно также задать цвет/интенсивность:
    GLfloat diffuse0[4] = { 1.f, 0.9f, 0.9f, 1.f };
    GLfloat diffuse1[4] = { 0.9f, 1.f, 0.9f, 1.f };
    GLfloat diffuse2[4] = { 0.9f, 0.9f, 1.f, 1.f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse1);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse2);

	gAxes.draw();

	//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV 
	// TODO: add draw code for native data here  
	// to learn OpenGL functions 
	gLineCone.draw();
	gCircle.draw();
	gParabola.draw();
	glFlush();


	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 

	glPopMatrix();

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
