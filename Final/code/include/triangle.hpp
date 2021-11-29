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
	int vertex[3], textureVertex[3], normalVectorID[3];
	double nu, nv, nd, bnu, bnv, cnu, cnv;
	int mainCoord;

	double d;
	Triangle(Material* m) : Object3D(m) {}
	double GetMinCoord(int coord) {
		double x0 = v1[coord];
		double x1 = v2[coord];
		double x2 = v3[coord];
		if (x0 < x1)
			return (x0 < x2) ? x0 : x2;
		return (x1 < x2) ? x1 : x2;
	}

	double GetMaxCoord(int coord) {
		double x0 = v1[coord];
		double x1 = v2[coord];
		double x2 = v3[coord];
		if (x0 > x1)
			return (x0 > x2) ? x0 : x2;
		return (x1 > x2) ? x1 : x2;
	}

	void Setparameters(){
		Vector3f B = v3 -v1, C = v2 - v1;
		normal = Vector3f::cross(B,C);
		normal.normalize();
		//normal.print();
		if (normal == Vector3f::ZERO) {
			normal = Vector3f(0, 0, 1);
			return;
		}
		if (fabs(normal[0]) > fabs(normal[1]))
			mainCoord = (fabs(normal[0]) > fabs(normal[2])) ? 0 : 2;
		else
			mainCoord = (fabs(normal[1]) > fabs(normal[2])) ? 1 : 2;
		int u = (mainCoord + 1) % 3, v = (mainCoord + 2) % 3;
		double krec = 1.0 / normal[mainCoord];
		nu = normal[u] * krec;
		nv = normal[v] * krec;
		nd = Vector3f::dot(normal, v1) * krec;
		double reci = 1.0 / (B[u] * C[v] - B[v] * C[u]);
		bnu = B[u] * reci;
		bnv = -B[v] * reci;
		cnu = C[v] * reci;
		cnv = -C[u] * reci;
		
	}
    // a b c are three vertex positions of the triangle
	Triangle( const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m) {
		v1 = a, v2 = b, v3 = c;
	}

	double myDet(const Vector3f &a, const Vector3f &b, const Vector3f &c) {
    return a.x() * (b.y() * c.z() - b.z() * c.y()) 
        - b.x() * (a.y() * c.z() - a.z() * c.y()) 
        + c.x() * (a.y() * b.z() - a.z() * b.y());
	}

	bool intersect( const Ray& ray,  Hit& hit , double tmin) override {
		
		Vector3f E1 = v1 - v2, E2 = v1 - v3, S = v1 - ray.getOrigin();
		double t = myDet(S, E1, E2);
		double beta = myDet(ray.getDirection(), S, E2);
		double gamma = myDet(ray.getDirection(), E1, S);
		double n = myDet(ray.getDirection(), E1, E2);
		t /= n;
		beta /= n;
		gamma /= n;
		if (!(-EPS <= beta && beta <= 1 + EPS && 
			-EPS <= gamma && gamma <= 1 + EPS && beta + gamma <= 1 + EPS))
			return false;

		Vector3f norm = Vector3f::cross(v2-v1, v3-v1);
		if (Vector3f::dot(norm, ray.getDirection()) > 0) norm = norm * -1;
		norm.normalize();        
		if (t >= 0 and t <= hit.getT()){
			hit.set(t, material, norm);
			return true;
		}
		return false;	
	}


};

#endif // TRIANGLE_H
