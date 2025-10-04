#include "CircleConeLit.hpp"
#include <cmath>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/gl.h>

// --- приватные утилиты для вектора ---
static inline Vec3 vsub(const Vec3& a, const Vec3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline Vec3 vcross(const Vec3& a, const Vec3& b) {
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
static inline float vlen(const Vec3& a) { return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z); }
static inline Vec3 vnorm(const Vec3& a) {
    float L = vlen(a);
    if (L > 1e-12f) return { a.x / L, a.y / L, a.z / L };
    return { 0.f, 0.f, 1.f };
}
static inline Vec3 vadd(const Vec3& a, const Vec3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }

// --- реализация класса ---
void CircleConeLit::releaseNormals() {
    if (mNormals) {
        delete[] mNormals;
        mNormals = nullptr;
        mNormalCount = 0;
    }
}

void CircleConeLit::computeVertexNormals() {
    releaseNormals();
    const auto& V = vertices();
    const auto& I = indices();

    mNormalCount = V.size();
    if (!mNormalCount) return;

    mNormals = new Vec3[mNormalCount];
    // инициализируем нулями
    for (size_t i = 0; i < mNormalCount; ++i) mNormals[i] = { 0.f, 0.f, 0.f };

    // аккумулируем нормали граней на вершины
    for (size_t k = 0; k + 2 < I.size(); k += 3) {
        unsigned ia = I[k + 0], ib = I[k + 1], ic = I[k + 2];
        const Vec3& A = V[ia];
        const Vec3& B = V[ib];
        const Vec3& C = V[ic];

        // нормаль треугольника (ориентация важна для направления)
        Vec3 n = vcross(vsub(B, A), vsub(C, A));
        // без нормализации здесь — чтобы был «вес» площади
        mNormals[ia] = vadd(mNormals[ia], n);
        mNormals[ib] = vadd(mNormals[ib], n);
        mNormals[ic] = vadd(mNormals[ic], n);
    }

    // нормализуем пер-вершинные
    for (size_t i = 0; i < mNormalCount; ++i) {
        mNormals[i] = vnorm(mNormals[i]);
    }
}

void CircleConeLit::build() {
    // построить геометрию родителя
    CircleCone::build();
    // посчитать нормали
    computeVertexNormals();
}

void CircleConeLit::draw() const {
    const auto& V = vertices();
    const auto& I = indices();

    // --- включаем освещение ---
    GLboolean wasLighting = glIsEnabled(GL_LIGHTING);
    if (!wasLighting) glEnable(GL_LIGHTING);

    // полезно, чтобы масштабирование не портило нормали
    GLboolean wasNormalize = glIsEnabled(GL_NORMALIZE);
    if (!wasNormalize) glEnable(GL_NORMALIZE);

    // материал можно через glColorMaterial (у тебя уже включено в main),
    // но на всякий случай зададим базовый спекуляр
    GLfloat spec[4] = { 0.25f, 0.25f, 0.25f, 1.f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.f);

    // полупрозрачная заливка
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.25f, 0.25f, 0.7f, 0.6f);

    // ——— заполнение треугольниками с пер-вершинными нормалями ———
    glBegin(GL_TRIANGLES);
    for (size_t k = 0; k + 2 < I.size(); k += 3) {
        unsigned ia = I[k + 0], ib = I[k + 1], ic = I[k + 2];

        if (mNormals) glNormal3f(mNormals[ia].x, mNormals[ia].y, mNormals[ia].z);
        glVertex3f(V[ia].x, V[ia].y, V[ia].z);

        if (mNormals) glNormal3f(mNormals[ib].x, mNormals[ib].y, mNormals[ib].z);
        glVertex3f(V[ib].x, V[ib].y, V[ib].z);

        if (mNormals) glNormal3f(mNormals[ic].x, mNormals[ic].y, mNormals[ic].z);
        glVertex3f(V[ic].x, V[ic].y, V[ic].z);
    }
    glEnd();

    // ——— опциональный каркас по тем же треугольникам ———
    if (mWireframe) {
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.f, -1.f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glLineWidth(1.0f);
        glColor4f(0.05f, 0.05f, 0.2f, 0.9f);

        glBegin(GL_TRIANGLES);
        for (size_t k = 0; k + 2 < I.size(); k += 3) {
            unsigned ia = I[k + 0], ib = I[k + 1], ic = I[k + 2];
            glVertex3f(V[ia].x, V[ia].y, V[ia].z);
            glVertex3f(V[ib].x, V[ib].y, V[ib].z);
            glVertex3f(V[ic].x, V[ic].y, V[ic].z);
        }
        glEnd();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
    }

    // ——— восстановление состояний ———
    if (!wasNormalize) glDisable(GL_NORMALIZE);
    if (!wasLighting)  glDisable(GL_LIGHTING);
}
