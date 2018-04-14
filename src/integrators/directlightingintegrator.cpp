#include "directlightingintegrator.h"

Color3f UniformSampleOneLight(Intersection &isect, const Scene &scene , std::shared_ptr<Sampler> sampler, const Ray &ray);
Color3f EstimateDirect(const Intersection &it, const Light &light, const Point2f &uLight, const Scene &scene, const Ray &ray);
Float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf) ;

Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Color3f L(0.f);

    // Find closest ray intersection or return background radiance
    Intersection isect;
    if (!scene.Intersect(ray, &isect))
    {
        for (const auto &light : scene.lights)
        {
            L += light->Le(ray);
        }
        return L;
    }

    //This is a light...perhaps
    if (!isect.ProduceBSDF())
    {
        //return Li(isect.SpawnRay(ray.direction), scene, sampler, depth);
        return isect.objectHit->areaLight->L(isect,ray.direction*-1.f);
    }
    else if (scene.lights.size() > 0)
    {
        // Compute direct lighting for _DirectLightingIntegrator_ integrator
        L += UniformSampleOneLight(isect, scene, sampler, ray);
    }

    return L;
}

Color3f UniformSampleOneLight(Intersection &isect, const Scene &scene , std::shared_ptr<Sampler> sampler, const Ray &ray)
{
    // Randomly choose a single light to sample, _light_
    int nLights = int(scene.lights.size());

    if (nLights == 0)
    {
        return Color3f(0.f);
    }

    int lightNum;

    Float lightPdf;

    //Which Light?
    lightNum = std::min((int)(sampler->Get1D() * nLights), nLights - 1);

    lightPdf = 1.f / nLights;

    const std::shared_ptr<Light> &light = scene.lights[lightNum];

    Point2f uLight = sampler->Get2D();
    //Point2f uScattering = sampler->Get2D();

    return EstimateDirect(isect, *light, uLight,
                          scene, ray ) / lightPdf;
}

Color3f EstimateDirect(const Intersection &isect, const Light &light, const Point2f &uLight,const Scene &scene, const Ray &ray)
{
    BxDFType bsdfFlags =BSDF_ALL;

    Color3f Ld(0.f);
    // Sample light source with multiple importance sampling
    Vector3f wi;

    Float lightPdf = 0, scatteringPdf = 0;

    //Sample Light -> Sample Shape -> Sample squareplane/disc
    glm::vec3 point;
    Color3f Li = light.Sample_Li(isect , uLight, &wi, &lightPdf, &point);

    //Visibility test
    bool visible = false;
    glm::vec3 dir = glm::normalize(point - isect.point);
    Intersection isect2;
    //from sample point to origin point on surface
    Ray ray2(point, dir);
    ray2.origin = point;
    ray2.direction = -dir;

    if(scene.Intersect(ray2 , &isect2))
    {
        if(isect2.ProduceBSDF())
        {
            float len= glm::length(isect2.point - isect.point);

            if(len<0.001f)
            {
                visible = true;
            }
        }
    }

    //Handle BSDF on surface
    if ( !IsBlack(Li))
    {
        // Compute BSDF or phase function's value for light sample
        Color3f f = Color3f(0);


        // Evaluate BSDF for light sampling strategy
        const Intersection tmp = (const Intersection)isect;
        f = tmp.bsdf->f(-ray.direction, wi, bsdfFlags)*AbsDot(wi, isect.normalGeometric);

        if(IsBlack(f))
        {
            f =tmp.bsdf->f(ray.direction, wi, bsdfFlags)*AbsDot(wi, isect.normalGeometric);
        }

        //scatteringPdf = isect.bsdf->Pdf(-ray.direction, wi, bsdfFlags);

        if (!IsBlack(f))
        {
            // Compute effect of visibility for light source sample
            if (!visible)
            {
                Li = Color3f(0.f);
            }
            // Add light's contribution to reflected radiance
            else
            {
                if(lightPdf !=0)
                {
                    //Float weight = PowerHeuristic(1, lightPdf, 1, scatteringPdf);
                    //Ld += f * Li * weight / lightPdf;

                    Ld += f*Li/lightPdf;
                }
            }
        }
    }

    return Ld;
}

Float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    Float f = nf * fPdf, g = ng * gPdf;
    return (f * f) / (f * f + g * g);
}








/*
Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    //TODO

    Color3f L(0.f);

    //Magic number here
    int nSamples = 1;

    //Hit nothing
    Intersection isect;

    bool ishit=scene.Intersect(ray, &isect);

    if(!ishit)
    {
        return Color3f(0.f);
    }

    //If it is not a light mesh
    if(isect.objectHit->areaLight == nullptr)
    {
        if (scene.lights.size() > 0)
        {
            isect.ProduceBSDF();

            //For each light
            for (int j = 0; j < scene.lights.size(); ++j)
            {
                // Accumulate contribution of _j_th light to _L_
                const std::shared_ptr<Light> &light = scene.lights[j];


                // Estimate direct lighting using sample arrays
                Color3f Ld(0.f);


                //For each sample in this light
                for (int k = 0; k < nSamples; ++k)
                {
                    glm::vec3 wiW, woW;
                    float Lpdf, Spdf=0;

                    Point2f p= sampler->Get2D();

                    Color3f Li= glm::vec3(0,0,0);

                    woW=-ray.direction;

                    Intersection isect2;

                    Point3f point = glm::vec3(3);



                    Li = light->Sample_Li(isect , p , &wiW , &Lpdf, &point);

                    //Visibility Test

                    glm::vec3 dir=glm::normalize(point - isect.point);

                    Ray ray2(point , dir);


                    ray2.origin=point+ 0.00001f*dir;
                    ray2.direction= dir;

                    if(scene.Intersect(ray2 , &isect2))
                    {
                        isect2.ProduceBSDF();
                        //Test visibility
                        if( (isect2.point - isect.point).length() > 0.00001f)
                        {
                            Li = Color3f(0,1,1);
                        }
                        else
                        {
                            Color3f f = isect.bsdf->f(woW, wiW) ;
                            f = f* AbsDot(wiW, isect.normalGeometric);

                            Spdf = isect.bsdf->Pdf(woW, wiW);

                            if(!IsBlack(f))
                            {
                                if(Spdf!=0)
                                {
                                    Li=Li*f/Spdf;
                                }
                            }
                            else
                            {
                                Li = Color3f(0,1,0);
                            }
                        }
                    }
                    else
                    {
                        Li = Color3f(1,0,0);
                    }

                    if(Lpdf!=0)
                    {
                        Ld+=Li/Lpdf;
                    }

                    Ld+=Li;

                    L += Ld * (1.0f / (float)nSamples);
                }
            }
            L = L*(1.f / (float)scene.lights.size());
        }
    }
    else
    {
        return isect.objectHit->areaLight->L(isect,ray.direction*-1.f);
    }

    return L;
}
*/


