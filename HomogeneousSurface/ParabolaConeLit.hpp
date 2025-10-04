#pragma once
#include "ParabolaCone.hpp"

// Производный класс: считает и хранит пер-вершинные нормали динамически,
// рисует с освещением; каркас включается флагом.
class ParabolaConeLit : public ParabolaCone {
public:
    ParabolaConeLit(int levels = 12, int segments = 32, float depth = 3.f)
        : ParabolaCone(levels, segments, depth) {
    }

    ~ParabolaConeLit() override { releaseNormals(); }

    // Перестроить геометрию и пересчитать нормали
    void build() override;

    // Рисовать с освещением; при mWireframe=true поверх — каркас
    void draw() const override;

    void setWireframe(bool on) { mWireframe = on; }

    const Vec3* normals() const { return mNormals; }
    size_t normalCount() const { return mNormalCount; }

private:
    // динамически выделяемые пер-вершинные нормали
    Vec3* mNormals = nullptr;   // размер = vertices().size()
    size_t mNormalCount = 0;
    bool   mWireframe = false;

    void releaseNormals();
    void computeVertexNormals(); // усреднение нормалей треугольников -> вершины
};
