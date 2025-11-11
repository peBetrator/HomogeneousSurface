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

    const int   L = std::max(1, mLevels);
    const int   S = std::max(1, mSegments);
    const float D = std::max(0.f, mDepth);

    const unsigned apex = 0;
    mVerts.push_back({ 0.f, 0.f, 0.f });

    auto pointOnCircleZ1 = [&](float t)->Vec3 {
        // окружность центра (-2, 2), радиус sqrt(2)
        const float cx = -2.f, cy = 2.f, R = std::sqrt(2.f);
        float ang = t * 2.f * 3.1415926535f;
        return { cx + R * std::cos(ang), cy + R * std::sin(ang), 1.f };
        };

    auto emitRing = [&](int l) {
        float s = (float)l / (float)L;   // s ∈ [-1..-1/L] U [1/L..1]
        float z = s * D;
        for (int i = 0; i <= S; ++i) {
            float t = (float)i / (float)S;
            Vec3  P = pointOnCircleZ1(t);
            mVerts.push_back({ s * P.x, s * P.y, z });
        }
        };

    // низ: -L..-1
    for (int l = -L; l <= -1; ++l) emitRing(l);
    // верх: +1..+L
    for (int l = 1; l <= L; ++l) emitRing(l);

    // Веера от апекса к первым кольцам
    for (int i = 0; i < S; ++i) {
        // нижний веер (если включишь GL_CULL_FACE и нужен наружный фейс, поменяй местами v1/v2)
        mIdx.push_back(apex); mIdx.push_back(ringIndex(-1, i + 1)); mIdx.push_back(ringIndex(-1, i));
        // верхний веер
        mIdx.push_back(apex); mIdx.push_back(ringIndex(+1, i + 1)); mIdx.push_back(ringIndex(+1, i));
    }

    // сшивка низ: l = -L..-2
    for (int l = -L; l <= -2; ++l) {
        for (int i = 0; i < S; ++i) {
            unsigned v00 = ringIndex(l, i);
            unsigned v01 = ringIndex(l, i + 1);
            unsigned v10 = ringIndex(l + 1, i);
            unsigned v11 = ringIndex(l + 1, i + 1);
            mIdx.push_back(v00); mIdx.push_back(v01); mIdx.push_back(v10);
            mIdx.push_back(v01); mIdx.push_back(v11); mIdx.push_back(v10);
        }
    }

    // сшивка верх: l = 1..L-1
    for (int l = 1; l <= L - 1; ++l) {
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
