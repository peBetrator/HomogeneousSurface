#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "UnitPlane.hpp"

//   v10(0,0,1)      v11(1,0,1)
//        +-----------+
//        |         / |
//        | tri2   /  |
//        |       /   |
//        |     /     |
//        |   /  tri1 |
//        | /          |
//        +-----------+
//   v00(0,0,0)      v01(1,0,0)
void UnitPlane::draw() const {
    glBegin(GL_TRIANGLES);
    // tri1: v00, v01, v11
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(1.0, 0.0, 0.0);
    glVertex3d(1.0, 0.0, 1.0);
    // tri2: v00, v11, v10
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(1.0, 0.0, 1.0);
    glVertex3d(0.0, 0.0, 1.0);
    glEnd();
}
