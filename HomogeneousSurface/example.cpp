#include <windows.h>
#include <GL/gl.h>
#pragma comment(lib,"OpenGL32.lib")
#include <GL/glu.h>
#pragma comment(lib,"Glu32.lib")
#pragma comment (lib, "legacy_stdio_definitions.lib")
#include "GL/glaux.h"
#pragma comment(lib,"Glaux.lib")

#include <cmath>
#include <algorithm> // std::max

struct Vec3 { double x{}, y{}, z{}; };

// === DynCone (как был), но на Vec3 и std::cos/sin ===
class DynCone {
	int nh, ns;
	Vec3* rings;
public:
	DynCone(int heightSeg, int sectors)
		: nh(heightSeg < 1 ? 1 : heightSeg),
		ns(sectors < 3 ? 3 : sectors),
		rings(new Vec3[nh * ns]) {
		const double PI = 3.14159265358979323846;
		for (int j = 0;j < ns;++j) {
			double a = 2.0 * PI * j / ns;
			rings[(nh - 1) * ns + j] = { std::cos(a), std::sin(a), 1.0 };
		}
		for (int i = 0;i < nh - 1;++i) {
			double t = double(i + 1) / nh;
			for (int j = 0;j < ns;++j) {
				const Vec3& b = rings[(nh - 1) * ns + j];
				rings[i * ns + j] = { b.x * t, b.y * t, t };
			}
		}
	}
	~DynCone() { delete[] rings; }

	void draw() const {
		glBegin(GL_TRIANGLE_FAN);
		glVertex3d(0, 0, 0);
		for (int j = 0;j < ns;++j) glVertex3d(rings[j].x, rings[j].y, rings[j].z);
		glVertex3d(rings[0].x, rings[0].y, rings[0].z);
		glEnd();

		for (int i = 0;i < nh - 1;++i) {
			glBegin(GL_TRIANGLE_STRIP);
			for (int j = 0;j < ns;++j) {
				const Vec3& a = rings[i * ns + j];
				const Vec3& b = rings[(i + 1) * ns + j];
				glVertex3d(a.x, a.y, a.z);
				glVertex3d(b.x, b.y, b.z);
			}
			const Vec3& a0 = rings[i * ns + 0];
			const Vec3& b0 = rings[(i + 1) * ns + 0];
			glVertex3d(a0.x, a0.y, a0.z);
			glVertex3d(b0.x, b0.y, b0.z);
			glEnd();
		}
	}
};

// === Усечённый конус между двумя точками ===
class DynSegmentCone {
	int ns;
	Vec3* ring0, * ring1;
public:
	DynSegmentCone(int sectors = 24) : ns(sectors < 3 ? 3 : sectors) {
		ring0 = new Vec3[ns];
		ring1 = new Vec3[ns];
	}
	~DynSegmentCone() { delete[] ring0; delete[] ring1; }

	void draw(const Vec3& p0, const Vec3& p1, double r0, double r1) {
		Vec3 d{ p1.x - p0.x, p1.y - p0.y, p1.z - p0.z };
		double L = std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
		if (L <= 1e-9) return;

		Vec3 w{ d.x / L, d.y / L, d.z / L };
		Vec3 base{ 0,0,1 }; if (std::fabs(w.z) > 0.9) base = { 1,0,0 };
		Vec3 u{ w.y * base.z - w.z * base.y, w.z * base.x - w.x * base.z, w.x * base.y - w.y * base.x };
		double ul = std::sqrt(u.x * u.x + u.y * u.y + u.z * u.z); u = { u.x / ul, u.y / ul, u.z / ul };
		Vec3 v{ w.y * u.z - w.z * u.y, w.z * u.x - w.x * u.z, w.x * u.y - w.y * u.x };

		const double PI = 3.14159265358979323846;
		for (int j = 0;j < ns;++j) {
			double a = 2.0 * PI * j / ns;
			double ca = std::cos(a), sa = std::sin(a);
			ring0[j] = { p0.x + r0 * (u.x * ca + v.x * sa),
						 p0.y + r0 * (u.y * ca + v.y * sa),
						 p0.z + r0 * (u.z * ca + v.z * sa) };
			ring1[j] = { p1.x + r1 * (u.x * ca + v.x * sa),
						 p1.y + r1 * (u.y * ca + v.y * sa),
						 p1.z + r1 * (u.z * ca + v.z * sa) };
		}

		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0;j < ns;++j) {
			glVertex3d(ring0[j].x, ring0[j].y, ring0[j].z);
			glVertex3d(ring1[j].x, ring1[j].y, ring1[j].z);
		}
		glVertex3d(ring0[0].x, ring0[0].y, ring0[0].z);
		glVertex3d(ring1[0].x, ring1[0].y, ring1[0].z);
		glEnd();
	}
};

// === Полуконус по кривой (sweep 180°) ===
class HalfConeSweep {
	int nt, ns;
	Vec3* stripA, * stripB;
public:
	HalfConeSweep(int tSteps = 64, int sSteps = 24)
		: nt(tSteps < 2 ? 2 : tSteps), ns(steps < 3 ? 3 : sSteps) { // <== если опечатка, поправь: sSteps
		stripA = new Vec3[ns + 1];
		stripB = new Vec3[ns + 1];
	}
	~HalfConeSweep() { delete[] stripA; delete[] stripB; }

	static Vec3 curve(double t, double R, double K) {
		return { R * t, -K * std::sqrt(std::max(0.0, t)), 0.0 };
	}
	static double radius(double t, double r0, double r1) { return r0 * (1.0 - t) + r1 * t; }

	void draw(double R, double K, double r0, double r1) {
		const double PI = 3.14159265358979323846;

		auto makeStrip = [&](const Vec3& P, const Vec3& T, double rad, Vec3* out) {
			Vec3 w = T; double wl = std::sqrt(w.x * w.x + w.y * w.y + w.z * w.z); w = { w.x / wl,w.y / wl,w.z / wl };
			Vec3 base{ 0,0,1 }; if (std::fabs(w.z) > 0.9) base = { 1,0,0 };
			Vec3 u{ w.y * base.z - w.z * base.y, w.z * base.x - w.x * base.z, w.x * base.y - w.y * base.x };
			double ul = std::sqrt(u.x * u.x + u.y * u.y + u.z * u.z); u = { u.x / ul,u.y / ul,u.z / ul };
			Vec3 v{ w.y * u.z - w.z * u.y, w.z * u.x - w.x * u.z, w.x * u.y - w.y * u.x };

			for (int j = 0;j <= ns;++j) {
				double a = (-0.5 + double(j) / ns) * PI; // [-pi/2 .. +pi/2]
				double ca = std::cos(a), sa = std::sin(a);
				out[j] = { P.x + rad * (u.x * ca + v.x * sa),
						   P.y + rad * (u.y * ca + v.y * sa),
						   P.z + rad * (u.z * ca + v.z * sa) };
			}
			};

		for (int i = 1;i <= nt;++i) {
			double t0 = double(i - 1) / nt, t1 = double(i) / nt;
			Vec3 P0 = curve(t0, R, K), P1 = curve(t1, R, K);
			Vec3 T = { P1.x - P0.x, P1.y - P0.y, P1.z - P0.z };
			double r0i = radius(t0, r0, r1), r1i = radius(t1, r0, r1);

			makeStrip(P0, T, r0i, stripA);
			makeStrip(P1, T, r1i, stripB);

			glBegin(GL_TRIANGLE_STRIP);
			for (int j = 0;j <= ns;++j) {
				glVertex3d(stripA[j].x, stripA[j].y, stripA[j].z);
				glVertex3d(stripB[j].x, stripB[j].y, stripB[j].z);
			}
			glEnd();
		}
	}
};



// --- БАЗОВЫЙ МЕШ "конус" для стрелки оси (динамика памяти)
class ConeMesh {
	int nh;     // деления по высоте
	int ns;     // деления по кругу
	Vec3* ringApex; // вершины по высоте/углу (динамический массив: nh*ns)
public:
	ConeMesh(int heightSeg, int sectors)
		: nh(heightSeg < 1 ? 1 : heightSeg), ns(sectors < 3 ? 3 : sectors) {
		ringApex = new Vec3[nh * ns];

		// Геометрия конуса единичной высоты: основание на z=1 радиуса 1, вершина в (0,0,0).
		const double PI = 3.14159265358979323846;
		for (int j = 0; j < ns; ++j) {
			double a = 2.0 * PI * (double)j / (double)ns;
			// Кольцо у основания (верхний уровень)
			ringApex[(nh - 1) * ns + j] = { std::cos(a), std::sin(a), 1.0 };
		}
		// Интерполяция по высоте (линейная усадка радиуса к вершине)
		for (int i = 0; i < nh - 1; ++i) {
			double t = (double)(i + 1) / (double)nh; // (0..1], 1=основание
			for (int j = 0; j < ns; ++j) {
				const Vec3& b = ringApex[(nh - 1) * ns + j];
				ringApex[i * ns + j] = { b.x * t, b.y * t, t };
			}
		}
	}

	~ConeMesh() { delete[] ringApex; }

	void draw() const {
		// крышка-веер от вершины
		glBegin(GL_TRIANGLE_FAN);
		glVertex3d(0.0, 0.0, 0.0);
		for (int j = 0; j < ns; ++j) glVertex3d(ringApex[0 * ns + j].x, ringApex[0 * ns + j].y, ringApex[0 * ns + j].z);
		glVertex3d(ringApex[0 * ns + 0].x, ringApex[0 * ns + 0].y, ringApex[0 * ns + 0].z);
		glEnd();

		// боковая поверхность (полосами)
		for (int i = 0; i < nh - 1; ++i) {
			glBegin(GL_TRIANGLE_STRIP);
			for (int j = 0; j < ns; ++j) {
				const Vec3& a = ringApex[i * ns + j];
				const Vec3& b = ringApex[(i + 1) * ns + j];
				glVertex3d(a.x, a.y, a.z);
				glVertex3d(b.x, b.y, b.z);
			}
			// замкнуть на первый сектор
			const Vec3& a0 = ringApex[i * ns + 0];
			const Vec3& b0 = ringApex[(i + 1) * ns + 0];
			glVertex3d(a0.x, a0.y, a0.z);
			glVertex3d(b0.x, b0.y, b0.z);
			glEnd();
		}
	}
};

// --- Стрелка (ось + наконечник)
class AxisArrow {
	double length;
	double coneRadius;
	double coneHeight;
	ConeMesh tip;
public:
	AxisArrow(double len, int coneHSeg, int coneSectors, double coneR = 0.1, double coneH = 0.2)
		: length(len), coneRadius(coneR), coneHeight(coneH), tip(coneHSeg, coneSectors) {
	}

	void drawX() const {
		// линия X
		glBegin(GL_LINES);
		glVertex3d(-length, 0.0, 0.0);
		glVertex3d(length, 0.0, 0.0);
		glEnd();
		// конус на +X
		glPushMatrix();
		glTranslated(length - coneHeight, 0.0, 0.0);
		glRotated(90.0, 0.0, 1.0, 0.0);
		glScaled(coneRadius, coneRadius, coneHeight);
		tip.draw();
		glPopMatrix();
	}

	void drawY() const {
		glBegin(GL_LINES);
		glVertex3d(0.0, -length, 0.0);
		glVertex3d(0.0, length, 0.0);
		glEnd();
		glPushMatrix();
		glTranslated(0.0, length - coneHeight, 0.0);
		glRotated(-90.0, 1.0, 0.0, 0.0);
		glScaled(coneRadius, coneRadius, coneHeight);
		tip.draw();
		glPopMatrix();
	}

	void drawZ() const {
		glBegin(GL_LINES);
		glVertex3d(0.0, 0.0, -length);
		glVertex3d(0.0, 0.0, length);
		glEnd();
		glPushMatrix();
		glTranslated(0.0, 0.0, length - coneHeight);
		// без поворота: конус из (0,0,0)->(0,0,1)
		glScaled(coneRadius, coneRadius, coneHeight);
		tip.draw();
		glPopMatrix();
	}
};

// --- Оси 3D (толщина линий + цвета)
class Axes3D {
	AxisArrow arrow;
public:
	Axes3D(double len, int coneHSeg = 8, int coneSec = 24, double coneR = 0.1, double coneH = 0.25)
		: arrow(len, coneHSeg, coneSec, coneR, coneH) {
	}

	void draw() const {
		glLineWidth(1.5f);
		glEnable(GL_LINE_SMOOTH);

		// X: черный->красный
		glBegin(GL_LINES);
		glColor3d(0, 0, 0); glVertex3d(-arrowLen(), 0, 0);
		glColor3d(1, 0, 0); glVertex3d(arrowLen(), 0, 0);
		glEnd();
		glColor3d(1, 0, 0); arrow.drawX();

		// Y: черный->зеленый
		glBegin(GL_LINES);
		glColor3d(0, 0, 0); glVertex3d(0, -arrowLen(), 0);
		glColor3d(0, 1, 0); glVertex3d(0, arrowLen(), 0);
		glEnd();
		glColor3d(0, 1, 0); arrow.drawY();

		// Z: черный->синий
		glBegin(GL_LINES);
		glColor3d(0, 0, 0); glVertex3d(0, 0, -arrowLen());
		glColor3d(0, 0, 1); glVertex3d(0, 0, arrowLen());
		glEnd();
		glColor3d(0, 0, 1); arrow.drawZ();
	}
private:
	double arrowLen() const { return 5.5; } // можешь вынести в параметры
};

// --- ПОВЕРХНОСТЬ (z = f(x,y)) — динамика памяти, параметрическая сетка
typedef double (*HeightFn)(double x, double y);

class ParamSurface {
	int nx, ny;        // разбиения по x/y
	double xmin, xmax, ymin, ymax;
	Vec3* verts;       // динамический массив (nx*ny)
public:
	ParamSurface(int nx_, int ny_, double xmin_, double xmax_, double ymin_, double ymax_, HeightFn f)
		: nx(nx_ < 2 ? 2 : nx_), ny(ny_ < 2 ? 2 : ny_), xmin(xmin_), xmax(xmax_), ymin(ymin_), ymax(ymax_) {
		verts = new Vec3[nx * ny];
		const double dx = (xmax - xmin) / (nx - 1);
		const double dy = (ymax - ymin) / (ny - 1);
		for (int iy = 0; iy < ny; ++iy) {
			double y = ymin + dy * iy;
			for (int ix = 0; ix < nx; ++ix) {
				double x = xmin + dx * ix;
				double z = f ? f(x, y) : 0.0;
				verts[iy * nx + ix] = { x, y, z };
			}
		}
	}

	~ParamSurface() { delete[] verts; }

	void drawFilled() const {
		// Рендерим полосами по y
		for (int iy = 0; iy < ny - 1; ++iy) {
			glBegin(GL_TRIANGLE_STRIP);
			for (int ix = 0; ix < nx; ++ix) {
				const Vec3& v0 = verts[(iy)*nx + ix];
				const Vec3& v1 = verts[(iy + 1) * nx + ix];
				// простой цвет/нормали можно добавить здесь
				glVertex3d(v0.x, v0.y, v0.z);
				glVertex3d(v1.x, v1.y, v1.z);
			}
			glEnd();
		}
	}

	void drawWire() const {
		glDisable(GL_LIGHTING);
		glColor3d(0.0, 0.0, 0.0);
		// Изолинии по y
		for (int iy = 0; iy < ny; ++iy) {
			glBegin(GL_LINE_STRIP);
			for (int ix = 0; ix < nx; ++ix) {
				const Vec3& v = verts[iy * nx + ix];
				glVertex3d(v.x, v.y, v.z);
			}
			glEnd();
		}
		// Изолинии по x
		for (int ix = 0; ix < nx; ++ix) {
			glBegin(GL_LINE_STRIP);
			for (int iy = 0; iy < ny; ++iy) {
				const Vec3& v = verts[iy * nx + ix];
				glVertex3d(v.x, v.y, v.z);
			}
			glEnd();
		}
		glEnable(GL_LIGHTING);
	}
};

double surfaceFn(double x, double y) {
	return 0.2 * (x * x - y * y);
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

// «окружность» как перевёрнутый конус (детализация 24x24)
static DynCone g_ball(24, 24);

// «соединительная линия» как усечённый конус между точками
static DynSegmentCone g_stick(24);

// «полуконус» по кривой
static HalfConeSweep g_curve(64, 32);


void CALLBACK display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(0.0, 0.0, -6.0);
	glRotated(35.0, 1.0, 0.0, 0.0);
	glRotated(-35.0, 0.0, 1.0, 0.0);

	// точка в центре
	glDisable(GL_LIGHTING);
	glPointSize(8.0f);
	glBegin(GL_POINTS); glColor3d(0, 0, 0); glVertex3d(0, 0, 0); glEnd();
	glEnable(GL_LIGHTING);

	// 1) «окружность» как перевёрнутый конус
	glPushMatrix();
	glTranslated(-2.0, 1.2, 0.0);
	glRotated(180.0, 1, 0, 0);
	glScaled(0.8, 0.8, 0.8);
	glColor3d(0, 0, 0);
	g_ball.draw();
	glPopMatrix();

	// 2) наклонная «линия» (усечённый конус)
	{
		Vec3 p0{ -2.0, 1.2, 0.0 }, p1{ 0.0, 0.0, 0.0 };
		glColor3d(0, 0, 0);
		g_stick.draw(p0, p1, 0.12, 0.07);
	}

	// 3) кривая — полуконус
	glColor3d(0, 0, 0);
	g_curve.draw(/*R*/3.0, /*K*/2.5, /*r0*/0.18, /*r1*/0.06);

	auxSwapBuffers();
}


int main()
{
	// Устанавливаем координаты окна на экране 
	// левый верхний угол (0, 0) 
	// ширина и высота 500  
	auxInitPosition(0, 0, 500, 500);

	// Устанавливаем параметры контекста OpenGL 
	auxInitDisplayMode(AUX_RGB | AUX_DEPTH | AUX_DOUBLE);

	// Создаем окно на экране 
	auxInitWindow(L"OpenGL");

	// Это окно будет получать сообщения о событиях 
	// от клавиатуры, мыши, таймера и любые другие сообщения. 
	// Пока не поступают никакие сообщения циклически будет 
	// вызываться функция display(). 
	// Так можно создавать анимации. 
	// Если нужно статическое изображение 
	// следующая строка может быть закомментирована 
	auxIdleFunc(display);

	// В случае изменения размеров окна – поступает  
	// соответствующее сообщение 
	// В Windows - это WM_SIZE. 
	// Указываем, что функция resize() должна быть вызвана  
	// каждый раз когда изменяются размеры окна 
	auxReshapeFunc(resize);

	// Далее задаем ряд тестов и параметров 
	// Включается тест прозрачности, т.е. будет приниматься 
	// во внимание 4-й параметр в glColor() 
	glEnable(GL_ALPHA_TEST);

	// Тест глубины 
	glEnable(GL_DEPTH_TEST);

	// Функция glColor() будет задавать  
	// свойства материалов. 

	// Следовательно, отсутствует необходимость 
	// в дополнительном вызове функции glMaterialfv() 
	glEnable(GL_COLOR_MATERIAL);

	// Разрешаем освешение 
	glEnable(GL_LIGHTING);

	// Активируем источник освещения с номером 0 
	glEnable(GL_LIGHT0);
	// Задаем позицию источника освещения 
	float pos[4] = { 3.0f, 3.0f, 3.0f, 1.0f };
	float dir[3] = { -1.0f, -1.0f, -1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);

	// Разрешаем смешивание цветов (для прозрачных поверхностей) 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Устанавливаем цвет начальной закраски окна 
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// Указываем, что функция display() должна использоваться для 
	// перерисовки окна. 
	// Эта функция будет вызвана каждый раз когда возникает 
	// необходимость в перерисовки окна,   
	// т.е. при поступлении сообщения WM_PAINT от Windows 
	// Например, когда окно развертывается на весь экран. 
	auxMainLoop(display);

	return 0;
}