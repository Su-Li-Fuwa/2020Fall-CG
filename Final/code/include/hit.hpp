#ifndef HIT_H
#define HIT_H

#include <vecmath.h>
#include "ray.hpp"

#define HITPOINTOUTER 0.1

class Material;

class Hit {
public:

    // constructors
    Hit() {
        material = nullptr;
        t = 1e38;
    }

    Hit(double _t, Material *m, const Vector3f &n, double _u=0, double _v=0) {
        t = _t;
        material = m;
        normal = n;
        u = _u;
        v = _v;
    }

    Hit(const Hit &h) {
        t = h.t;
        material = h.material;
        normal = h.normal;
        u = h.u;
        v = h.v;
    }

    // destructor
    ~Hit() = default;

    double getT() const {
        return t;
    }

    Material *getMaterial() const {
        return material;
    }

    const Vector3f &getNormal() const {
        return normal;
    }

    double getU() const{
        return u;
    }

    double getV() const{
        return v;
    }

    void set(double _t, Material *m, const Vector3f &n, double _u=0, double _v=0) {
        t = _t;
        material = m;
        normal = n;
        u = _u;
        v = _v;
    }

private:
    double t;
    Material *material;
    Vector3f normal;
    double u, v;

};

inline std::ostream &operator<<(std::ostream &os, const Hit &h) {
    os << "Hit <" << h.getT() << ", " << h.getNormal() << ">";
    return os;
}

#endif // HIT_H
