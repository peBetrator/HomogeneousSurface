#pragma once
#include "ISurface3D.hpp"
#include <cmath>
#include <vector>

// Фигура на основе системы неравенств:
// |x| > 1
// |y| > 1
// |x| + |y| < 5
// x ≠ 1, y ≠ 1 (автоматически выполняются из |x|>1, |y|>1)
class CustomRegionCone : public ISurface3D {
public:
    CustomRegionCone(int levels = 12, int segmentsPerSide = 16, float depth = 3.0f);

    void setLevels(int v);
    void setSegmentsPerSide(int v);
    void setDepth(float v);

    void build() override;
    void draw() const override;

    const std::vector<Vec3>& vertices() const override { return mVerts; }
    const std::vector<unsigned>& indices() const override { return mIdx; }

protected:
    int levels() const { return mLevels; }
    int segmentsPerSide() const { return mSegmentsPerSide; }

private:
    int   mLevels;
    int   mSegmentsPerSide; // сегментов на каждой стороне треугольника
    float mDepth;

    std::vector<Vec3>     mVerts;
    std::vector<unsigned> mIdx;

    // Генерация контура области в плоскости z=1
    std::vector<Vec3> generateRegionContour();

    static inline int clampMin(int v, int minv) { return v < minv ? minv : v; }
    static inline float clampMin(float v, float m) { return v < m ? m : v; }

    inline unsigned ringIndex(int l, int i) const {
        const int L = mLevels;
        const int totalPoints = (int)((mVerts.size() - 1) / (2 * L));
        int ordinal = (l < 0) ? (l + L) : (l - 1 + L);
        return 1u + (unsigned)ordinal * totalPoints + (unsigned)i;
    }
};
