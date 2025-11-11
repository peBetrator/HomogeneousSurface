#pragma once
#include "AstroidCone.hpp"

class AstroidConeLit : public AstroidCone {
public:
    AstroidConeLit(int levels = 12, int segments = 64, float depth = 3.f)
        : AstroidCone(levels, segments, depth) {
    }

    ~AstroidConeLit() override { releaseNormals(); }

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
    Vec3* mNormals = nullptr; // размер = vertices().size()
    size_t mNormalCount = 0;
    bool   mWireframe = false;

    void releaseNormals();
    void computeVertexNormals(); // усреднение нормалей треугольников -> вершины
};
