#include "ParabolaCone.hpp"
#include <cmath>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/gl.h>

namespace {
    inline float root2() { return std::sqrt(2.0f); }
}

// Конструктор: по умолчанию — ровно дуга от (0,-4) -> (4,0) с вершиной (1,-1)
ParabolaCone::ParabolaCone(int levels, int segments, float depth)
    : mLevels(levels)
    , mSegments(segments)
    , mDepth(depth)
    , mA(-root2() / 8.0f)               // a = -√2/8
    , mUMin(-2.0f * root2())            // u_min = -2√2   -> (0,-4)
    , mUMax(+2.0f * root2())            // u_max = +2√2   -> (4,0)
    , mGamma(1.0f)                    // равномерный шаг по u
    , mUseExplicit(false)             // никакие «жёсткие точки» по умолчанию
{
}

// сеттеры
void ParabolaCone::setLevels(int v) { mLevels = v; }
void ParabolaCone::setSegments(int v) { mSegments = v; }
void ParabolaCone::setDepth(float v) { mDepth = v; }
// Если захочешь чуть «шире/уже» дугу — можно подвинуть a и/или диапазон u:
void ParabolaCone::setCurvatureA(float a) { mA = a; }
void ParabolaCone::setURange(float uMin, float uMax) { mUMin = uMin; mUMax = uMax; }
// Гамма влияет на плотность выборки вдоль u (1.0 = равномерно)
void ParabolaCone::setSamplingGamma(float g) { mGamma = (g < 0.2f ? 0.2f : g); }

// Режим «жёстких точек» — не используем тут, но оставляем совместимость
void ParabolaCone::setExplicitSamplePoints(const std::vector<Vec3>& pts) {
    mExplicitZ1.clear();
    mExplicitZ1.reserve(pts.size());
    for (auto& p : pts) mExplicitZ1.push_back({ p.x, p.y, 1.0f });
    if (mExplicitZ1.size() >= 2) mSegments = (int)mExplicitZ1.size() - 1;
    mUseExplicit = !mExplicitZ1.empty();
}

// Индекс вершины в кольце
inline unsigned ParabolaCone::ringIndex(int l, int i) const {
    return 1u + (unsigned)(l - 1) * (mSegments + 1u) + (unsigned)i;
}

// Точка дуги на плоскости z=1 по параметру u (повёрнутая парабола)
static inline Vec3 pointRotParabolaZ1(float u, float a) {
    // v = a u^2
    float v = a * u * u;
    // поворот +45° и перенос к вершине (1,-1):
    // x = 1 + (u - v)/√2
    // y = -1 + (u + v)/√2
    const float invRoot2 = 1.0f / std::sqrt(2.0f);
    float x = 1.0f + (u - v) * invRoot2;
    float y = -1.0f + (u + v) * invRoot2;
    return { x, y, 1.0f };
}

void ParabolaCone::build() {
    mVerts.clear();
    mIdx.clear();

    const int   L = clampMin(mLevels, 1);
    const int   S = clampMin(mSegments, 1);
    const float D = clampMin(mDepth, 0.0f);

    // 0) апекс
    const unsigned apexIndex = 0;
    mVerts.push_back({ 0.0f, 0.0f, 0.0f });

    // 1) кольца уровней: параметризуем ровно по u ∈ [u_min, u_max]
    for (int l = 1; l <= L; ++l) {
        float s = (float)l / (float)L;      // масштаб к апексу
        float z = s * D;

        for (int i = 0; i <= S; ++i) {
            float t = (float)i / (float)S; // 0..1
            float tn = std::pow(t, mGamma); // неравномерная выборка (по желанию)
            float u = mUMin + tn * (mUMax - mUMin);

            Vec3 P = pointRotParabolaZ1(u, mA);   // (x,y,1) РОВНО на дуге
            mVerts.push_back({ s * P.x, s * P.y, z });
        }
    }

    // 2) триангуляция
    // 2.a) вентилятор апекса к первому кольцу
    for (int i = 0; i < S; ++i) {
        mIdx.push_back(apexIndex);
        mIdx.push_back(ringIndex(1, i + 1));
        mIdx.push_back(ringIndex(1, i));
    }
    // 2.b) сшивка между кольцами
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

void ParabolaCone::draw() const {
    // временно без освещения (нормали не считаем)
    GLboolean wasLighting = glIsEnabled(GL_LIGHTING);
    if (wasLighting) glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- заливка ---
    glColor4f(0.7f, 0.25f, 0.25f, 0.35f);
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

    // --- wireframe по тем же треугольникам ---
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.f, -1.f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.0f);
    glColor4f(0.15f, 0.05f, 0.05f, 0.9f);
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
