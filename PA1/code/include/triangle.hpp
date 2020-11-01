#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
using namespace std;

// TODO: implement this class and add more fields as necessary,
class Triangle: public Object3D {

public:
	Vector3f v1,v2,v3,normal;
	float d;
	Triangle() = delete;

    // a b c are three vertex positions of the triangle
	Triangle( const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m) {
		v1 = a, v2 = b, v3 = c;
		//printf("%f %f %f\n", v1.x(), v1.y(), v1.z());
		//printf("%f %f %f\n", normal.x(), normal.y(), normal.z());
		//cin.get();
	}

	bool intersect( const Ray& ray,  Hit& hit , float tmin) override {
        float denominator = Vector3f::dot(normal, ray.getDirection());
        if (denominator != 0){
			
            float dividend = d -Vector3f::dot(normal, ray.getOrigin()), tTmp = dividend/denominator;
            if (tTmp > 0 and tTmp < hit.getT() and tTmp >= tmin){		
				Vector3f l1(v1-ray.pointAtParameter(tTmp)), l2(v2-ray.pointAtParameter(tTmp)), l3(v3-ray.pointAtParameter(tTmp));
				Vector3f h1 = Vector3f::cross(l1,l2), h2 = Vector3f::cross(l2,l3), h3 = Vector3f::cross(l3,l1);
				float tmp[3] = {Vector3f::dot(h1,h2), Vector3f::dot(h2,h3), Vector3f::dot(h3,h1)};
				//printf("%f %f %f\n",tmp[0], tmp[1], tmp[2]);
				if (tmp[0] >= 0 and tmp[1] >= 0 and tmp[2] >=0){
					//printf("Hit!\n");
					hit.set(tTmp, material, normal.normalized());
					return true;
				}
			}
		}
        return false;
	}

};

#endif // TRIANGLE_H
