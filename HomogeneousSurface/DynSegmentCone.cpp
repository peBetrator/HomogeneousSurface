#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "DynSegmentCone.hpp"
#include <cmath>

DynSegmentCone::DynSegmentCone(int sectors) : ns(sectors < 3 ? 3 : sectors) {
    ring0 = new Vec3[ns]; ring1 = new Vec3[ns];
}
DynSegmentCone::~DynSegmentCone() { delete[] ring0; delete[] ring1; }

void DynSegmentCone::draw(const Vec3& p0, const Vec3& p1, double r0, double r1) {
    Vec3 d{ p1.x - p0.x, p1.y - p0.y, p1.z - p0.z };
    double L = std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
    if (L <= 1e-9) return;

    Vec3 w{ d.x / L, d.y / L, d.z / L };
    Vec3 base{ 0,0,1 }; if (std::fabs(w.z) > 0.9) base = Vec3{ 1,0,0 };
    Vec3 u{ w.y * base.z - w.z * base.y, w.z * base.x - w.x * base.z, w.x * base.y - w.y * base.x };
    double ul = std::sqrt(u.x * u.x + u.y * u.y + u.z * u.z); u = { u.x / ul,u.y / ul,u.z / ul };
    Vec3 v{ w.y * u.z - w.z * u.y, w.z * u.x - w.x * u.z, w.x * u.y - w.y * u.x };

    const double PI = 3.14159265358979323846;
    for (int j = 0;j < ns;++j) {
        double a = 2.0 * PI * j / ns, ca = std::cos(a), sa = std::sin(a);
        ring0[j] = { p0.x + r0 * (u.x * ca + v.x * sa),
                     p0.y + r0 * (u.y * ca + v.y * sa),
                     p0.z + r0 * (u.z * ca + v.z * sa) };
        ring1[j] = { p1.x + r1 * (u.x * ca + v.x * sa),
                     p1.y + r1 * (u.y * ca + v.y * sa),
                     p1.z + r1 * (u.z * ca + v.z * sa) };
    }

    glBegin(GL_TRIANGLES);
    for (int j = 0;j < ns;++j) {
        int jn = (j + 1) % ns;
        const Vec3& a0 = ring0[j];
        const Vec3& a1 = ring0[jn];
        const Vec3& b0 = ring1[j];
        const Vec3& b1 = ring1[jn];
        // tri1: a0, b0, a1
        glVertex3d(a0.x, a0.y, a0.z);
        glVertex3d(b0.x, b0.y, b0.z);
        glVertex3d(a1.x, a1.y, a1.z);
        // tri2: a1, b0, b1
        glVertex3d(a1.x, a1.y, a1.z);
        glVertex3d(b0.x, b0.y, b0.z);
        glVertex3d(b1.x, b1.y, b1.z);
    }
    glEnd();
}

