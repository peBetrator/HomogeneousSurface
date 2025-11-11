#pragma once
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "GL/glaux.h"
#include <string>

class Background {
public:
    Background();
    ~Background();

    // Загружает BMP по пути относительно .exe (например, "background_lion.bmp" или "assets\\bg.bmp")
    bool loadFromExeDir(const char* relativePath);

    // Рисует картинку на фоне в текущем viewport (как у тебя)
    void draw() const;

    // Освободить текстуру вручную (необязательно)
    void release();

private:
    GLuint mTex;
    bool   mLoaded;

    // --- служебные ---
    static std::string exeDirA();
    static std::string joinPathA(const std::string& dir, const char* file);

    // Попробовать glaux, иначе — свой парсер BMP (24/32 BI_RGB)
    static AUX_RGBImageRec* loadBMPRobust(const char* filenameRelToExe);
};
