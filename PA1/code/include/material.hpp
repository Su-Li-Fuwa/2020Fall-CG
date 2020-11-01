#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "ray.hpp"
#include "hit.hpp"
#include <iostream>

// TODO: Implement Shade function that computes Phong introduced in class.
class Material {
public:

    explicit Material(const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0) :
            diffuseColor(d_color), specularColor(s_color), shininess(s) {

    }

    virtual ~Material() = default;

    virtual Vector3f getDiffuseColor() const {
        return diffuseColor;
    }


    Vector3f Shade(const Ray &ray, const Hit &hit,
                   const Vector3f &dirToLight, const Vector3f &lightColor) {
        // Normalized
        Vector3f hitNormed = hit.getNormal().normalized(), dirToLighted = dirToLight.normalized(), rayed = ray.getDirection().normalized();
        Vector3f Ri = 2*Vector3f::dot(hitNormed, dirToLighted)*hitNormed - dirToLighted;
        Ri.normalize();
        Vector3f diffuse = std::max(Vector3f::dot(dirToLighted, hitNormed), float(0))*diffuseColor;
        Vector3f specular = pow(std::max(Vector3f::dot(-rayed,Ri), float(0)),shininess)*specularColor;
        
        Vector3f colorSum = diffuse + specular;
        Vector3f shaded(lightColor.x()*colorSum.x(), lightColor.y()*colorSum.y(), lightColor.z()*colorSum.z());
        return shaded;
        //return Vector3f::ZERO;
    }

protected:
    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;
};


#endif // MATERIAL_H
