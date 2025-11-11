#include "CustomRegionCone.hpp"
#include <cmath>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

CustomRegionCone::CustomRegionCone(int levels, int segmentsPerSide, float depth)
    : mLevels(levels), mSegmentsPerSide(segmentsPerSide), mDepth(depth) {
}

void CustomRegionCone::setLevels(int v) { mLevels = v; }
void CustomRegionCone::setSegmentsPerSide(int v) { mSegmentsPerSide = v; }
void CustomRegionCone::setDepth(float v) { mDepth = v; }

// Генерация контура области: |x| > 1, |y| > 1, |x| + |y| < 5, x≠1, y≠1
std::vector<Vec3> CustomRegionCone::generateRegionContour() {
    std::vector<Vec3> contour;
    const int segs = std::max(2, mSegmentsPerSide);
    const float eps = 0.02f; // небольшой отступ от x=1, y=1

    // Квадрант I (x>0, y>0): треугольник (1+eps,1+eps), (1+eps,4), (4,1+eps)
    // Сторона 1: x=1+eps, y от 1+eps до 4
    for (int i = 0; i < segs; ++i) {
        float t = (float)i / (float)segs;
        float x = 1.0f + eps;
        float y = (1.0f + eps) + t * (3.0f - eps); // от 1+eps до 4
        contour.push_back({x, y, 1.0f});
    }
    // Сторона 2: диагональ от (1+eps,4) до (4,1+eps), x+y=5
    for (int i = 0; i < segs; ++i) {
        float t = (float)i / (float)segs;
        float x = (1.0f + eps) + t * (3.0f - eps); // от 1+eps до 4
        float y = 5.0f - x;        // x+y=5
        contour.push_back({x, y, 1.0f});
    }
    // Сторона 3: y=1+eps, x от 4 до 1+eps
    for (int i = 0; i < segs; ++i) {
        float t = (float)i / (float)segs;
        float x = 4.0f - t * (3.0f - eps); // от 4 до 1+eps
        float y = 1.0f + eps;
        contour.push_back({x, y, 1.0f});
    }

    // Квадрант II (x<0, y>0): треугольник (-1-eps,1+eps), (-1-eps,4), (-4,1+eps)
    for (int i = 0; i < segs; ++i) {
        float t = (float)i / (float)segs;
        float x = -1.0f - eps;
        float y = (1.0f + eps) + t * (3.0f - eps);
        contour.push_back({x, y, 1.0f});
    }
    for (int i = 0; i < segs; ++i) {
        float t = (float)i / (float)segs;
        float x = (-1.0f - eps) - t * (3.0f - eps); // от -1-eps до -4
        float y = 5.0f + x;         // -x+y=5 -> y=5+x
        contour.push_back({x, y, 1.0f});
    }
    for (int i = 0; i < segs; ++i) {
        float t = (float)i / (float)segs;
        float x = -4.0f + t * (3.0f - eps); // от -4 до -1-eps
        float y = 1.0f + eps;
        contour.push_back({x, y, 1.0f});
    }

    // Квадрант III (x<0, y<0): треугольник (-1-eps,-1-eps), (-1-eps,-4), (-4,-1-eps)
    for (int i = 0; i < segs; ++i) {
        float t = (float)i / (float)segs;
        float x = -1.0f - eps;
        float y = (-1.0f - eps) - t * (3.0f - eps); // от -1-eps до -4
        contour.push_back({x, y, 1.0f});
    }
    for (int i = 0; i < segs; ++i) {
        float t = (float)i / (float)segs;
        float x = (-1.0f - eps) - t * (3.0f - eps); // от -1-eps до -4
        float y = -5.0f - x;        // -x-y=5 -> y=-5-x
        contour.push_back({x, y, 1.0f});
    }
    for (int i = 0; i < segs; ++i) {
        float t = (float)i / (float)segs;
        float x = -4.0f + t * (3.0f - eps); // от -4 до -1-eps
        float y = -1.0f - eps;
        contour.push_back({x, y, 1.0f});
    }

    // Квадрант IV (x>0, y<0): треугольник (1+eps,-1-eps), (1+eps,-4), (4,-1-eps)
    for (int i = 0; i < segs; ++i) {
        float t = (float)i / (float)segs;
        float x = 1.0f + eps;
        float y = (-1.0f - eps) - t * (3.0f - eps); // от -1-eps до -4
        contour.push_back({x, y, 1.0f});
    }
    for (int i = 0; i < segs; ++i) {
        float t = (float)i / (float)segs;
        float x = (1.0f + eps) + t * (3.0f - eps); // от 1+eps до 4
        float y = -5.0f + x;       // x-y=5 -> y=x-5
        contour.push_back({x, y, 1.0f});
    }
    for (int i = 0; i < segs; ++i) {
        float t = (float)i / (float)segs;
        float x = 4.0f - t * (3.0f - eps); // от 4 до 1+eps
        float y = -1.0f - eps;
        contour.push_back({x, y, 1.0f});
    }

    return contour;
}

void CustomRegionCone::build() {
    mVerts.clear();
    mIdx.clear();

    const int L = std::max(1, mLevels);
    const float D = std::max(0.f, mDepth);
    const int segs = std::max(2, mSegmentsPerSide);

    // Апекс
    const unsigned apex = 0;
    mVerts.push_back({0.f, 0.f, 0.f});

    // Вместо одного замкнутого контура, создадим 4 отдельных треугольника
    // Каждый треугольник имеет 3 стороны по segs точек = 3*segs точек на треугольник
    const int pointsPerTriangle = 3 * segs;

    // Генерируем все 4 треугольника
    std::vector<Vec3> contour = generateRegionContour();
    const int totalPoints = (int)contour.size();

    // Генерация колец
    auto emitRing = [&](int l) {
        float s = (float)l / (float)L;
        float z = s * D;
        for (const auto& P : contour) {
            mVerts.push_back({s * P.x, s * P.y, z});
        }
    };

    // Низ: -L..-1
    for (int l = -L; l <= -1; ++l) emitRing(l);
    // Верх: +1..+L
    for (int l = 1; l <= L; ++l) emitRing(l);

    // Веера от апекса - НО только внутри каждого треугольника, не соединяя их
    for (int tri = 0; tri < 4; ++tri) {
        int startIdx = tri * pointsPerTriangle;
        int endIdx = startIdx + pointsPerTriangle;

        for (int i = startIdx; i < endIdx - 1; ++i) {
            // Нижний веер
            mIdx.push_back(apex);
            mIdx.push_back(ringIndex(-1, i + 1));
            mIdx.push_back(ringIndex(-1, i));

            // Верхний веер
            mIdx.push_back(apex);
            mIdx.push_back(ringIndex(+1, i));
            mIdx.push_back(ringIndex(+1, i + 1));
        }
    }

    // Сшивка низ - только внутри каждого треугольника
    for (int l = -L; l <= -2; ++l) {
        for (int tri = 0; tri < 4; ++tri) {
            int startIdx = tri * pointsPerTriangle;
            int endIdx = startIdx + pointsPerTriangle;

            for (int i = startIdx; i < endIdx - 1; ++i) {
                unsigned v00 = ringIndex(l, i);
                unsigned v01 = ringIndex(l, i + 1);
                unsigned v10 = ringIndex(l + 1, i);
                unsigned v11 = ringIndex(l + 1, i + 1);
                mIdx.push_back(v00); mIdx.push_back(v01); mIdx.push_back(v10);
                mIdx.push_back(v01); mIdx.push_back(v11); mIdx.push_back(v10);
            }
        }
    }

    // Сшивка верх - только внутри каждого треугольника
    for (int l = 1; l <= L - 1; ++l) {
        for (int tri = 0; tri < 4; ++tri) {
            int startIdx = tri * pointsPerTriangle;
            int endIdx = startIdx + pointsPerTriangle;

            for (int i = startIdx; i < endIdx - 1; ++i) {
                unsigned v00 = ringIndex(l, i);
                unsigned v01 = ringIndex(l, i + 1);
                unsigned v10 = ringIndex(l + 1, i);
                unsigned v11 = ringIndex(l + 1, i + 1);
                mIdx.push_back(v00); mIdx.push_back(v01); mIdx.push_back(v10);
                mIdx.push_back(v01); mIdx.push_back(v11); mIdx.push_back(v10);
            }
        }
    }
}

void CustomRegionCone::draw() const {
    GLboolean wasLighting = glIsEnabled(GL_LIGHTING);
    if (wasLighting) glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.3f, 0.7f, 0.9f, 0.6f);

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
