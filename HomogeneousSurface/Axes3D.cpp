#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "GL/glaux.h"
#include "Axes3D.hpp"

void Axes3D::draw() const {
    glLineWidth(1.5f);
    glEnable(GL_LINE_SMOOTH);

    // Ось X
    glBegin(GL_LINES);
    glColor3d(0, 0, 0); glVertex3d(-5.5, 0, 0);
    glColor3d(1, 0, 0); glVertex3d(5.5, 0, 0);
    glEnd();
    glPushMatrix();
    glColor3d(1, 0, 0);
    glTranslated(5.3, 0, 0);
    glRotated(90, 0, 1, 0);
    auxSolidCone(0.1, 0.2);
    glPopMatrix();

    // Ось Y
    glBegin(GL_LINES);
    glColor3d(0, 0, 0); glVertex3d(0, -5.5, 0);
    glColor3d(0, 1, 0); glVertex3d(0, 5.5, 0);
    glEnd();
    glPushMatrix();
    glColor3d(0, 1, 0);
    glTranslated(0, 5.3, 0);
    glRotated(-90, 1, 0, 0);
    auxSolidCone(0.1, 0.2);
    glPopMatrix();

    // Ось Z
    glBegin(GL_LINES);
    glColor3d(0, 0, 0); glVertex3d(0, 0, -5.5);
    glColor3d(0, 0, 1); glVertex3d(0, 0, 5.5);
    glEnd();
    glPushMatrix();
    glColor3d(0, 0, 1);
    glTranslated(0, 0, 5.3);
    auxSolidCone(0.1, 0.2);
    glPopMatrix();
}
