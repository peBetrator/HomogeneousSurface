#pragma once
#include "CustomRegionCone.hpp"

class CustomRegionConeLit : public CustomRegionCone {
public:
    CustomRegionConeLit(int levels = 12, int segmentsPerSide = 16, float depth = 3.f)
        : CustomRegionCone(levels, segmentsPerSide, depth) {
    }

    ~CustomRegionConeLit() override { releaseNormals(); }

    void build() override;
    void draw() const override;

    void setWireframe(bool on) { mWireframe = on; }

    const struct Vec3* normals() const { return mNormals; }
    size_t normalCount() const { return mNormalCount; }

private:
    Vec3* mNormals = nullptr;
    size_t mNormalCount = 0;
    bool   mWireframe = false;

    void releaseNormals();
    void computeVertexNormals();
};
