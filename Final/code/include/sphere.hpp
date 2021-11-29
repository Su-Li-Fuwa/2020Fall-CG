#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement functions and add more fields as necessary

class Sphere : public Object3D {
public:
    Sphere() : Object3D() {
        // unit ball at the center
        this->center =  Vector3f(0,0,0);
        this->radius = 1;
    }

    Sphere(const Vector3f &center, double radius, Material *material) : Object3D(material) {
        this->center = center;
        this->radius = radius;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, double tmin) override {
        Vector3f OC, OH, CH, red =  r.getDirection().normalized();
        double CH2, tOH;
        OC = center - r.getOrigin();
        tOH = Vector3f::dot(OC, red);
        OH = tOH*red;
        CH2 = Vector3f::dot(OC,OC) - Vector3f::dot(OH,OH);
        if (CH2 > radius*radius)
            return false;
        else{
            double tPH = sqrt(radius*radius - CH2), tOP;
            if (OC.length() > radius)
                tOP = (tOH - tPH) / Vector3f::dot(r.getDirection(), red);
            else
                tOP = (tOH + tPH) / Vector3f::dot(r.getDirection(), red);
            if (tOP <= 0)
                return false;
            if (tOP < h.getT() and tOP >= tmin){
                Vector3f Norm = r.pointAtParameter(tOP) - center;
                Vector3f d1 = Vector3f::cross(material->textDir, Vector3f(1.2,2.3,3.4).normalized()).normalized();
                Vector3f d2 = Vector3f::cross(material->textDir, d1).normalized();
                Norm.normalize();
                double cosTheta = Vector3f::dot(Norm, d1);
                Vector3f projected = (Norm - cosTheta*d1).normalized();
                double cosPhi = Vector3f::dot(projected, d2),
                      sinPhi = Vector3f::dot(projected, material->textDir);
                double Theta = acos(cosTheta), Phi = acos(cosPhi);
                if (sinPhi<0)   Phi += M_PI;
                double u = double(Phi/(2*M_PI))*material->tw, v = double(Theta/(1*M_PI))*material->th;
                if (Vector3f::dot(r.getDirection(), Norm) < 0)
                    h.set(tOP, material, Norm, u, v);
                else
                    h.set(tOP, material, -Norm ,u, v);
            }
            return true;
        }
    }

protected:
    Vector3f center;
    double radius;
};


#endif
