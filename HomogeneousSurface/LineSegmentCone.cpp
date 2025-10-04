#include "LineSegmentCone.hpp"
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/gl.h>

LineSegmentCone::LineSegmentCone(int levels, int segments, float depth)
    : mLevels(levels), mSegments(segments), mDepth(depth) {
}

void LineSegmentCone::setLevels(int v) { mLevels = v; }
void LineSegmentCone::setSegments(int v) { mSegments = v; }
void LineSegmentCone::setDepth(float v) { mDepth = v; }

void LineSegmentCone::build() {
    mVerts.clear();
    mIdx.clear();

    const int   L = clampMin(mLevels, 1);
    const int   S = clampMin(mSegments, 1);
    const float D = clampMin(mDepth, 0.0f);

    const Vec3 A = { -1.0f,  1.0f, 1.0f };
    const Vec3 B = { 1.0f, -1.0f, 1.0f };

    const unsigned apexIndex = 0;
    mVerts.push_back({ 0.f, 0.f, 0.f });

    auto lerp = [](const Vec3& a, const Vec3& b, float t)->Vec3 {
        return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, 1.0f };
        };

    // если не выносишь в .hpp, можно так:
    auto ringIndexLocal = [&](int l, int i)->unsigned {
        int ordinal = (l < 0) ? (l + L) : (l - 1 + L);
        return 1u + (unsigned)ordinal * (S + 1u) + (unsigned)i;
        };

    auto emitRing = [&](int l) {
        float s = (float)l / (float)L; // s ∈ [-1..-1/L] U [1/L..1]
        float z = s * D;
        for (int i = 0; i <= S; ++i) {
            float t = (float)i / (float)S;
            Vec3  P = lerp(A, B, t);      // точка на отрезке на плоскости z=1
            mVerts.push_back({ s * P.x, s * P.y, z });
        }
        };

    // нижние уровни: -L..-1
    for (int l = -L; l <= -1; ++l) emitRing(l);
    // верхние уровни: +1..+L
    for (int l = 1; l <= L; ++l) emitRing(l);

    // веера от апекса к первым кольцам
    for (int i = 0; i < S; ++i) {
        // низ (если включён GL_CULL_FACE и нужен одинаковый «внешний» фейс —
        // можно поменять местами последние два индекса)
        mIdx.push_back(apexIndex);
        mIdx.push_back(ringIndexLocal(-1, i + 1));
        mIdx.push_back(ringIndexLocal(-1, i));

        // верх
        mIdx.push_back(apexIndex);
        mIdx.push_back(ringIndexLocal(+1, i + 1));
        mIdx.push_back(ringIndexLocal(+1, i));
    }

    // сшивка колец: низ
    for (int l = -L; l <= -2; ++l) {
        for (int i = 0; i < S; ++i) {
            unsigned v00 = ringIndexLocal(l, i);
            unsigned v01 = ringIndexLocal(l, i + 1);
            unsigned v10 = ringIndexLocal(l + 1, i);
            unsigned v11 = ringIndexLocal(l + 1, i + 1);
            mIdx.push_back(v00); mIdx.push_back(v01); mIdx.push_back(v10);
            mIdx.push_back(v01); mIdx.push_back(v11); mIdx.push_back(v10);
        }
    }
    // сшивка колец: верх
    for (int l = 1; l <= L - 1; ++l) {
        for (int i = 0; i < S; ++i) {
            unsigned v00 = ringIndexLocal(l, i);
            unsigned v01 = ringIndexLocal(l, i + 1);
            unsigned v10 = ringIndexLocal(l + 1, i);
            unsigned v11 = ringIndexLocal(l + 1, i + 1);
            mIdx.push_back(v00); mIdx.push_back(v01); mIdx.push_back(v10);
            mIdx.push_back(v01); mIdx.push_back(v11); mIdx.push_back(v10);
        }
    }
}

void LineSegmentCone::draw() const {
    // временно без освещения (нормали не считаем)
    GLboolean wasLighting = glIsEnabled(GL_LIGHTING);
    if (wasLighting) glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- заливка треугольниками ---
    glColor4f(0.2f, 0.2f, 0.2f, 0.35f);
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