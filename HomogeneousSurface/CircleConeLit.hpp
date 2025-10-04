#pragma once
#include "CircleCone.hpp"

class CircleConeLit : public CircleCone {
public:
    CircleConeLit(int levels = 12, int segments = 32, float depth = 3.f)
        : CircleCone(levels, segments, depth) {
    }

    ~CircleConeLit() override { releaseNormals(); }

    // Перестраиваем геометрию и пересчитываем нормали
    void build() override;

    // Рисуем с освещением. При mWireframe=true поверх рисуется каркас.
    void draw() const override;

    void setWireframe(bool on) { mWireframe = on; }

    // Доступ (если понадобится)
    const struct Vec3* normals() const { return mNormals; }
    size_t normalCount() const { return mNormalCount; }

private:
    // динамические данные нормалей
    Vec3* mNormals = nullptr; // размер = mVerts.size()
    size_t mNormalCount = 0;
    bool   mWireframe = false;

    void releaseNormals();
    void computeVertexNormals(); // усреднение нормалей треугольников -> вершины
};
