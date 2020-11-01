#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <glut.h>

// (PA2): Copy from PA1

class Sphere : public Object3D {
public:
    Sphere() : Object3D() {
        // unit ball at the center
        this->center =  Vector3f(0,0,0);
        this->radius = 1;
    }

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material) {
        this->center = center;
        this->radius = radius;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        Vector3f OC, OH, CH, red =  r.getDirection().normalized();
        float CH2, tOH;
        OC = center - r.getOrigin();
        tOH = Vector3f::dot(OC, red);
        OH = tOH*red;
        CH2 = Vector3f::dot(OC,OC) - Vector3f::dot(OH,OH);
        if (CH2 > radius*radius)
            return false;
        else{
            float tPH = sqrt(radius*radius - CH2), tOP;
            tOP = (tOH - tPH) / Vector3f::dot(r.getDirection(), red);
            //printf("%f\n", tOP);
            if (tOP < 0)
                return false;
            if (tOP < h.getT() and tOP >= tmin){
                Vector3f Norm = r.pointAtParameter(tOP) - center;
                Norm.normalize();
                if (Vector3f::dot(r.getDirection(), Norm) < 0)
                    h.set(tOP, material, Norm);
                else
                    h.set(tOP, material, -Norm);
            }
            return true;
        }
    }

    void drawGL() override {
        Object3D::drawGL();
        glMatrixMode(GL_MODELVIEW); glPushMatrix();
        glTranslatef(center.x(), center.y(), center.z());
        glutSolidSphere(radius, 80, 80);
        glPopMatrix();
    }

protected:
    Vector3f center;
    float radius;

};


#endif
