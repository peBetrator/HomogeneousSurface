#include <windows.h>
#include <GL/gl.h>
#pragma comment(lib,"OpenGL32.lib")
#include <GL/glu.h>
#pragma comment(lib,"Glu32.lib")
#pragma comment (lib, "legacy_stdio_definitions.lib")
#include "GL/glaux.h"
#pragma comment(lib,"Glaux.lib")

// наши классы
#include "Vec3.hpp"
#include "Axes3D.hpp"
#include "DynCone.hpp"
#include "DynSegmentCone.hpp"
#include "HalfConeSweep.hpp"

// -------------------- Глобальные объекты сцены --------------------
static Axes3D g_axes;
static DynCone         g_ball(24, 24);         // «окружность» как перевёрнутый конус
static DynSegmentCone  g_stick(24);            // соединительный усечённый конус/стержень
static HalfConeSweep   g_curve(64, 32);        // кривая как «полуконус»

// -------------------- Матрицы/проекция --------------------
void CALLBACK resize(int width, int height)
{
    GLuint wp = width < height ? width - 20 : height - 20;
    glViewport(10, 10, wp, wp);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-6.2, 6.2, -6.2, 6.2, 2.0, 12.0);

    glMatrixMode(GL_MODELVIEW);
}

// -------------------- Рендер --------------------
void CALLBACK display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glTranslated(0.0, 0.0, -6.0);
    glRotated(35.0, 1.0, 0.0, 0.0);
    glRotated(-35.0, 0.0, 1.0, 0.0);

    // Система координат (x y z)
    glPointSize(6.0f);
    glBegin(GL_POINTS); glColor3d(0, 0, 0); glVertex3d(0, 0, 0); glEnd();
    g_axes.draw();

    // 1) «Окружность»: перевёрнутый конус
    glPushMatrix();
    glTranslated(-2.0, 1.2, 0.0);     // позиция «круга»
    glRotated(180.0, 1, 0, 0);        // перевернуть, чтобы конус уходил «назад»
    glScaled(0.8, 0.8, 0.8);          // радиус/высота
    glColor3d(0.0, 0.0, 0.0);
    g_ball.draw();
    glPopMatrix();

    // 2) «Линия» между кругом и узлом: усечённый конус
    {
        Vec3 pCircle{ -2.0, 1.2, 0.0 };
        Vec3 pJoint{ 0.0, 0.0, 0.0 };
        glColor3d(0.0, 0.0, 0.0);
        g_stick.draw(pCircle, pJoint, /*r0*/0.12, /*r1*/0.07);
    }

    // 3) Кривая как полуконус
    glColor3d(0.0, 0.0, 0.0);
    g_curve.draw(/*R*/3.0, /*K*/2.5, /*r0*/0.18, /*r1*/0.06);

    glPopMatrix();

    auxSwapBuffers();
}

// -------------------- Точка входа --------------------
int main()
{
    // окно
    auxInitPosition(0, 0, 500, 500);
    auxInitDisplayMode(AUX_RGB | AUX_DEPTH | AUX_DOUBLE);
    auxInitWindow(L"OpenGL");

    // коллбеки
    auxIdleFunc(display);
    auxReshapeFunc(resize);

    // состояние OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    float pos[4] = { 3.0f, 3.0f, 3.0f, 1.0f };
    float dir[3] = { -1.0f,-1.0f,-1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1, 1, 1, 1);

    auxMainLoop(display);
    return 0;
}
