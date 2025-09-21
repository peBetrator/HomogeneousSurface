#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "CurvedSheet.hpp"
#include <cmath>
#include <algorithm>

// --- параметры касания и середины ---
static const double EPS = 1e-3;   // «почти на оси»
static const double Xend = 3.0;    // конец на оси x: (3, 0)
static const double Ybeg = 3.0;    // начало на оси y: (0, -3)

// середина пересечения с соединительной плоскостью
static const double XM = 1.0;      // >0
static const double YM = -1.0;     // <0

// Кубический Безье полностью в ПНижн четверти:
// P0 = (EPS, -Ybeg)  — почти на оси y, горизонтальная касательная
// P3 = (Xend, -EPS)  — почти на оси x, вертикальная касательная
// P1 = P0 + (tx, 0),  P2 = P3 + (0, ty)
// Условие B(0.5) = (XM, YM)  ⇒
//    tx = (8*XM - 4*P0.x - 4*P3.x)/3
//    ty = (8*YM - 4*P0.y - 4*P3.y)/3
static inline Vec3 bezier(double t)
{
    const Vec3 P0{ EPS,  -Ybeg, 0.0 };   // почти на Oy
    const Vec3 P2{ Xend, -EPS,  0.0 };   // почти на Ox

    // Для квадр. Безье: B(0.5) = (P0 + 2*P1 + P2)/4 = M  ⇒  P1 = 2*M - (P0+P2)/2
    const Vec3 M{ XM, YM, 0.0 };
    const Vec3 P1{
        2.0 * M.x - 0.5 * (P0.x + P2.x),
        2.0 * M.y - 0.5 * (P0.y + P2.y),
        0.0
    };

    const double u = 1.0 - t;
    const double b0 = u * u;
    const double b1 = 2.0 * u * t;
    const double b2 = t * t;

    Vec3 p{
        b0 * P0.x + b1 * P1.x + b2 * P2.x,
        b0 * P0.y + b1 * P1.y + b2 * P2.y,
        0.0
    };

    // страховка — не дать точкам пересечь оси
    p.x = std::max<double>(p.x, EPS);   // x >= EPS
    p.y = std::min<double>(p.y, -EPS);  // y <= -EPS
    return p;
}

CurvedSheet::CurvedSheet(int tSteps)
    : nt(tSteps < 2 ? 2 : tSteps)
{
    edge0 = new Vec3[nt + 1];
    edge1 = new Vec3[nt + 1];
}

CurvedSheet::~CurvedSheet()
{
    delete[] edge0;
    delete[] edge1;
}

void CurvedSheet::draw(double /*R*/, double /*K*/)
{
    for (int i = 0; i <= nt; ++i) {
        double t = double(i) / nt;
        Vec3 p = bezier(t);                 // x>0, y<0; середина в (XM,YM)
        edge0[i] = { p.x, p.y, 0.0 };
        edge1[i] = { p.x, p.y, 1.0 };
    }

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < nt; ++i) {
        const Vec3& a0 = edge0[i];
        const Vec3& a1 = edge0[i + 1];
        const Vec3& b0 = edge1[i];
        const Vec3& b1 = edge1[i + 1];

        // tri1
        glVertex3d(a0.x, a0.y, a0.z);
        glVertex3d(b0.x, b0.y, b0.z);
        glVertex3d(a1.x, a1.y, a1.z);
        // tri2
        glVertex3d(a1.x, a1.y, a1.z);
        glVertex3d(b0.x, b0.y, b0.z);
        glVertex3d(b1.x, b1.y, b1.z);
    }
    glEnd();
}
