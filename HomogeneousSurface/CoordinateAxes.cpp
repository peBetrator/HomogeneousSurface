#ifdef _WIN32#include "CoordinateAxes.hpp"
#define NOMINMAX
#include <windows.h>
#endif

#include "CoordinateAxes.hpp"

void CoordinateAxes::draw() const
{
    // сохранить состояние
    GLboolean wasLineSmooth = glIsEnabled(GL_LINE_SMOOTH);
    GLboolean wasPointSmooth = glIsEnabled(GL_POINT_SMOOTH);

    if (mUseLineSmooth) glEnable(GL_LINE_SMOOTH); else glDisable(GL_LINE_SMOOTH);

    // точка-начало координат
    if (mShowOriginPoint) {
        glPointSize(10.0f);
        glEnable(GL_POINT_SMOOTH);
        setColor(mZeroColor);
        glBegin(GL_POINTS);
        glVertex3f(0.f, 0.f, 0.f);
        glEnd();
        if (!wasPointSmooth) glDisable(GL_POINT_SMOOTH);
    }

    // оси
    glLineWidth(mLineWidth);
    glBegin(GL_LINES);
    // X
    setColor(mZeroColor); glVertex3f(-mLength, 0.f, 0.f);
    setColor(mXColor);    glVertex3f(mLength, 0.f, 0.f);
    // Y
    setColor(mZeroColor); glVertex3f(0.f, -mLength, 0.f);
    setColor(mYColor);    glVertex3f(0.f, mLength, 0.f);
    // Z
    setColor(mZeroColor); glVertex3f(0.f, 0.f, -mLength);
    setColor(mZColor);    glVertex3f(0.f, 0.f, mLength);
    glEnd();

    // стрелочки на положительных концах (как у тебя было)
    setColor(mXColor);
    drawArrowAlongX(mLength - mArrowHeight * 1.0f, mArrowRadius, mArrowHeight);

    setColor(mYColor);
    drawArrowAlongY(mLength - mArrowHeight * 1.0f, mArrowRadius, mArrowHeight);

    setColor(mZColor);
    drawArrowAlongZ(mLength - mArrowHeight * 1.0f, mArrowRadius, mArrowHeight);

    // восстановить состояние
    if (!wasLineSmooth) glDisable(GL_LINE_SMOOTH);
}

void CoordinateAxes::drawArrowAlongX(GLfloat x, GLfloat radius, GLfloat height)
{
    glPushMatrix();
    glTranslatef(x, 0.f, 0.f);
    glRotatef(90.f, 0.f, 1.f, 0.f); // вдоль +X
    auxSolidCone(radius, height);
    glPopMatrix();
}

void CoordinateAxes::drawArrowAlongY(GLfloat y, GLfloat radius, GLfloat height)
{
    glPushMatrix();
    glTranslatef(0.f, y, 0.f);
    glRotatef(-90.f, 1.f, 0.f, 0.f); // вдоль +Y
    auxSolidCone(radius, height);
    glPopMatrix();
}

void CoordinateAxes::drawArrowAlongZ(GLfloat z, GLfloat radius, GLfloat height)
{
    glPushMatrix();
    glTranslatef(0.f, 0.f, z);       // вдоль +Z (без поворота)
    auxSolidCone(radius, height);
    glPopMatrix();
}
