#pragma once
#include "Vec3.hpp"

class HalfConeSweep {
    int nt, ns;
    Vec3* stripA, * stripB;
public:
    HalfConeSweep(int tSteps = 64, int sSteps = 24);
    ~HalfConeSweep();
    void draw(double R, double K, double r0, double r1); // полу-конус по кривой
};
