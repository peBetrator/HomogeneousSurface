#include "CustomRegionConeLit.hpp"
#include <cmath>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

// --- векторные утилиты ---
static inline Vec3 vsub(const Vec3& a, const Vec3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline Vec3 vcross(const Vec3& a, const Vec3& b) {
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
static inline float vlen(const Vec3& a) { return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z); }
static inline Vec3 vnorm(const Vec3& a) {
    float L = vlen(a);
    if (L > 1e-12f) return { a.x / L, a.y / L, a.z / L };
    return { 0.f, 0.f, 1.f };
}
static inline Vec3 vadd(const Vec3& a, const Vec3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }

void CustomRegionConeLit::releaseNormals() {
    if (mNormals) {
        delete[] mNormals;
        mNormals = nullptr;
        mNormalCount = 0;
    }
}

void CustomRegionConeLit::computeVertexNormals() {
    releaseNormals();
    const auto& V = vertices();
    const auto& I = indices();

    mNormalCount = V.size();
    if (!mNormalCount) return;

    mNormals = new Vec3[mNormalCount];
    for (size_t i = 0; i < mNormalCount; ++i) mNormals[i] = { 0.f, 0.f, 0.f };

    for (size_t k = 0; k + 2 < I.size(); k += 3) {
        unsigned ia = I[k + 0], ib = I[k + 1], ic = I[k + 2];
        const Vec3& A = V[ia];
        const Vec3& B = V[ib];
        const Vec3& C = V[ic];

        Vec3 n = vcross(vsub(B, A), vsub(C, A));
        mNormals[ia] = vadd(mNormals[ia], n);
        mNormals[ib] = vadd(mNormals[ib], n);
        mNormals[ic] = vadd(mNormals[ic], n);
    }

    for (size_t i = 0; i < mNormalCount; ++i) {
        mNormals[i] = vnorm(mNormals[i]);
    }
}

void CustomRegionConeLit::build() {
    CustomRegionCone::build();
    computeVertexNormals();
}

void CustomRegionConeLit::draw() const {
    const auto& V = vertices();
    const auto& I = indices();

    GLboolean wasLighting = glIsEnabled(GL_LIGHTING);
    GLboolean wasNormalize = glIsEnabled(GL_NORMALIZE);
    GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
    GLboolean wasTex2D = glIsEnabled(GL_TEXTURE_2D);
    GLboolean wasColorMat = glIsEnabled(GL_COLOR_MATERIAL);
    GLint     twoSideBefore = 0;
    glGetIntegerv(GL_LIGHT_MODEL_TWO_SIDE, &twoSideBefore);

    if (!wasLighting)  glEnable(GL_LIGHTING);
    if (!wasNormalize) glEnable(GL_NORMALIZE);
    if (wasCull)       glDisable(GL_CULL_FACE);
    if (wasTex2D)      glDisable(GL_TEXTURE_2D);
    if (wasColorMat)   glDisable(GL_COLOR_MATERIAL);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Материал для фигуры: золотистый спереди, зелёный сзади
    const GLfloat frontAmb[4] = { 0.15f, 0.12f, 0.05f, 1.0f };
    const GLfloat frontDif[4] = { 0.95f, 0.75f, 0.25f, 0.85f }; // золотой
    const GLfloat frontSpc[4] = { 0.50f, 0.40f, 0.20f, 0.85f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, frontAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, frontDif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, frontSpc);
    glMaterialf(GL_FRONT, GL_SHININESS, 40.f);

    const GLfloat backAmb[4] = { 0.05f, 0.15f, 0.10f, 1.0f };
    const GLfloat backDif[4] = { 0.25f, 0.85f, 0.45f, 0.85f }; // зелёный
    const GLfloat backSpc[4] = { 0.20f, 0.40f, 0.30f, 0.85f };
    glMaterialfv(GL_BACK, GL_AMBIENT, backAmb);
    glMaterialfv(GL_BACK, GL_DIFFUSE, backDif);
    glMaterialfv(GL_BACK, GL_SPECULAR, backSpc);
    glMaterialf(GL_BACK, GL_SHININESS, 20.f);

    // Заливка
    glBegin(GL_TRIANGLES);
    for (size_t k = 0; k + 2 < I.size(); k += 3) {
        unsigned ia = I[k + 0], ib = I[k + 1], ic = I[k + 2];

        if (mNormals) glNormal3f(mNormals[ia].x, mNormals[ia].y, mNormals[ia].z);
        glVertex3f(V[ia].x, V[ia].y, V[ia].z);

        if (mNormals) glNormal3f(mNormals[ib].x, mNormals[ib].y, mNormals[ib].z);
        glVertex3f(V[ib].x, V[ib].y, V[ib].z);

        if (mNormals) glNormal3f(mNormals[ic].x, mNormals[ic].y, mNormals[ic].z);
        glVertex3f(V[ic].x, V[ic].y, V[ic].z);
    }
    glEnd();

    // Каркас
    if (mWireframe) {
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.f, -1.f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glLineWidth(1.5f);
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

        glBegin(GL_TRIANGLES);
        for (size_t k = 0; k + 2 < I.size(); k += 3) {
            unsigned ia = I[k + 0], ib = I[k + 1], ic = I[k + 2];
            glVertex3f(V[ia].x, V[ia].y, V[ia].z);
            glVertex3f(V[ib].x, V[ib].y, V[ib].z);
            glVertex3f(V[ic].x, V[ic].y, V[ic].z);
        }
        glEnd();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
    }

    // Восстановить состояния
    if (!twoSideBefore) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    if (wasTex2D)      glEnable(GL_TEXTURE_2D);
    if (wasColorMat)   glEnable(GL_COLOR_MATERIAL);
    if (wasCull)       glEnable(GL_CULL_FACE);
    if (!wasNormalize) glDisable(GL_NORMALIZE);
    if (!wasLighting)  glDisable(GL_LIGHTING);
}
