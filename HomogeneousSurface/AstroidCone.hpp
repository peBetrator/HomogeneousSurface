#pragma once
#include "ISurface3D.hpp"
#include <cmath>

class AstroidCone : public ISurface3D {
public:
    AstroidCone(int levels = 12, int segments = 64, float depth = 3.0f);

    void setLevels(int v);
    void setSegments(int v);
    void setDepth(float v);
    void setSize(float size); // Размер астроиды

    void build() override;
    void draw() const override;

    const std::vector<Vec3>& vertices() const override { return mVerts; }
    const std::vector<unsigned>& indices() const override { return mIdx; }

protected:
    int levels() const { return mLevels; }
    int segments() const { return mSegments; }

private:
    // параметры построения
    int   mLevels;
    int   mSegments;
    float mDepth;
    float mSize; // Размер астроиды

    // геометрия
    std::vector<Vec3>     mVerts;
    std::vector<unsigned> mIdx;

    static inline int   clampMin(int v, int minv) { return v < minv ? minv : v; }
    static inline float clampMin(float v, float m) { return v < m ? m : v; }

    // Вычисление точки на астроиде: x = a*cos³(t), y = a*sin³(t)
    static inline Vec3 astroidPoint(float size, float angleRad) {
        float ct = std::cos(angleRad);
        float st = std::sin(angleRad);
        return Vec3{
            size * ct * ct * ct,  // x = a * cos³(t)
            size * st * st * st,  // y = a * sin³(t)
            1.0f
        };
    }

    // маппер индексов по кольцу/углу
    inline unsigned ringIndex(int l, int i) const {
        const int L = mLevels;
        const int S = mSegments;
        int ordinal = (l < 0) ? (l + L) : (l - 1 + L);
        return 1u + (unsigned)ordinal * (S + 1u) + (unsigned)i;
    }
};
