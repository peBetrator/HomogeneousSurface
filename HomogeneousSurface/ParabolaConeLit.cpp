#include "ParabolaConeLit.hpp"
#include <cmath>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/gl.h>

// ——— векторная математика (минимум) ———
static inline Vec3 vsub(const Vec3& a, const Vec3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline Vec3 vadd(const Vec3& a, const Vec3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline Vec3 vcross(const Vec3& a, const Vec3& b) {
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
static inline float vlen(const Vec3& a) { return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z); }
static inline Vec3 vnorm(const Vec3& a) {
    float L = vlen(a);
    if (L > 1e-12f) return { a.x / L, a.y / L, a.z / L };
    return { 0.f, 0.f, 1.f };
}

// ——— реализация ParabolaConeLit ———
void ParabolaConeLit::releaseNormals() {
    if (mNormals) {
        delete[] mNormals;
        mNormals = nullptr;
        mNormalCount = 0;
    }
}

void ParabolaConeLit::computeVertexNormals() {
    releaseNormals();

    const auto& V = vertices();
    const auto& I = indices();

    mNormalCount = V.size();
    if (!mNormalCount) return;

    mNormals = new Vec3[mNormalCount];
    for (size_t i = 0; i < mNormalCount; ++i) mNormals[i] = { 0.f, 0.f, 0.f };

    // аккумулируем нормали треугольников (взвешены площадью, т.к. без нормализации)
    for (size_t k = 0; k + 2 < I.size(); k += 3) {
        unsigned ia = I[k + 0], ib = I[k + 1], ic = I[k + 2];
        const Vec3& A = V[ia];
        const Vec3& B = V[ib];
        const Vec3& C = V[ic];
        Vec3 n = vcross(vsub(B, A), vsub(C, A)); // ориентация важна
        mNormals[ia] = vadd(mNormals[ia], n);
        mNormals[ib] = vadd(mNormals[ib], n);
        mNormals[ic] = vadd(mNormals[ic], n);
    }

    // нормализуем пер-вершинные нормали
    for (size_t i = 0; i < mNormalCount; ++i) mNormals[i] = vnorm(mNormals[i]);
}

void ParabolaConeLit::build() {
    // строим геометрию базового класса (твоя double-cone логика уже внутри)
    ParabolaCone::build();
    // считаем нормали
    computeVertexNormals();
}

void ParabolaConeLit::draw() const {
    const auto& V = vertices();
    const auto& I = indices();

    // ---- сохранить состояния ----
    GLboolean wasLighting = glIsEnabled(GL_LIGHTING);
    GLboolean wasNormalize = glIsEnabled(GL_NORMALIZE);
    GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
    GLboolean wasColorMat = glIsEnabled(GL_COLOR_MATERIAL);
    GLboolean wasTex2D = glIsEnabled(GL_TEXTURE_2D);
    GLint     twoSideBefore = 0; glGetIntegerv(GL_LIGHT_MODEL_TWO_SIDE, &twoSideBefore);

    // ---- корректные состояния для заливки ----
    if (!wasLighting)  glEnable(GL_LIGHTING);
    if (!wasNormalize) glEnable(GL_NORMALIZE);
    if (wasCull)       glDisable(GL_CULL_FACE);
    if (wasColorMat)   glDisable(GL_COLOR_MATERIAL); // чтобы glColor не сбивал материал
    if (wasTex2D)      glDisable(GL_TEXTURE_2D);     // вдруг текстура фона осталась
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // Гарантируем FILL перед заливкой (на случай, если где-то остался GL_LINE)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Прозрачность (можно начать с 1.0, чтобы проверить, видно ли заливку)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ---- ДВУСТОРОННИЙ МАТЕРИАЛ: FRONT=фиолетовый, BACK=зелёный ----
    // FRONT (фиолетовый)
    const GLfloat frontAmb[4] = { 0.10f, 0.06f, 0.14f, 0.85f };
    const GLfloat frontDif[4] = { 0.60f, 0.25f, 0.80f, 0.85f };
    const GLfloat frontSpc[4] = { 0.30f, 0.25f, 0.35f, 0.85f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, frontAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, frontDif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, frontSpc);
    glMaterialf(GL_FRONT, GL_SHININESS, 32.f);

    // BACK (зелёный)
    const GLfloat backAmb[4] = { 0.06f, 0.10f, 0.06f, 0.85f };
    const GLfloat backDif[4] = { 0.25f, 0.80f, 0.35f, 0.85f };
    const GLfloat backSpc[4] = { 0.20f, 0.30f, 0.20f, 0.85f };
    glMaterialfv(GL_BACK, GL_AMBIENT, backAmb);
    glMaterialfv(GL_BACK, GL_DIFFUSE, backDif);
    glMaterialfv(GL_BACK, GL_SPECULAR, backSpc);
    glMaterialf(GL_BACK, GL_SHININESS, 18.f);

    // ---- ЗАЛИВКА ПОЛИГОНАМИ С НОРМАЛЯМИ ----
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

    // ---- КАРКАС (опционально) ----
    if (mWireframe) {
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.f, -1.f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glLineWidth(1.0f);

        // Включим ColorMaterial ТОЛЬКО для каркаса, либо
        // можно оставить материал тот же; здесь покажу через цвет:
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glColor4f(0.05f, 0.05f, 0.05f, 1.0f);

        glBegin(GL_TRIANGLES);
        for (size_t k = 0; k + 2 < I.size(); k += 3) {
            unsigned ia = I[k + 0], ib = I[k + 1], ic = I[k + 2];
            glVertex3f(V[ia].x, V[ia].y, V[ia].z);
            glVertex3f(V[ib].x, V[ib].y, V[ib].z);
            glVertex3f(V[ic].x, V[ic].y, V[ic].z);
        }
        glEnd();

        glDisable(GL_COLOR_MATERIAL); // вернули как было
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
    }

    // ---- восстановить состояния ----
    if (wasTex2D)      glEnable(GL_TEXTURE_2D);
    if (wasColorMat)   glEnable(GL_COLOR_MATERIAL);
    if (!twoSideBefore) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    if (wasCull)       glEnable(GL_CULL_FACE);
    if (!wasNormalize) glDisable(GL_NORMALIZE);
    if (!wasLighting)  glDisable(GL_LIGHTING);
}
