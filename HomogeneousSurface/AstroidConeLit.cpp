/**
 * AstroidConeLit.cpp
 *
 * Реализация астроиды с освещением по модели Фонга (Phong Lighting Model).
 *
 * Дополнения к базовой геометрии:
 * - Вычисление вершинных нормалей для плавного затенения (Gouraud shading)
 * - Настройка материалов (ambient, diffuse, specular, shininess)
 * - Двустороннее освещение (разные цвета для передней и задней стороны)
 * - Опциональный каркас (wireframe)
 */

#include "AstroidConeLit.hpp"
#include <cmath>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

// ============================================================================
// Утилиты для работы с векторами
// ============================================================================

/**
 * Вычитание векторов: result = a - b
 */
static inline Vec3 vsub(const Vec3& a, const Vec3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

/**
 * Векторное произведение (cross product): result = a × b
 *
 * Формула:
 *   a × b = (a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x)
 *
 * Свойства:
 * - Результат перпендикулярен обоим векторам
 * - Длина результата = площадь параллелограмма |a|·|b|·sin(θ)
 * - Направление определяется правилом правой руки
 */
static inline Vec3 vcross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

/**
 * Длина (модуль) вектора: |a| = √(x² + y² + z²)
 */
static inline float vlen(const Vec3& a) {
    return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

/**
 * Нормализация вектора (приведение к единичной длине): â = a / |a|
 *
 * Если длина очень мала (< 1e-12), возвращаем вектор (0, 0, 1)
 */
static inline Vec3 vnorm(const Vec3& a) {
    float L = vlen(a);
    if (L > 1e-12f) return { a.x / L, a.y / L, a.z / L };
    return { 0.f, 0.f, 1.f };  // fallback для вырожденных случаев
}

/**
 * Сложение векторов: result = a + b
 */
static inline Vec3 vadd(const Vec3& a, const Vec3& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

// ============================================================================
// Методы класса AstroidConeLit
// ============================================================================

/**
 * Освободить массив нормалей
 */
void AstroidConeLit::releaseNormals() {
    if (mNormals) {
        delete[] mNormals;
        mNormals = nullptr;
        mNormalCount = 0;
    }
}

/**
 * Вычисление вершинных нормалей для плавного затенения
 *
 * Алгоритм (среднее взвешенное по площадям):
 * 1. Инициализируем нормали всех вершин нулевым вектором
 * 2. Для каждого треугольника:
 *    - Вычисляем нормаль грани: n = (B-A) × (C-A)
 *    - Добавляем эту нормаль к нормалям всех 3 вершин треугольника
 * 3. Нормализуем все вершинные нормали (приводим к единичной длине)
 *
 * Почему не нормализуем нормали граней?
 * Ненормализованная нормаль грани имеет длину, пропорциональную площади треугольника.
 * Таким образом, большие треугольники вносят больший вклад в вершинную нормаль,
 * что даёт более корректное освещение на неравномерной сетке.
 *
 * Результат: плавное затенение (Gouraud shading)
 * В OpenGL нормали интерполируются между вершинами, создавая плавные переходы освещения.
 */
void AstroidConeLit::computeVertexNormals() {
    releaseNormals();

    const auto& V = vertices();  // массив вершин
    const auto& I = indices();   // массив индексов треугольников

    mNormalCount = V.size();
    if (!mNormalCount) return;

    // Выделяем память для нормалей (по одной на каждую вершину)
    mNormals = new Vec3[mNormalCount];

    // ШАГ 1: Инициализация нулевыми векторами
    for (size_t i = 0; i < mNormalCount; ++i) {
        mNormals[i] = { 0.f, 0.f, 0.f };
    }

    // ШАГ 2: Накопление нормалей граней
    // Индексы идут по 3 (каждые 3 индекса = 1 треугольник)
    for (size_t k = 0; k + 2 < I.size(); k += 3) {
        unsigned ia = I[k + 0];  // индекс вершины A
        unsigned ib = I[k + 1];  // индекс вершины B
        unsigned ic = I[k + 2];  // индекс вершины C

        const Vec3& A = V[ia];
        const Vec3& B = V[ib];
        const Vec3& C = V[ic];

        // Вычисляем нормаль треугольника по формуле векторного произведения:
        // n = (B - A) × (C - A)
        //
        // Геометрически:
        // - (B - A) — вектор от A к B (сторона треугольника)
        // - (C - A) — вектор от A к C (другая сторона)
        // - Их векторное произведение перпендикулярно плоскости треугольника
        // - Длина = площадь параллелограмма = 2 * площадь треугольника
        Vec3 n = vcross(vsub(B, A), vsub(C, A));

        // Добавляем нормаль грани ко всем 3 вершинам треугольника
        // (без нормализации, чтобы сохранить «вес» площади)
        mNormals[ia] = vadd(mNormals[ia], n);
        mNormals[ib] = vadd(mNormals[ib], n);
        mNormals[ic] = vadd(mNormals[ic], n);
    }

    // ШАГ 3: Нормализация вершинных нормалей
    // Приводим все нормали к единичной длине
    for (size_t i = 0; i < mNormalCount; ++i) {
        mNormals[i] = vnorm(mNormals[i]);
    }
}

/**
 * Построить геометрию с нормалями
 */
void AstroidConeLit::build() {
    // Сначала строим базовую геометрию (вершины + индексы)
    AstroidCone::build();

    // Затем вычисляем нормали для освещения
    computeVertexNormals();
}

/**
 * Отрисовка астроиды с освещением
 *
 * OpenGL функции освещения:
 * - glEnable(GL_LIGHTING) — включить освещение
 * - glEnable(GL_NORMALIZE) — автоматически нормализовать нормали (при масштабировании)
 * - glDisable(GL_CULL_FACE) — рисовать обе стороны полигонов
 * - glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE) — двустороннее освещение
 *
 * Материалы (glMaterialfv):
 * - GL_AMBIENT — цвет при окружающем освещении (ambient light)
 * - GL_DIFFUSE — цвет при рассеянном освещении (diffuse light), зависит от угла к источнику
 * - GL_SPECULAR — цвет бликов (specular highlights), зависит от угла отражения
 * - GL_SHININESS — степень зеркальности (чем больше, тем меньше и ярче блики)
 *
 * Модель Фонга:
 * I = I_ambient + I_diffuse + I_specular
 * I_ambient  = light.ambient * material.ambient
 * I_diffuse  = light.diffuse * material.diffuse * max(N·L, 0)
 * I_specular = light.specular * material.specular * max(R·V, 0)^shininess
 *
 * где:
 * - N = нормаль к поверхности
 * - L = направление на источник света
 * - R = направление отражённого луча
 * - V = направление на наблюдателя (камеру)
 */
void AstroidConeLit::draw() const {
    const auto& V = vertices();
    const auto& I = indices();

    // ============================================================
    // Сохранение текущих состояний OpenGL
    // ============================================================
    // Хорошая практика: сохранить состояния перед изменением,
    // чтобы не повлиять на другие объекты
    GLboolean wasLighting = glIsEnabled(GL_LIGHTING);
    GLboolean wasNormalize = glIsEnabled(GL_NORMALIZE);
    GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
    GLboolean wasTex2D = glIsEnabled(GL_TEXTURE_2D);
    GLboolean wasColorMat = glIsEnabled(GL_COLOR_MATERIAL);
    GLint     twoSideBefore = 0;
    glGetIntegerv(GL_LIGHT_MODEL_TWO_SIDE, &twoSideBefore);

    // ============================================================
    // Настройка состояний для корректной отрисовки с освещением
    // ============================================================
    if (!wasLighting)  glEnable(GL_LIGHTING);   // включаем освещение
    if (!wasNormalize) glEnable(GL_NORMALIZE);  // автонормализация нормалей
    if (wasCull)       glDisable(GL_CULL_FACE); // рисуем обе стороны
    if (wasTex2D)      glDisable(GL_TEXTURE_2D); // отключаем текстуры
    if (wasColorMat)   glDisable(GL_COLOR_MATERIAL); // используем glMaterial, а не glColor

    // Включаем двустороннее освещение (разные материалы для передней и задней стороны)
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // Рисуем заливку (не линии)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Включаем прозрачность
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ============================================================
    // Настройка материала для ПЕРЕДНЕЙ стороны (FRONT)
    // ============================================================
    // Пурпурный цвет для передней стороны астроиды
    const GLfloat frontAmb[4] = { 0.15f, 0.05f, 0.15f, 1.0f }; // ambient (окружающий)
    const GLfloat frontDif[4] = { 0.85f, 0.25f, 0.85f, 0.85f }; // diffuse (рассеянный) — пурпурный
    const GLfloat frontSpc[4] = { 0.40f, 0.20f, 0.40f, 0.85f }; // specular (зеркальный)

    glMaterialfv(GL_FRONT, GL_AMBIENT, frontAmb);   // цвет ambient
    glMaterialfv(GL_FRONT, GL_DIFFUSE, frontDif);   // цвет diffuse
    glMaterialfv(GL_FRONT, GL_SPECULAR, frontSpc);  // цвет бликов
    glMaterialf(GL_FRONT, GL_SHININESS, 32.f);      // зеркальность (0-128)

    // ============================================================
    // Настройка материала для ЗАДНЕЙ стороны (BACK)
    // ============================================================
    // Оранжевый цвет для задней стороны астроиды
    const GLfloat backAmb[4] = { 0.15f, 0.10f, 0.05f, 1.0f };
    const GLfloat backDif[4] = { 0.90f, 0.60f, 0.25f, 0.85f }; // оранжевый
    const GLfloat backSpc[4] = { 0.40f, 0.30f, 0.20f, 0.85f };

    glMaterialfv(GL_BACK, GL_AMBIENT, backAmb);
    glMaterialfv(GL_BACK, GL_DIFFUSE, backDif);
    glMaterialfv(GL_BACK, GL_SPECULAR, backSpc);
    glMaterialf(GL_BACK, GL_SHININESS, 18.f);       // меньше зеркальность для задней стороны

    // ============================================================
    // Отрисовка треугольников с нормалями
    // ============================================================
    glBegin(GL_TRIANGLES);

    for (size_t k = 0; k + 2 < I.size(); k += 3) {
        unsigned ia = I[k + 0];  // индекс вершины A
        unsigned ib = I[k + 1];  // индекс вершины B
        unsigned ic = I[k + 2];  // индекс вершины C

        // Для каждой вершины задаём нормаль, затем координаты
        // OpenGL использует последнюю заданную нормаль для следующей вершины

        // Вершина A
        if (mNormals) glNormal3f(mNormals[ia].x, mNormals[ia].y, mNormals[ia].z);
        glVertex3f(V[ia].x, V[ia].y, V[ia].z);

        // Вершина B
        if (mNormals) glNormal3f(mNormals[ib].x, mNormals[ib].y, mNormals[ib].z);
        glVertex3f(V[ib].x, V[ib].y, V[ib].z);

        // Вершина C
        if (mNormals) glNormal3f(mNormals[ic].x, mNormals[ic].y, mNormals[ic].z);
        glVertex3f(V[ic].x, V[ic].y, V[ic].z);
    }

    glEnd();

    // ============================================================
    // Опциональный каркас (wireframe) поверх заливки
    // ============================================================
    if (mWireframe) {
        // glPolygonOffset смещает линии ближе к камере, чтобы они не "сражались" (z-fighting)
        // с заливкой за глубину
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.f, -1.f);  // отрицательные значения = ближе к камере

        // Переключаемся на отрисовку линий
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // Настраиваем внешний вид линий
        glLineWidth(1.5f);                    // толщина линий
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);   // чёрный цвет для каркаса

        // Рисуем те же треугольники, но режим GL_LINE превращает их в линии
        glBegin(GL_TRIANGLES);
        for (size_t k = 0; k + 2 < I.size(); k += 3) {
            unsigned ia = I[k + 0], ib = I[k + 1], ic = I[k + 2];
            glVertex3f(V[ia].x, V[ia].y, V[ia].z);
            glVertex3f(V[ib].x, V[ib].y, V[ib].z);
            glVertex3f(V[ic].x, V[ic].y, V[ic].z);
        }
        glEnd();

        // Возвращаем режим заливки
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
    }

    // ============================================================
    // Восстановление исходных состояний OpenGL
    // ============================================================
    if (!twoSideBefore) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    if (wasTex2D)      glEnable(GL_TEXTURE_2D);
    if (wasColorMat)   glEnable(GL_COLOR_MATERIAL);
    if (wasCull)       glEnable(GL_CULL_FACE);
    if (!wasNormalize) glDisable(GL_NORMALIZE);
    if (!wasLighting)  glDisable(GL_LIGHTING);
}
