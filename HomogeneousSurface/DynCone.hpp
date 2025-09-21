#pragma once
#include "Vec3.hpp"

class DynCone {
    int nh, ns;
    Vec3* rings; // nh*ns
public:
    DynCone(int heightSeg, int sectors);
    ~DynCone();
    void draw() const; // вызывает OpenGL
};
