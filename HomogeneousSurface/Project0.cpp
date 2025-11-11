#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif 

#include "CoordinateAxes.hpp"
#include "Background.hpp"
#include "AstroidConeLit.hpp"

static CoordinateAxes gAxes;
static Background gBackground;

// Одна красивая астроида!
static AstroidConeLit gAstroid(12, 64, 3.0f); // levels, segments, depth

// ---- анимация вращения сцены ----
static GLFWwindow* gWindow = nullptr;
static bool   gSpinEnabled = true;     // включить/выключить авто-вращение
static float  gYawDeg = -35.0f;   // поворот вокруг Y (горизонт)
static float  gPitchDeg = 35.0f;    // поворот вокруг X (наклон)
static float  gYawSpeedDps = 20.0f;    // скорость по Y, градусов в секунду
static double gLastTime = 0.0;        // для дельта-времени
static float gKeyYawSpeedDps = 90.0f;   // скорость поворота по Y от стрелок
static float gKeyPitchSpeedDps = 90.0f;   // скорость поворота по X от стрелок
static float gCamDist = 6.0f;    // дистанция "камеры" (Translate z)

void initScene() {
	gAxes.setLength(5.5f);
	gAxes.setArrow(0.1f, 0.2f);
	gAxes.setLineWidth(1.5f);
	gAxes.setShowOriginPoint(true);
	gAxes.setLineSmooth(true);

	if (!gBackground.loadFromExeDir("assets/background_lion.bmp")) {
		std::cerr << "Warning: Can't load assets/background_lion.bmp" << std::endl;
	}

	// Построить астроиду
	gAstroid.build();
	gAstroid.setWireframe(true); // С каркасом видно структуру

    gLastTime = glfwGetTime();
}

void resize(int width, int height)
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

void display(void)
{
    // очистка
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- 1) фон (в экранных координатах, сам чинит свои стейты) ---
    gBackground.draw();

    // --- 2) тайминг / анимация ---
    double now = glfwGetTime();
    float dt = (float)(now - gLastTime); // сек
    gLastTime = now;

    if (gSpinEnabled) {
        gYawDeg += gYawSpeedDps * dt;
        if (gYawDeg > 360.f)  gYawDeg -= 360.f;
        if (gYawDeg < -360.f) gYawDeg += 360.f;
    }

    // --- 3) обработка клавиш (стрелки, PgUp/PgDn, Space, R) ---
    // Yaw (влево/вправо)
    if (glfwGetKey(gWindow, GLFW_KEY_LEFT) == GLFW_PRESS) gYawDeg -= gKeyYawSpeedDps * dt;
    if (glfwGetKey(gWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) gYawDeg += gKeyYawSpeedDps * dt;
    // Pitch (вверх/вниз)
    if (glfwGetKey(gWindow, GLFW_KEY_UP) == GLFW_PRESS) gPitchDeg -= gKeyPitchSpeedDps * dt;
    if (glfwGetKey(gWindow, GLFW_KEY_DOWN) == GLFW_PRESS) gPitchDeg += gKeyPitchSpeedDps * dt;
    if (gPitchDeg > 89.f) gPitchDeg = 89.f;
    if (gPitchDeg < -89.f) gPitchDeg = -89.f;
    // Зум (PgUp / PgDn)
    if (glfwGetKey(gWindow, GLFW_KEY_PAGE_UP) == GLFW_PRESS) gCamDist -= 3.0f * dt; // PgUp
    if (glfwGetKey(gWindow, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) gCamDist += 3.0f * dt; // PgDn
    if (gCamDist < 2.5f) gCamDist = 2.5f;
    if (gCamDist > 12.f) gCamDist = 12.f;

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

    // рисуем оси и астроиду
    gAxes.draw();
    gAstroid.draw();

    glFlush();
    glPopMatrix();
}

// Callback для обработки нажатий клавиш (Space и R)
static bool gSpaceWasPressed = false;
static bool gRWasPressed = false;

void processKeyCallbacks() {
    // Обработка Space (toggle spin)
    bool spacePressed = (glfwGetKey(gWindow, GLFW_KEY_SPACE) == GLFW_PRESS);
    if (spacePressed && !gSpaceWasPressed) {
        gSpinEnabled = !gSpinEnabled;
    }
    gSpaceWasPressed = spacePressed;

    // Обработка R (reset orientation)
    bool rPressed = (glfwGetKey(gWindow, GLFW_KEY_R) == GLFW_PRESS);
    if (rPressed && !gRWasPressed) {
        gYawDeg = -35.f;
        gPitchDeg = 35.f;
    }
    gRWasPressed = rPressed;
}


// Callback для изменения размера окна
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    resize(width, height);
}

int main()
{
    // Инициализация GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Настройка GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing

    // Создание окна 500x500
    gWindow = glfwCreateWindow(500, 500, "OpenGL - Homogeneous Surface", nullptr, nullptr);
    if (!gWindow) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(gWindow);
    glfwSetFramebufferSizeCallback(gWindow, framebuffer_size_callback);

    // Инициализация размера viewport
    int width, height;
    glfwGetFramebufferSize(gWindow, &width, &height);
    resize(width, height);

    // Инициализация сцены (фон, оси, геометрия и т.д.)
    initScene();

    // Глобальные GL-состояния
    glEnable(GL_DEPTH_TEST);           // Z-тест
    glEnable(GL_BLEND);                // прозрачность в фигурах
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_COLOR_MATERIAL);       // материал берём из glColor
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    glShadeModel(GL_SMOOTH);           // Gouraud
    glEnable(GL_NORMALIZE);            // корректные нормали при масштабах

    glDisable(GL_CULL_FACE);           // рисуем обе стороны

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
    while (!glfwWindowShouldClose(gWindow)) {
        // Обработка событий
        glfwPollEvents();

        // Обработка клавиш Space и R
        processKeyCallbacks();

        // Отрисовка
        display();

        // Swap buffers
        glfwSwapBuffers(gWindow);
    }

    glfwDestroyWindow(gWindow);
    glfwTerminate();
    return 0;
}
