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
    glBegin(GL_TRIANGLES);

    // Крышка (веер) независимыми треугольниками: вершина -> два соседних на первом кольце
    for (int j = 0; j < ns; ++j) {
        const Vec3& v0 = rings[0 * ns + j];
        const Vec3& v1 = rings[0 * ns + ((j + 1) % ns)];
        glVertex3d(0.0, 0.0, 0.0);
        glVertex3d(v0.x, v0.y, v0.z);
        glVertex3d(v1.x, v1.y, v1.z);
    }

    // Боковина: каждая ячейка сетки = 2 независимых треугольника
    for (int i = 0; i < nh - 1; ++i) {
        for (int j = 0; j < ns; ++j) {
            int jn = (j + 1) % ns;
            const Vec3& v00 = rings[i * ns + j];
            const Vec3& v10 = rings[(i + 1) * ns + j];
            const Vec3& v01 = rings[i * ns + jn];
            const Vec3& v11 = rings[(i + 1) * ns + jn];

            // tri1
            glVertex3d(v00.x, v00.y, v00.z);
            glVertex3d(v10.x, v10.y, v10.z);
            glVertex3d(v01.x, v01.y, v01.z);

            // tri2
            glVertex3d(v01.x, v01.y, v01.z);
            glVertex3d(v10.x, v10.y, v10.z);
            glVertex3d(v11.x, v11.y, v11.z);
        }
    }

    glEnd();
}
