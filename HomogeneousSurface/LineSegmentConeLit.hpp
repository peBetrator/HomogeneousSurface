#pragma once
#include "LineSegmentCone.hpp"

class LineSegmentConeLit : public LineSegmentCone {
public:
    LineSegmentConeLit(int levels = 12, int segments = 32, float depth = 3.f)
        : LineSegmentCone(levels, segments, depth) {
    }
    ~LineSegmentConeLit() override { releaseNormals(); releaseUVs(); releaseTextures(); }

    // Перестроить геометрию и пересчитать нормали
    void build() override;

    // Рисовать с освещением; при mWireframe=true поверх — каркас
    void draw() const override;

    void setWireframe(bool on) { mWireframe = on; }

    // Загрузка двух BMP (пути относительно рабочего каталога exe)
    bool loadTextures(const char* frontBmp, const char* backBmp);

private:
    struct Vec2 { float u, v; };

    // --- нормали (как раньше) ---
    void releaseNormals();
    void computeVertexNormals();
    mutable Vec3* mNormals = nullptr;
    mutable size_t mNormalCount = 0;

    // --- текстуры ---
    bool   createTextureFromBMP(const char* filename, unsigned& texOut);
    void   releaseTextures();
    unsigned mTexFront = 0;
    unsigned mTexBack = 0;

    // --- UV координаты ---
    void releaseUVs();
    void buildUVs();                  // параллельно mVerts
    Vec2* mUVs = nullptr;
    size_t mUVCount = 0;

    bool   mWireframe = false;
};
