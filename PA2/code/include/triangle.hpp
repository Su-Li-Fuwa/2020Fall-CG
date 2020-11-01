#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>

using namespace std;

//  (PA2): Copy from PA1
class Triangle: public Object3D
{

public:
	Vector3f normal;
	Vector3f vertices[3];
	float d;

	Triangle() = delete;

    // a b c are three vertex positions of the triangle
	Triangle( const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m) {
		vertices[0] = a, vertices[1] = b, vertices[2] = c;
	}

	bool intersect( const Ray& ray,  Hit& hit , float tmin) override {
        float denominator = Vector3f::dot(normal, ray.getDirection());
        if (denominator != 0){
			
            float dividend = d -Vector3f::dot(normal, ray.getOrigin()), tTmp = dividend/denominator;
            if (tTmp > 0 and tTmp < hit.getT() and tTmp >= tmin){		
				Vector3f l1(vertices[0]-ray.pointAtParameter(tTmp)), l2(vertices[1]-ray.pointAtParameter(tTmp)), l3(vertices[2]-ray.pointAtParameter(tTmp));
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


    void drawGL() override {
        Object3D::drawGL();
        glBegin(GL_TRIANGLES);
        glNormal3fv(normal);
        glVertex3fv(vertices[0]); glVertex3fv(vertices[1]); glVertex3fv(vertices[2]);
        glEnd();
    }

protected:
};

#endif //TRIANGLE_H
