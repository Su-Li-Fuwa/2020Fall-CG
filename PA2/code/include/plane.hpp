#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

//  (PA2): Copy from PA1

class Plane : public Object3D {
public:
    Plane() : Object3D() {
    }

    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m) {
        this->norm = normal.normalized();
        this->d = d/Vector3f::dot(this->norm, normal);
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        float denominator = Vector3f::dot(norm, r.getDirection());
        if (denominator != 0){
            float dividend = d -Vector3f::dot(norm, r.getOrigin()), tTmp = dividend/denominator;
            if (tTmp > 0){
                if (tTmp < h.getT() and tTmp >= tmin){
                    if (Vector3f::dot(r.getDirection(), norm) < 0)
                        h.set(tTmp, material, norm);
                    else
                        h.set(tTmp, material, -norm);
                }
                return true;
            }
        }
        return false;
    }

    void drawGL() override {
        Object3D::drawGL();
        Vector3f xAxis = Vector3f::RIGHT;
        Vector3f yAxis = Vector3f::cross(norm, xAxis);
        xAxis = Vector3f::cross(yAxis, norm);
        const float planeSize = 10.0;
        glBegin(GL_TRIANGLES);
        glNormal3fv(norm);
        glVertex3fv(d * norm + planeSize * xAxis + planeSize * yAxis);
        glVertex3fv(d * norm - planeSize * xAxis - planeSize * yAxis);
        glVertex3fv(d * norm + planeSize * xAxis - planeSize * yAxis);
        glNormal3fv(norm);
        glVertex3fv(d * norm + planeSize * xAxis + planeSize * yAxis);
        glVertex3fv(d * norm - planeSize * xAxis + planeSize * yAxis);
        glVertex3fv(d * norm - planeSize * xAxis - planeSize * yAxis);
        glEnd();
    }

protected:
    Vector3f norm;
    float d;

};

#endif //PLANE_H
		

