#include "LineSegmentConeLit.hpp"
#include <algorithm>
#include <cmath>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/gl.h>
#include "GL/glaux.h"
#pragma comment(lib,"Glaux.lib")

#include <vector>
#include <string>
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")


static std::string exeDirA() {
    char buf[MAX_PATH] = { 0 };
    DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return std::string(".");
    PathRemoveFileSpecA(buf);
    return std::string(buf);
}
static std::string joinPathA(const std::string& dir, const char* file) {
    char full[MAX_PATH] = { 0 };
    lstrcpynA(full, dir.c_str(), MAX_PATH);
    PathAppendA(full, file);
    return std::string(full);
}

// Попытка 1: glaux; Попытка 2: свой парсер 24/32-bit BI_RGB (с разворотом Y, BGR->RGB)
static AUX_RGBImageRec* LoadBMP_Robust(const char* filenameRelToExe) {
    if (!filenameRelToExe) return nullptr;
    std::string full = joinPathA(exeDirA(), filenameRelToExe);

    // 1) glaux
    {
        FILE* f = nullptr;
        if (fopen_s(&f, full.c_str(), "rb") == 0 && f) {
            fclose(f);
            if (AUX_RGBImageRec* img = auxDIBImageLoadA(full.c_str()))
                return img;
        }
    }

    // 2) свой
    FILE* fp = nullptr;
    if (fopen_s(&fp, full.c_str(), "rb") != 0 || !fp) return nullptr;

    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    if (fread(&bfh, sizeof(bfh), 1, fp) != 1) { fclose(fp); return nullptr; }
    if (bfh.bfType != 0x4D42) { fclose(fp); return nullptr; } // 'BM'
    if (fread(&bih, sizeof(bih), 1, fp) != 1) { fclose(fp); return nullptr; }
    if (bih.biCompression != BI_RGB || (bih.biBitCount != 24 && bih.biBitCount != 32)) {
        fclose(fp); return nullptr;
    }
    if (bih.biWidth <= 0 || bih.biHeight == 0) { fclose(fp); return nullptr; }

    int width = bih.biWidth;
    int height = std::abs(bih.biHeight);
    bool flipY = (bih.biHeight > 0); // BMP «вверх ногами», если положительный

    fseek(fp, bfh.bfOffBits, SEEK_SET);

    int bpp = bih.biBitCount / 8;
    int rowRaw = width * bpp;
    int rowStride = ((rowRaw + 3) / 4) * 4;

    std::vector<unsigned char> raw(rowStride * height);
    if (fread(raw.data(), 1, raw.size(), fp) != raw.size()) { fclose(fp); return nullptr; }
    fclose(fp);

    // Конвертируем BGR(A)->RGB, при необходимости переворачиваем
    unsigned char* rgb = (unsigned char*)malloc(width * height * 3);
    if (!rgb) return nullptr;

    auto copyLine = [&](int srcRow, int dstRow) {
        const unsigned char* src = raw.data() + srcRow * rowStride;
        unsigned char* dst = rgb + dstRow * width * 3;
        for (int x = 0;x < width;++x) {
            unsigned char B = src[x * bpp + 0];
            unsigned char G = src[x * bpp + 1];
            unsigned char R = src[x * bpp + 2];
            dst[x * 3 + 0] = R; dst[x * 3 + 1] = G; dst[x * 3 + 2] = B;
        }
        };
    if (flipY) for (int y = 0;y < height;++y) copyLine(y, height - 1 - y);
    else       for (int y = 0;y < height;++y) copyLine(y, y);

    AUX_RGBImageRec* img = (AUX_RGBImageRec*)malloc(sizeof(AUX_RGBImageRec));
    img->sizeX = width; img->sizeY = height; img->data = rgb;
    return img;
}


// ===== векторная математика (коротко) =====
static inline Vec3 vsub(const Vec3& a, const Vec3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline Vec3 vadd(const Vec3& a, const Vec3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline Vec3 vcross(const Vec3& a, const Vec3& b) {
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
static inline float vlen(const Vec3& a) { return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z); }
static inline Vec3 vnorm(const Vec3& a) { float L = vlen(a); return (L > 1e-12f) ? Vec3{ a.x / L,a.y / L,a.z / L } : Vec3{ 0,0,1 }; }

// ===== нормали =====
void LineSegmentConeLit::releaseNormals() {
    if (mNormals) { delete[] mNormals; mNormals = nullptr; mNormalCount = 0; }
}
void LineSegmentConeLit::computeVertexNormals() {
    releaseNormals();
    const auto& V = vertices();
    const auto& I = indices();
    mNormalCount = V.size();
    if (!mNormalCount) return;
    mNormals = new Vec3[mNormalCount];
    for (size_t i = 0;i < mNormalCount;++i) mNormals[i] = { 0,0,0 };

    for (size_t k = 0;k + 2 < I.size();k += 3) {
        unsigned ia = I[k], ib = I[k + 1], ic = I[k + 2];
        Vec3 n = vcross(vsub(V[ib], V[ia]), vsub(V[ic], V[ia])); // area-weighted
        mNormals[ia] = vadd(mNormals[ia], n);
        mNormals[ib] = vadd(mNormals[ib], n);
        mNormals[ic] = vadd(mNormals[ic], n);
    }
    for (size_t i = 0;i < mNormalCount;++i) mNormals[i] = vnorm(mNormals[i]);
}

// ===== UV =====
void LineSegmentConeLit::releaseUVs() {
    if (mUVs) { delete[] mUVs; mUVs = nullptr; mUVCount = 0; }
}
void LineSegmentConeLit::buildUVs() {
    releaseUVs();
    const auto& V = vertices();
    const int L = std::max(1, levels());
    const int S = std::max(1, segments());

    mUVCount = V.size();
    if (!mUVCount) return;
    mUVs = new Vec2[mUVCount];

    // 0 — апекс
    if (mUVCount > 0)
        mUVs[0] = { 0.5f, 0.5f };

    size_t idx = 1;
    // низ
    for (int l = -L; l <= -1; ++l) {
        float v = float(l + L) / float(2 * L);
        for (int i = 0; i <= S; ++i) {
            float u = float(i) / float(S);
            if (idx < mUVCount) mUVs[idx++] = { u, v };
        }
    }
    // верх
    for (int l = 1; l <= L; ++l) {
        float v = float(l + L - 1) / float(2 * L);
        for (int i = 0; i <= S; ++i) {
            float u = float(i) / float(S);
            if (idx < mUVCount) mUVs[idx++] = { u, v };
        }
    }
}

// ===== текстуры =====
bool LineSegmentConeLit::createTextureFromBMP(const char* filenameRelToExe, unsigned& texOut) {
    AUX_RGBImageRec* img = LoadBMP_Robust(filenameRelToExe);
    if (!img) {
        MessageBoxA(nullptr, (std::string("Can't load BMP: ") + filenameRelToExe).c_str(),
            "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    if (!texOut) glGenTextures(1, &texOut);
    glBindTexture(GL_TEXTURE_2D, texOut);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, img->sizeX, img->sizeY,
        GL_RGB, GL_UNSIGNED_BYTE, img->data);

    if (img->data) free(img->data);
    free(img);
    return true;
}
void LineSegmentConeLit::releaseTextures() {
    if (mTexFront) { glDeleteTextures(1, &mTexFront); mTexFront = 0; }
    if (mTexBack) { glDeleteTextures(1, &mTexBack); mTexBack = 0; }
}
bool LineSegmentConeLit::loadTextures(const char* frontBmp, const char* backBmp) {
    releaseTextures();
    bool okF = frontBmp ? createTextureFromBMP(frontBmp, mTexFront) : false;
    bool okB = backBmp ? createTextureFromBMP(backBmp, mTexBack) : false;
    return okF && okB;
}

// ===== build/draw =====
void LineSegmentConeLit::build() {
    // 1) гео из базового класса (double-cone)
    LineSegmentCone::build();
    // 2) нормали
    computeVertexNormals();
    // 3) UV
    buildUVs();
}

void LineSegmentConeLit::draw() const {
    const auto& V = vertices();
    const auto& I = indices();

    // ---- сохранить состояние ----
    GLboolean wasLighting = glIsEnabled(GL_LIGHTING);
    GLboolean wasNormalize = glIsEnabled(GL_NORMALIZE);
    GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
    GLboolean wasTex2D = glIsEnabled(GL_TEXTURE_2D);
    GLboolean wasColorMat = glIsEnabled(GL_COLOR_MATERIAL);

    //if (wasLighting)  glDisable(GL_LIGHTING);
    if (!wasTex2D)    glEnable(GL_TEXTURE_2D);
    if (!wasCull)     glEnable(GL_CULL_FACE);
    if (wasColorMat)  glDisable(GL_COLOR_MATERIAL);

    // гарантируем заливку
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // режим наложения текстуры «как есть»
    GLint oldEnv = 0;
    glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &oldEnv);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glColor4f(1, 1, 1, 1); // чтобы ничто не умножало текстуру

    // ----- PASS 1: FRONT (наружные) -----
    if (mTexFront) glBindTexture(GL_TEXTURE_2D, mTexFront);
    glCullFace(GL_BACK);
    glBegin(GL_TRIANGLES);
    for (size_t k = 0; k + 2 < I.size(); k += 3) {
        unsigned ia = I[k], ib = I[k + 1], ic = I[k + 2];
        if (mUVs) glTexCoord2f(mUVs[ia].u, mUVs[ia].v);
        glVertex3f(V[ia].x, V[ia].y, V[ia].z);
        if (mUVs) glTexCoord2f(mUVs[ib].u, mUVs[ib].v);
        glVertex3f(V[ib].x, V[ib].y, V[ib].z);
        if (mUVs) glTexCoord2f(mUVs[ic].u, mUVs[ic].v);
        glVertex3f(V[ic].x, V[ic].y, V[ic].z);
    }
    glEnd();

    // ----- PASS 2: BACK (внутренние) -----
    if (mTexBack) glBindTexture(GL_TEXTURE_2D, mTexBack);
    glCullFace(GL_FRONT);
    glBegin(GL_TRIANGLES);
    for (size_t k = 0; k + 2 < I.size(); k += 3) {
        unsigned ia = I[k], ib = I[k + 1], ic = I[k + 2];
        if (mUVs) glTexCoord2f(mUVs[ia].u, mUVs[ia].v);
        glVertex3f(V[ia].x, V[ia].y, V[ia].z);
        if (mUVs) glTexCoord2f(mUVs[ib].u, mUVs[ib].v);
        glVertex3f(V[ib].x, V[ib].y, V[ib].z);
        if (mUVs) glTexCoord2f(mUVs[ic].u, mUVs[ic].v);
        glVertex3f(V[ic].x, V[ic].y, V[ic].z);
    }
    glEnd();

    // ----- опциональный каркас поверх -----
    if (mWireframe) {
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.f, -1.f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f);
        glColor4f(0.05f, 0.05f, 0.05f, 1.0f);

        glBegin(GL_TRIANGLES);
        for (size_t k = 0; k + 2 < I.size(); k += 3) {
            unsigned ia = I[k], ib = I[k + 1], ic = I[k + 2];
            glVertex3f(V[ia].x, V[ia].y, V[ia].z);
            glVertex3f(V[ib].x, V[ib].y, V[ib].z);
            glVertex3f(V[ic].x, V[ic].y, V[ic].z);
        }
        glEnd();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
        if (wasTex2D) glEnable(GL_TEXTURE_2D);
    }

    // ----- восстановить состояние -----
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, oldEnv);
    if (wasColorMat) glEnable(GL_COLOR_MATERIAL);
    if (!wasCull)    glDisable(GL_CULL_FACE);
    if (!wasTex2D)   glDisable(GL_TEXTURE_2D);
    if (!wasNormalize) glDisable(GL_NORMALIZE);
    if (wasLighting) glEnable(GL_LIGHTING);
}

