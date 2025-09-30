#include "LineSegmentCone.hpp"
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/gl.h>    // или <GL/glut.h>/<GL/freeglut.h> — как у тебя в проекте

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

    // A(-1,1,1) -> B(1,-1,1) на z=1
    const Vec3 A = { -1.0f,  1.0f, 1.0f };
    const Vec3 B = { 1.0f, -1.0f, 1.0f };

    // 0) апекс M(0,0,0)
    const unsigned apexIndex = 0;
    mVerts.push_back({ 0.0f, 0.0f, 0.0f });

    // удобный доступ к индексу в кольце
    auto ringIndex = [&](int l, int i)->unsigned {
        return 1u + (unsigned)(l - 1) * (S + 1u) + (unsigned)i;
        };

    // 1) кольца уровней
    for (int l = 1; l <= L; ++l) {
        float s = (float)l / (float)L;   // 0..1
        float z = s * D;
        for (int i = 0; i <= S; ++i) {
            float t = (float)i / (float)S;
            Vec3 P = lerp(A, B, t);          // точка на отрезке (z=1)
            mVerts.push_back({ s * P.x, s * P.y, z });
        }
    }

    // 2.a) вентилятор апекса к первому кольцу
    for (int i = 0; i < S; ++i) {
        unsigned v0 = apexIndex;
        unsigned v1 = ringIndex(1, i + 1);
        unsigned v2 = ringIndex(1, i);
        mIdx.push_back(v0); mIdx.push_back(v1); mIdx.push_back(v2);
    }

    // 2.b) между уровнями
    for (int l = 1; l < L; ++l) {
        for (int i = 0; i < S; ++i) {
            unsigned v00 = ringIndex(l, i);
            unsigned v01 = ringIndex(l, i + 1);
            unsigned v10 = ringIndex(l + 1, i);
            unsigned v11 = ringIndex(l + 1, i + 1);

            // два треугольника из «квада»
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