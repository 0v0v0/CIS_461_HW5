#include "diffusearealight.h"

Color3f DiffuseAreaLight::L(const Intersection &isect, const Vector3f &w) const
{
    //TODO

    if(twoSided)
    {
        return emittedLight;
    }
    else if( glm::dot(isect.normalGeometric, w) > 0)
    {
        return emittedLight;
    }

    return glm::vec3(0);

}

Color3f DiffuseAreaLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                    Vector3f *wi, Float *pdf, Point3f *p) const
{
    //Generate intersection
    Intersection pShape = shape->Sample(ref, xi, pdf);

    *p=pShape.point;

    //pShape.mediumInterface = mediumInterface;

    if (*pdf == 0 || glm::length(pShape.point - ref.point) == 0)
    {
        *pdf = 0;
        return Color3f(0);
    }
    *wi = glm::normalize(pShape.point - ref.point);

    return L(pShape, -*wi);


}
