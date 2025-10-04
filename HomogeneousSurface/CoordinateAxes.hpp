#pragma once
#include <GL/gl.h>
#include "GL/glaux.h"

class CoordinateAxes {
public:
    struct Color3 { GLfloat r, g, b; };

    CoordinateAxes()
        : mLength(5.5f),
        mLineWidth(1.5f),
        mArrowRadius(0.1f),
        mArrowHeight(0.2f),
        mShowOriginPoint(true),
        mUseLineSmooth(true),
        // цвета осей (X=красный, Y=зелёный, Z=синий), «чёрный» у начала
        mXColor{ 1.f, 0.f, 0.f },
        mYColor{ 0.f, 1.f, 0.f },
        mZColor{ 0.f, 0.f, 1.f },
        mZeroColor{ 0.f, 0.f, 0.f }
    {
    }

    // настройки
    void setLength(GLfloat L) { mLength = L; }
    void setLineWidth(GLfloat w) { mLineWidth = w; }
    void setArrow(GLfloat radius, GLfloat height) { mArrowRadius = radius; mArrowHeight = height; }
    void setShowOriginPoint(bool v) { mShowOriginPoint = v; }
    void setLineSmooth(bool v) { mUseLineSmooth = v; }

    void setXColor(Color3 c) { mXColor = c; }
    void setYColor(Color3 c) { mYColor = c; }
    void setZColor(Color3 c) { mZColor = c; }
    void setZeroColor(Color3 c) { mZeroColor = c; }

    // основной вызов
    void draw() const;

private:
    GLfloat mLength;
    GLfloat mLineWidth;
    GLfloat mArrowRadius;
    GLfloat mArrowHeight;
    bool    mShowOriginPoint;
    bool    mUseLineSmooth;

    Color3  mXColor, mYColor, mZColor, mZeroColor;

    // внутренние помощники
    static void setColor(const Color3& c) { glColor3f(c.r, c.g, c.b); }
    static void drawArrowAlongX(GLfloat x, GLfloat radius, GLfloat height);
    static void drawArrowAlongY(GLfloat y, GLfloat radius, GLfloat height);
    static void drawArrowAlongZ(GLfloat z, GLfloat radius, GLfloat height);
};
