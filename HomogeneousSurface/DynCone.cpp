#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "DynCone.hpp"
#include <cmath>

DynCone::DynCone(int heightSeg, int sectors)
    : nh(heightSeg < 1 ? 1 : heightSeg),
    ns(sectors < 3 ? 3 : sectors),
    rings(new Vec3[nh * ns]) {
    const double PI = 3.14159265358979323846;
    for (int j = 0;j < ns;++j) {
        double a = 2.0 * PI * j / ns;
        rings[(nh - 1) * ns + j] = { std::cos(a), std::sin(a), 1.0 };
    }
    for (int i = 0;i < nh - 1;++i) {
        double t = double(i + 1) / nh;
        for (int j = 0;j < ns;++j) {
            const Vec3& b = rings[(nh - 1) * ns + j];
            rings[i * ns + j] = { b.x * t, b.y * t, t };
        }
    }
}

DynCone::~DynCone() { delete[] rings; }

void DynCone::draw() const {
    glBegin(GL_TRIANGLE_FAN);
    glVertex3d(0, 0, 0);
    for (int j = 0;j < ns;++j) glVertex3d(rings[j].x, rings[j].y, rings[j].z);
    glVertex3d(rings[0].x, rings[0].y, rings[0].z);
    glEnd();

    for (int i = 0;i < nh - 1;++i) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0;j < ns;++j) {
            const Vec3& a = rings[i * ns + j];
            const Vec3& b = rings[(i + 1) * ns + j];
            glVertex3d(a.x, a.y, a.z);
            glVertex3d(b.x, b.y, b.z);
        }
        const Vec3& a0 = rings[i * ns + 0];
        const Vec3& b0 = rings[(i + 1) * ns + 0];
        glVertex3d(a0.x, a0.y, a0.z);
        glVertex3d(b0.x, b0.y, b0.z);
        glEnd();
    }
}
