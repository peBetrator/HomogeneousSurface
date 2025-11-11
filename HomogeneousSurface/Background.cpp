#include "Background.hpp"
#include <shlwapi.h>
#include <vector>
#include <cmath>
#pragma comment(lib,"Shlwapi.lib")

static AUX_RGBImageRec* tryGlauxLoad(const char* fullPath) {
    FILE* f = nullptr;
    if (fopen_s(&f, fullPath, "rb") != 0 || !f) return nullptr;
    fclose(f);
    return auxDIBImageLoadA(fullPath);
}

// -------- path helpers --------
std::string Background::exeDirA() {
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    buf[(n < MAX_PATH) ? n : (MAX_PATH - 1)] = '\0';
    PathRemoveFileSpecA(buf);
    return std::string(buf);
}

std::string Background::joinPathA(const std::string& dir, const char* file) {
    char full[MAX_PATH];
    lstrcpynA(full, dir.c_str(), MAX_PATH);
    PathAppendA(full, file);
    return std::string(full);
}

// -------- robust BMP loader (24/32-bit BI_RGB) --------
AUX_RGBImageRec* Background::loadBMPRobust(const char* filenameRelToExe) {
    std::string full = joinPathA(exeDirA(), filenameRelToExe);

    if (auto* img = tryGlauxLoad(full.c_str())) return img;

    FILE* fp = nullptr;
    if (fopen_s(&fp, full.c_str(), "rb") != 0 || !fp) return nullptr;

    BITMAPFILEHEADER bfh{};
    BITMAPINFOHEADER bih{};

    if (fread(&bfh, sizeof(bfh), 1, fp) != 1) { fclose(fp); return nullptr; }
    if (bfh.bfType != 0x4D42) { fclose(fp); return nullptr; } // 'BM'
    if (fread(&bih, sizeof(bih), 1, fp) != 1) { fclose(fp); return nullptr; }
    if (bih.biCompression != BI_RGB || (bih.biBitCount != 24 && bih.biBitCount != 32)) { fclose(fp); return nullptr; }
    if (bih.biWidth <= 0 || bih.biHeight == 0) { fclose(fp); return nullptr; }

    int width = bih.biWidth;
    int height = std::abs(bih.biHeight);
    bool flipY = (bih.biHeight > 0);

    fseek(fp, bfh.bfOffBits, SEEK_SET);

    const int bpp = bih.biBitCount / 8;   // 3 или 4
    const int rowRaw = width * bpp;
    const int rowStride = ((rowRaw + 3) / 4) * 4;

    std::vector<unsigned char> raw(rowStride * height);
    if (fread(raw.data(), 1, raw.size(), fp) != raw.size()) { fclose(fp); return nullptr; }
    fclose(fp);

    unsigned char* rgb = (unsigned char*)malloc(width * height * 3);
    if (!rgb) return nullptr;

    auto copyRow = [&](int srcRow, int dstRow) {
        const unsigned char* src = raw.data() + srcRow * rowStride;
        unsigned char* dst = rgb + dstRow * width * 3;
        for (int x = 0; x < width; ++x) {
            unsigned char B = src[x * bpp + 0];
            unsigned char G = src[x * bpp + 1];
            unsigned char R = src[x * bpp + 2];
            dst[x * 3 + 0] = R;
            dst[x * 3 + 1] = G;
            dst[x * 3 + 2] = B;
        }
        };

    if (flipY) {
        for (int y = 0; y < height; ++y) copyRow(y, height - 1 - y);
    }
    else {
        for (int y = 0; y < height; ++y) copyRow(y, y);
    }

    auto* img = (AUX_RGBImageRec*)malloc(sizeof(AUX_RGBImageRec));
    img->sizeX = width;
    img->sizeY = height;
    img->data = rgb;
    return img;
}

// -------- public --------
Background::Background() : mTex(0), mLoaded(false) {}
Background::~Background() { release(); }

bool Background::loadFromExeDir(const char* relativePath) {
    release();

    AUX_RGBImageRec* img = loadBMPRobust(relativePath);
    if (!img) return false;

    if (!mTex) glGenTextures(1, &mTex);
    glBindTexture(GL_TEXTURE_2D, mTex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // фильтры + обёртка
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // создаём мипмапы — надёжно работает с NPOT (512x341 и т.п.)
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB,
        img->sizeX, img->sizeY,
        GL_RGB, GL_UNSIGNED_BYTE, img->data);

    if (img->data) free(img->data);
    free(img);

    mLoaded = true;
    return true;
}

void Background::draw() const {
    if (!mLoaded || !mTex) return;

    // сохранить состояния
    GLboolean wasDepth = glIsEnabled(GL_DEPTH_TEST);
    GLboolean wasLight = glIsEnabled(GL_LIGHTING);
    GLboolean wasAlpha = glIsEnabled(GL_ALPHA_TEST);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mTex);
    glColor4f(1.f, 1.f, 1.f, 1.f);

    // рисуем во весь текущий viewport
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    // инвертируем V (BMP «вверхом вверх»)
    glBegin(GL_QUADS);
    glTexCoord2f(0.f, 1.f); glVertex2f(0.f, 0.f);
    glTexCoord2f(1.f, 1.f); glVertex2f(1.f, 0.f);
    glTexCoord2f(1.f, 0.f); glVertex2f(1.f, 1.f);
    glTexCoord2f(0.f, 0.f); glVertex2f(0.f, 1.f);
    glEnd();

    glPopMatrix(); // Projection
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    // откат состояний
    if (wasAlpha) glEnable(GL_ALPHA_TEST);
    if (wasLight) glEnable(GL_LIGHTING);
    if (wasDepth) glEnable(GL_DEPTH_TEST);
}

void Background::release() {
    if (mTex) {
        glDeleteTextures(1, &mTex);
        mTex = 0;
    }
    mLoaded = false;
}
