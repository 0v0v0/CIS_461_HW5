#pragma once
#include "material.h"

class MirrorMaterial : public Material
{
public:
    MirrorMaterial(const Color3f &Kr,
                  const std::shared_ptr<QImage> &textureMap,
                  const std::shared_ptr<QImage> &normalMap)
        : Kr(Kr), textureMap(textureMap), normalMap(normalMap)
    {}

    void ProduceBSDF(Intersection *isect) const;


private:
    Color3f Kr;                    // The spectral reflection of this material.
                                   // This is just the base color of the material

    std::shared_ptr<QImage> textureMap; // The color obtained from this (assuming it is non-null) is multiplied with the base material color.
    std::shared_ptr<QImage> normalMap;
};
