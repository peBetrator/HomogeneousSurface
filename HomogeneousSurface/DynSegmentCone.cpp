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
    Vec3 base{ 0,0,1 }; if (std::fabs(w.z) > 0.9) base = { 1,0,0 };
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

    glBegin(GL_TRIANGLE_STRIP);
    for (int j = 0;j < ns;++j) {
        glVertex3d(ring0[j].x, ring0[j].y, ring0[j].z);
        glVertex3d(ring1[j].x, ring1[j].y, ring1[j].z);
    }
    glVertex3d(ring0[0].x, ring0[0].y, ring0[0].z);
    glVertex3d(ring1[0].x, ring1[0].y, ring1[0].z);
    glEnd();
}
