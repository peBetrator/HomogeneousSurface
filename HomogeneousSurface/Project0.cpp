#include <windows.h>
#include <GL/gl.h>
#pragma comment(lib,"OpenGL32.lib")
#include <GL/glu.h>
#pragma comment(lib,"Glu32.lib")
#pragma comment (lib, "legacy_stdio_definitions.lib")
#include "GL/glaux.h"
#pragma comment(lib,"Glaux.lib")

#include <cmath>
#include <algorithm>

// наши классы
#include "Vec3.hpp"
#include "Axes3D.hpp"
#include "CurvedSheet.hpp"
#include "DynCone.hpp"
#include "DynSegmentCone.hpp"
#include "HalfConeSweep.hpp"
#include "UnitPlane.hpp"

// -------------------- Глобальные объекты сцены --------------------
static Axes3D g_axes;
static DynCone         g_ball(8, 10);         // «окружность» как перевёрнутый конус
static CurvedSheet g_asym(12);   // кривая как «полуконус»
static UnitPlane g_plane;

// -------------------- Глобальные переменные --------------------
static const double H = 1.6;   // одинаковая высота “в глубину”
static double XM = 1.0;        // Точка пересечения с соединительной плоскостью (внутри ПН четверти)
static double YM = -1.0;

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
	// Ротация камеры
	glRotated(35.0, 1.0, 0.0, 0.0);
	glRotated(-35.0, 0.0, 1.0, 0.0);

	// Система координат (x y z)
	glPointSize(6.0f);
	glBegin(GL_POINTS); glColor3d(0, 0, 0); glVertex3d(0, 0, 0); glEnd();
	g_axes.draw();

	// 1) Окружность-конус (фиолетовый + чёрный каркас)
	glPushMatrix();
	glTranslated(-2.0, 2.0, 0.0);
	glRotated(180.0, 1, 0, 0);
	glScaled(0.6, 0.6, H);

	// заливка
	glColor3d(1.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	g_ball.draw();

	// каркас
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1, -1);
	glColor3d(0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1.0f);
	g_ball.draw();
	glDisable(GL_POLYGON_OFFSET_LINE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // вернуть по умолчанию
	glPopMatrix();


	// 2) Соединительный усечённый конус (оранжевый + чёрный каркас)
	{
		const Vec3 pA = { -2.0,  2.0, 0.0 }; // апекс конуса
		const Vec3 pB = { XM,   YM,  0.0 };  // из CurvedSheet.cpp

		const double dx = pB.x - pA.x, dy = pB.y - pA.y;
		const double len = std::sqrt(dx * dx + dy * dy);
		const double angDeg = std::atan2(dy, dx) * 180.0 / 3.14159265358979323846;

		glPushMatrix();
		glTranslated(pA.x, pA.y, 0.0);
		glRotated(angDeg, 0, 0, 1);
		glScaled(len, 0.012, H);     // длина × ширина × глубина

		glColor3d(1.0, 0.5, 0.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		g_plane.draw();

		glEnable(GL_POLYGON_OFFSET_LINE); glPolygonOffset(-1, -1);
		glColor3d(0, 0, 0); glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); glLineWidth(1.0f);
		g_plane.draw();
		glDisable(GL_POLYGON_OFFSET_LINE); glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glPopMatrix();
	}


	// 3) Асимптота — изогнутая плоскость (бирюзовая + каркас), ПН четверть, середина — M
	glPushMatrix();
	glScaled(1.0, 1.0, H);             // одинаковая "высота" вглубь

	glColor3d(0.0, 0.6, 0.6);          // заливка
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	g_asym.draw(0.0, 0.0);

	glEnable(GL_POLYGON_OFFSET_LINE);  // каркас поверх
	glPolygonOffset(-1, -1);
	glColor3d(0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1.0f);
	g_asym.draw(0.0, 0.0);

	glDisable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPopMatrix();


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
