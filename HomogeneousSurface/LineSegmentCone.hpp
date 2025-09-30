#pragma once
#include "ISurface3D.hpp"

class LineSegmentCone : public ISurface3D {
public:
    // Конструктор с параметрами по умолчанию
    LineSegmentCone(int levels = 24, int segments = 64, float depth = 3.0f);

    // Параметры (можно менять на лету перед build())
    void setLevels(int v);
    void setSegments(int v);
    void setDepth(float v);

    // ISurface3D
    void build() override;
    void draw()  const override;

    const std::vector<Vec3>& vertices() const override { return mVerts; }
    const std::vector<unsigned>& indices()  const override { return mIdx; }

private:
    static inline int   clampMin(int v, int minv) { return v < minv ? minv : v; }
    static inline float clampMin(float v, float m) { return v < m ? m : v; }

    static inline Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
        return Vec3{ a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
    }

    // параметры
    int   mLevels;
    int   mSegments;
    float mDepth;

    // геометрия
    std::vector<Vec3>   mVerts;
    std::vector<unsigned> mIdx;
};