#pragma once
#include "Vec3.hpp"

class DynSegmentCone {
    int ns;
    Vec3* ring0, * ring1;
public:
    explicit DynSegmentCone(int sectors = 24);
    ~DynSegmentCone();
    void draw(const Vec3& p0, const Vec3& p1, double r0, double r1); // усечённый конус/цилиндр
};
