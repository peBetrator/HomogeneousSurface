#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "HalfConeSweep.hpp"

#include <cmath>
#include <algorithm>

static inline Vec3 curve(double t, double R, double K) {
    return { R * t, -K * std::sqrt(std::max<double>(0.0, t)), 0.0 };
}
static inline double radiusLerp(double t, double r0, double r1) {
    return r0 * (1.0 - t) + r1 * t;
}

HalfConeSweep::HalfConeSweep(int tSteps, int sSteps)
    : nt(tSteps < 2 ? 2 : tSteps), ns(sSteps < 3 ? 3 : sSteps) {
    stripA = new Vec3[ns + 1];
    stripB = new Vec3[ns + 1];
}

HalfConeSweep::~HalfConeSweep() { delete[] stripA; delete[] stripB; }

void HalfConeSweep::draw(double R, double K, double r0, double r1) {
    const double PI = 3.14159265358979323846;

    auto makeStrip = [&](const Vec3& P, const Vec3& T, double rad, Vec3* out) {
        Vec3 w = T; double wl = std::sqrt(w.x * w.x + w.y * w.y + w.z * w.z); w = { w.x / wl,w.y / wl,w.z / wl };
        Vec3 base{ 0,0,1 }; if (std::fabs(w.z) > 0.9) base = { 1,0,0 };
        Vec3 u{ w.y * base.z - w.z * base.y, w.z * base.x - w.x * base.z, w.x * base.y - w.y * base.x };
        double ul = std::sqrt(u.x * u.x + u.y * u.y + u.z * u.z); u = { u.x / ul,u.y / ul,u.z / ul };
        Vec3 v{ w.y * u.z - w.z * u.y, w.z * u.x - w.x * u.z, w.x * u.y - w.y * u.x };

        for (int j = 0; j <= ns; ++j) {
            double a = (-0.5 + double(j) / ns) * PI; // [-pi/2..+pi/2]
            double ca = std::cos(a), sa = std::sin(a);
            out[j] = { P.x + rad * (u.x * ca + v.x * sa),
                       P.y + rad * (u.y * ca + v.y * sa),
                       P.z + rad * (u.z * ca + v.z * sa) };
        }
        };

    for (int i = 1; i <= nt; ++i) {
        double t0 = double(i - 1) / nt, t1 = double(i) / nt;
        Vec3 P0 = curve(t0, R, K), P1 = curve(t1, R, K);
        Vec3 T{ P1.x - P0.x, P1.y - P0.y, P1.z - P0.z };
        double r0i = radiusLerp(t0, r0, r1), r1i = radiusLerp(t1, r0, r1);

        makeStrip(P0, T, r0i, stripA);
        makeStrip(P1, T, r1i, stripB);

        glBegin(GL_TRIANGLES);
        for (int j = 0; j < ns; ++j) {
            const Vec3& a0 = stripA[j];
            const Vec3& a1 = stripA[j + 1];
            const Vec3& b0 = stripB[j];
            const Vec3& b1 = stripB[j + 1];

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
}
