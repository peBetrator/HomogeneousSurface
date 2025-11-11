#include "Background.hpp"
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <stdint.h>
// BMP file structures for non-Windows platforms
#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

#define BI_RGB 0
#endif

// -------- robust BMP loader (24/32-bit BI_RGB) --------
RGBImageRec* Background::loadBMP(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) return nullptr;

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

    auto* img = (RGBImageRec*)malloc(sizeof(RGBImageRec));
    img->sizeX = width;
    img->sizeY = height;
    img->data = rgb;
    return img;
}

// -------- public --------
Background::Background() : mTex(0), mLoaded(false) {}
Background::~Background() { release(); }

bool Background::loadFromPath(const char* relativePath) {
    release();

    RGBImageRec* img = loadBMP(relativePath);
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
