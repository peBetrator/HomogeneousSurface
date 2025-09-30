// ParabolaCone.hpp
#pragma once
#include "ISurface3D.hpp"
#include <vector>

class ParabolaCone : public ISurface3D {
public:
    // levels: число слоёв, segments: разбиение вдоль параметра u, depth: глубина по Z
    ParabolaCone(int levels = 24, int segments = 256, float depth = 3.0f);

    // Параметры сетки
    void setLevels(int v);
    void setSegments(int v);
    void setDepth(float v);

    // Парабола, повёрнутая на +45°: v = a*u^2 (вершина в (1,-1))
    void setCurvatureA(float a);
    void setURange(float uMin, float uMax);

    // Неравномерная выборка по u (1.0 = равномерно)
    void setSamplingGamma(float g);

    // Режим «жёстких опорных точек» (не нужен для текущей дуги, но оставлен для совместимости)
    void setExplicitSamplePoints(const std::vector<Vec3>& pts);

    // ISurface3D
    void build() override;
    void draw()  const override;

    const std::vector<Vec3>& vertices() const override { return mVerts; }
    const std::vector<unsigned>& indices()  const override { return mIdx; }

private:
    // --- сетка ---
    int   mLevels;
    int   mSegments;
    float mDepth;

    // --- параметры повёрнутой параболы ---
    // v = a*u^2, u ∈ [mUMin, mUMax], вершина в (1,-1)
    float mA;      // кривизна
    float mUMin;   // левый предел по u
    float mUMax;   // правый предел по u
    float mGamma;  // степень неравномерной выборки вдоль u

    // --- опциональный режим «жёстких точек» на z=1 ---
    bool mUseExplicit;
    std::vector<Vec3> mExplicitZ1;

    // --- геометрия ---
    std::vector<Vec3>     mVerts;
    std::vector<unsigned> mIdx;

    // утилиты
    static inline int   clampMin(int v, int minv) { return v < minv ? minv : v; }
    static inline float clampMin(float v, float m) { return v < m ? m : v; }
    inline unsigned ringIndex(int l, int i) const;  // объявление для .cpp
};
