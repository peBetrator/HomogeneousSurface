#include "CircleCone.hpp"
#include <cmath>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/gl.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


CircleCone::CircleCone(int levels, int segments, float depth)
    : mLevels(levels), mSegments(segments), mDepth(depth),
    mCx(-2.0f), mCy(2.0f), mR(std::sqrt(2.0f)) {
}

void CircleCone::setLevels(int v) { mLevels = v; }
void CircleCone::setSegments(int v) { mSegments = v; }
void CircleCone::setDepth(float v) { mDepth = v; }
void CircleCone::setCircle(float cx, float cy, float r) { mCx = cx; mCy = cy; mR = r; }

void CircleCone::build() {
    mVerts.clear();
    mIdx.clear();

    const int   L = clampMin(mLevels, 1);
    const int   S = clampMin(mSegments, 3);   // минимум 3 для корректной триангуляции
    const float D = clampMin(mDepth, 0.0f);

    // 0) Апекс M(0,0,0)
    const unsigned apexIndex = 0;
    mVerts.push_back({ 0.0f, 0.0f, 0.0f });

    // 1) Кольца уровней (l = 1..L)
    // s = l/L, z = s * D; координаты на z=1 масштабируем к M(0,0)
    for (int l = 1; l <= L; ++l) {
        float s = (float)l / (float)L;   // 0..1
        float z = s * D;

        for (int i = 0; i <= S; ++i) {
            // угловой параметр (замыкаем кольцо дублированием первой вершины)
            float t = (float)i / (float)S;       // 0..1
            float theta = t * 2.0f * (float)M_PI;

            // базовая точка окружности на z=1
            Vec3 P = fromPolar(mCx, mCy, mR, theta);

            // масштабирование к M и перенос по Z
            mVerts.push_back({ s * P.x, s * P.y, z });
        }
    }

    // 2.a) Вентилятор от апекса к первому кольцу
    for (int i = 0; i < S; ++i) {
        unsigned v0 = apexIndex;
        unsigned v1 = ringIndex(1, i + 1);
        unsigned v2 = ringIndex(1, i);
        mIdx.push_back(v0); mIdx.push_back(v1); mIdx.push_back(v2);
    }

    // 2.b) Между кольцами: два треугольника на каждый «квад»
    for (int l = 1; l < L; ++l) {
        for (int i = 0; i < S; ++i) {
            unsigned v00 = ringIndex(l, i);
            unsigned v01 = ringIndex(l, i + 1);
            unsigned v10 = ringIndex(l + 1, i);
            unsigned v11 = ringIndex(l + 1, i + 1);

            mIdx.push_back(v00); mIdx.push_back(v01); mIdx.push_back(v10);
            mIdx.push_back(v01); mIdx.push_back(v11); mIdx.push_back(v10);
        }
    }
}

void CircleCone::draw() const {
    // Временно без освещения (нормали не считали)
    GLboolean wasLighting = glIsEnabled(GL_LIGHTING);
    if (wasLighting) glDisable(GL_LIGHTING);

    // Полупрозрачная заливка
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.25f, 0.25f, 0.7f, 0.35f);

    glBegin(GL_TRIANGLES);
    for (size_t k = 0; k + 2 < mIdx.size(); k += 3) {
        const Vec3& a = mVerts[mIdx[k + 0]];
        const Vec3& b = mVerts[mIdx[k + 1]];
        const Vec3& c = mVerts[mIdx[k + 2]];
        glVertex3f(a.x, a.y, a.z);
        glVertex3f(b.x, b.y, b.z);
        glVertex3f(c.x, c.y, c.z);
    }
    glEnd();

    // --- каркас по треугольникам (wireframe поверх заливки) ---
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.f, -1.f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glLineWidth(1.0f);
    glColor4f(0.1f, 0.1f, 0.1f, 0.9f);

    glBegin(GL_TRIANGLES);
    for (size_t k = 0; k + 2 < mIdx.size(); k += 3) {
        const Vec3& a = mVerts[mIdx[k + 0]];
        const Vec3& b = mVerts[mIdx[k + 1]];
        const Vec3& c = mVerts[mIdx[k + 2]];
        glVertex3f(a.x, a.y, a.z);
        glVertex3f(b.x, b.y, b.z);
        glVertex3f(c.x, c.y, c.z);
    }
    glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);

    if (wasLighting) glEnable(GL_LIGHTING);
}
