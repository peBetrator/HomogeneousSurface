#pragma once
#include "LineSegmentCone.hpp"

// Производный класс с пер-вершинными нормалями и опциональным каркасом.
class LineSegmentConeLit : public LineSegmentCone {
public:
    LineSegmentConeLit(int levels = 12, int segments = 32, float depth = 3.f)
        : LineSegmentCone(levels, segments, depth) {
    }

    ~LineSegmentConeLit() override { releaseNormals(); }

    // Перестроить геометрию и пересчитать нормали
    void build() override;

    // Рисовать с освещением; при mWireframe=true поверх — каркас
    void draw() const override;

    void setWireframe(bool on) { mWireframe = on; }

    // Доступ к нормалям (если нужно)
    const Vec3* normals() const { return mNormals; }
    size_t normalCount() const { return mNormalCount; }

private:
    // динамически выделяемые пер-вершинные нормали
    Vec3* mNormals = nullptr;  // размер = vertices().size()
    size_t mNormalCount = 0;
    bool   mWireframe = false;

    void releaseNormals();
    void computeVertexNormals(); // усреднение нормалей треугольников -> вершины
};
