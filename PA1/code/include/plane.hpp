#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement Plane representing an infinite plane
// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D {
public:
    Plane() : Object3D() {
    }

    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m) {
        this->normal = normal.normalized();
        this->d = d/Vector3f::dot(this->normal, normal);
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        float denominator = Vector3f::dot(normal, r.getDirection());
        if (denominator != 0){
            float dividend = d -Vector3f::dot(normal, r.getOrigin()), tTmp = dividend/denominator;
           // printf("^^^^^^ %f %f %f %f %f %f\n", dividend, denominator, tTmp, r.getDirection().x(), r.getDirection().y(), r.getDirection().z());
            if (tTmp > 0){
                if (tTmp < h.getT() and tTmp >= tmin){
                    if (Vector3f::dot(r.getDirection(), normal) < 0)
                        h.set(tTmp, material, normal);
                    else
                        h.set(tTmp, material, -normal);
                }
                return true;
            }
        }
        return false;
    }

protected:
    Vector3f normal;
    float d;

};

#endif //PLANE_H
		

