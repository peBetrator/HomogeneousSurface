#pragma once
#include "ISurface3D.hpp"

#include <cmath>

class CircleCone : public ISurface3D {
public:
    CircleCone(int levels = 24, int segments = 128, float depth = 3.0f);

    void setLevels(int v);
    void setSegments(int v);
    void setDepth(float v);
    void setCircle(float cx, float cy, float r); // опционально, если захочешь менять

    void build() override;
    void draw()  const override;

    const std::vector<Vec3>& vertices() const override { return mVerts; }
    const std::vector<unsigned>& indices()  const override { return mIdx; }

private:
    // параметры построения
    int   mLevels;
    int   mSegments;
    float mDepth;

    // окружность на плоскости z=1
    float mCx, mCy, mR;

    // геометрия
    std::vector<Vec3>     mVerts;
    std::vector<unsigned> mIdx;

    static inline int   clampMin(int v, int minv) { return v < minv ? minv : v; }
    static inline float clampMin(float v, float m) { return v < m ? m : v; }

    static inline Vec3 fromPolar(float cx, float cy, float r, float angRad) {
        return Vec3{ cx + r * std::cos(angRad), cy + r * std::sin(angRad), 1.0f };
    }

    // удобный маппер индексов по кольцу/углу
    inline unsigned ringIndex(int l, int i) const {
        const int L = mLevels;
        const int S = mSegments;
        // -L..-1 -> 0..L-1 ; +1..+L -> L..2L-1
        int ordinal = (l < 0) ? (l + L) : (l - 1 + L);
        return 1u + (unsigned)ordinal * (S + 1u) + (unsigned)i;
    }
};
