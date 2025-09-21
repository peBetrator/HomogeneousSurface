#pragma once
#include "Vec3.hpp"

// Изогнутая плоскость вдоль кривой: для каждого шага t берём две точки
// с z=0 и z=1 и сшиваем соседние сечения двумя треугольниками.
class CurvedSheet {
    int nt;           // шаги по кривой
    Vec3* edge0, * edge1; // z=0 и z=1 для каждого t
public:
    explicit CurvedSheet(int tSteps = 64);
    ~CurvedSheet();

    // R,K — параметры кривой (как раньше); высота задаётся масштабом по Z извне
    void draw(double R, double K);
};
