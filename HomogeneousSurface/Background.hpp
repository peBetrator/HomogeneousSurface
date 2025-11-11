#pragma once
#include <string>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// Simple replacement for AUX_RGBImageRec
struct RGBImageRec {
    int sizeX;
    int sizeY;
    unsigned char* data;
};

class Background {
public:
    Background();
    ~Background();

    // Загружает BMP по пути относительно текущей директории (например, "assets/background_lion.bmp")
    bool loadFromPath(const char* relativePath);

    // Для обратной совместимости
    bool loadFromExeDir(const char* relativePath) { return loadFromPath(relativePath); }

    // Рисует картинку на фоне в текущем viewport (как у тебя)
    void draw() const;

    // Освободить текстуру вручную (необязательно)
    void release();

private:
    GLuint mTex;
    bool   mLoaded;

    // Загрузить BMP файл (24/32-bit BI_RGB)
    static RGBImageRec* loadBMP(const char* filename);
};
