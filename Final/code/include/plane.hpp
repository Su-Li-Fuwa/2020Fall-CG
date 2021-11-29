#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

class Plane : public Object3D {
public:
    Plane() : Object3D() {
    }

    Plane(const Vector3f &normal, double d, Material *m) : Object3D(m) {
        this->normal = normal.normalized();
        this->d = d/Vector3f::dot(this->normal, normal);
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, double tmin) override {
        double denominator = Vector3f::dot(normal, r.getDirection());
        if (denominator != 0){
            double dividend = d -Vector3f::dot(normal, r.getOrigin()), tTmp = dividend/denominator;
            if (tTmp > 0){
                if (tTmp < h.getT() and tTmp >= tmin){
                    Vector3f hitPoint = r.pointAtParameter(tTmp);
                    Vector3f pX = Vector3f::cross(normal, material->textDir).normalized(), pY = Vector3f::cross(normal, pX).normalized();
                    double u = Vector3f::dot(hitPoint, pX), v = Vector3f::dot(hitPoint, pY);
                    if (Vector3f::dot(r.getDirection(), normal) < 0)
                        h.set(tTmp, material, normal, u, v);
                    else
                        h.set(tTmp, material, -normal, u, v);
                    return true;
                }
            }
        }
        return false;
    }

protected:
    Vector3f normal;
    double d;

};

class FinitePlane : public Object3D {
public:
    FinitePlane() : Object3D() {
    }

    FinitePlane(const Vector3f &normal, double d, Material *m, const Vector3f* vertex) : Object3D(m) {
        this->normal = normal.normalized();
        this->d = d/Vector3f::dot(this->normal, normal);
        for (int i=0;i<4;i++){
            this->vertex[i] = *(vertex+i);
            this->vertex[i].print();
        }
    }

    ~FinitePlane() override = default;

    bool intersect(const Ray &r, Hit &h, double tmin) override {
        double denominator = Vector3f::dot(normal, r.getDirection());
        if (denominator != 0){
            double dividend = d -Vector3f::dot(normal, r.getOrigin()), tTmp = dividend/denominator;
            if (tTmp > 0){
                if (tTmp < h.getT() and tTmp >= tmin){
                    Vector3f hitPoint = r.pointAtParameter(tTmp);
                    Vector3f differ[4] = {vertex[0]-hitPoint, vertex[1]-hitPoint, vertex[2]-hitPoint, vertex[3]-hitPoint};
                    Vector3f crossl[4] = {Vector3f::cross(differ[0], differ[1]), Vector3f::cross(differ[1], differ[2]), Vector3f::cross(differ[2], differ[3]), Vector3f::cross(differ[3], differ[0])};
                    double dotl[4] = {Vector3f::dot(crossl[0], crossl[1]), Vector3f::dot(crossl[1], crossl[2]), Vector3f::dot(crossl[2], crossl[3]), Vector3f::dot(crossl[3], crossl[0])};
                    if (dotl[0] >= EPS && dotl[1] >= EPS && dotl[2] >= EPS && dotl[3] >= EPS){
                        Vector3f tempW = vertex[3]-vertex[0], tempH = vertex[1]-vertex[0];
                        double u = Vector3f::dot((hitPoint - vertex[0]), tempW)/tempW.squaredLength()*material->tw, v = Vector3f::dot((hitPoint - vertex[0]), tempH)/tempH.squaredLength()*material->th;
                        if (Vector3f::dot(r.getDirection(), normal) < 0)
                            h.set(tTmp, material, normal, u, v);
                        else
                            h.set(tTmp, material, -normal, u, v);
                        return true;
                    }
                }
            }
        }
        return false;
    }

protected:
    Vector3f normal;
    double d;
    Vector3f vertex[4];

};

#endif //PLANE_H
		

