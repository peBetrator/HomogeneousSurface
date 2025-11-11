#include "AstroidCone.hpp"
#include <cmath>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AstroidCone::AstroidCone(int levels, int segments, float depth)
    : mLevels(levels), mSegments(segments), mDepth(depth), mSize(2.0f) {
}

void AstroidCone::setLevels(int v) { mLevels = v; }
void AstroidCone::setSegments(int v) { mSegments = v; }
void AstroidCone::setDepth(float v) { mDepth = v; }
void AstroidCone::setSize(float size) { mSize = size; }

void AstroidCone::build() {
    mVerts.clear();
    mIdx.clear();

    const int   L = std::max(1, mLevels);
    const int   S = std::max(1, mSegments);
    const float D = std::max(0.f, mDepth);

    // Апекс (вершина конуса)
    const unsigned apex = 0;
    mVerts.push_back({ 0.f, 0.f, 0.f });

    // Функция для точки на астроиде в плоскости z=1
    auto pointOnAstroidZ1 = [&](float t)->Vec3 {
        float ang = t * 2.f * (float)M_PI;
        float ct = std::cos(ang);
        float st = std::sin(ang);
        return {
            mSize * ct * ct * ct,  // x = a * cos³(t)
            mSize * st * st * st,  // y = a * sin³(t)
            1.f
        };
    };

    // Генерация колец на разных уровнях
    auto emitRing = [&](int l) {
        float s = (float)l / (float)L;   // s ∈ [-1..-1/L] U [1/L..1]
        float z = s * D;
        for (int i = 0; i <= S; ++i) {
            float t = (float)i / (float)S;
            Vec3  P = pointOnAstroidZ1(t);
            mVerts.push_back({ s * P.x, s * P.y, z });
        }
    };

    // низ: -L..-1
    for (int l = -L; l <= -1; ++l) emitRing(l);
    // верх: +1..+L
    for (int l = 1; l <= L; ++l) emitRing(l);

    // Веера от апекса к первым кольцам
    for (int i = 0; i < S; ++i) {
        // нижний веер
        mIdx.push_back(apex);
        mIdx.push_back(ringIndex(-1, i + 1));
        mIdx.push_back(ringIndex(-1, i));

        // верхний веер
        mIdx.push_back(apex);
        mIdx.push_back(ringIndex(+1, i));
        mIdx.push_back(ringIndex(+1, i + 1));
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

void AstroidCone::draw() const {
    // Временно без освещения
    GLboolean wasLighting = glIsEnabled(GL_LIGHTING);
    if (wasLighting) glDisable(GL_LIGHTING);

    // Полупрозрачная заливка
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Красивый цвет для астроиды
    glColor4f(0.9f, 0.3f, 0.4f, 0.5f);  // Розовато-красный

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

    if (wasLighting) glEnable(GL_LIGHTING);
}
