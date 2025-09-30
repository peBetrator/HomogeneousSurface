#pragma once
#include <vector>

struct Vec3 { float x, y, z; };

class ISurface3D {
public:
    virtual ~ISurface3D() = default;

    // перестроить геометрию с текущими параметрами
    virtual void build() = 0;

    // нарисовать (immediate mode, как в проекте)
    virtual void draw() const = 0;

    // доступ к вершинам/индексам (для будущей анимации)
    virtual const std::vector<Vec3>& vertices() const = 0;
    virtual const std::vector<unsigned>& indices()  const = 0;
};