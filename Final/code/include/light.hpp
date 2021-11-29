#ifndef LIGHT_H
#define LIGHT_H

#include <Vector3f.h>
#include "object3d.hpp"
#include "photon.hpp"

class Light {
public:
    Light() = default;

    virtual ~Light() = default;

    virtual void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const = 0;

    virtual bool isShadow(const Vector3f &p, const Vector3f &q) const = 0;

    double getColorPower() const{
        return (color.x()+color.y()+color.z())/3;
    }

    virtual int isHit(const Vector3f &o, const Vector3f &V) const = 0;
    
    virtual Photon EmitPhoton() const = 0;

    Vector3f color;
};


class DirectionalLight : public Light {
public:
    DirectionalLight() = delete;

    DirectionalLight(const Vector3f &d, const Vector3f &c) {
        direction = d.normalized();
        color = c;
    }

    ~DirectionalLight() override = default;

    ///@param p unsed in this function
    ///@param distanceToLight not well defined because it's not a point light
    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = -direction;
        col = color;
    }

    bool isShadow(const Vector3f &p, const Vector3f &q) const override {
        double tmp = Vector3f::dot(p-q, direction);
        return (tmp>0);
    }
    int isHit(const Vector3f &o, const Vector3f &V) const override {
        return false;
    }

    Photon EmitPhoton() const override{
        assert(false);
        double refrN = 1;
        Vector3f power = color / getColorPower();
	    Vector3f pos = Vector3f(0);
	    Vector3f dir = direction;
	    return Photon(pos, dir, power, 0, refrN);
    }

private:
    Vector3f direction;
};

class PointLight : public Light {
public:
    PointLight() = delete;

    PointLight(const Vector3f &p, const Vector3f &c) {
        position = p;
        color = c;
    }

    ~PointLight() override = default;

    int isHit(const Vector3f &o, const Vector3f &V) const override {
        return false;
    }

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = (position - p);
        dir = dir / dir.length();
        col = color;
    }

    bool isShadow(const Vector3f &p,const Vector3f &q) const override {
        double tmp = Vector3f::dot(p-position, q-position);
        return (tmp>0);
    }

    Photon EmitPhoton() const override{
        double refrN = 1;
        Vector3f power = color / getColorPower();
	    Vector3f pos = position;
	    Vector3f dir = Vector3f(2*ran()-1, 2*ran()-1, 2*ran()-1).normalized();
	    return Photon(pos, dir, power, 0, refrN);
    }

private:
    Vector3f position;
};


class AreaLight : public Light {
public:
	AreaLight(const Vector3f &p, const Vector3f &c, const Vector3f &dx, const Vector3f &dy) {
        position = p;
        color = c;
        //color.print();
        dirX = dx, dirY = dy;
    }
	~AreaLight() {}
	
    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = (position + dirX * ( ran() * 2 - 1 ) + dirY * ( ran() * 2 - 1 ) - p);
        dir = dir / dir.length();
        col = color;
    }

    bool isShadow(const Vector3f &p,const Vector3f &q) const override {
        Vector3f pos = position + dirX * ( ran() * 2 - 1 ) + dirY * ( ran() * 2 - 1);
        double tmp = Vector3f::dot(p-pos, q-pos);
        return (tmp>0);
    }


    int isHit(const Vector3f &o, const Vector3f &V) const override {
        Vector3f normal = Vector3f::cross(dirX, dirY).normalized();
        //if (Vector3f::dot(V, norm) == 0)   return false;
        double denominator = Vector3f::dot(normal, V);
        if (denominator >= EPS/10 || denominator <= -EPS/10 ){
			//printf("Hit!!\n");
            double dividend = Vector3f::dot(normal, position) -Vector3f::dot(normal, o), tTmp = dividend/denominator;
            if (tTmp > EPS){		
                Vector3f hitPoint = o + tTmp*V;
				Vector3f l1(position + dirX + dirY - hitPoint), l2(position + dirX - dirY - hitPoint), l3(position - dirX - dirY - hitPoint), l4(position - dirX + dirY - hitPoint);
				Vector3f h1 = Vector3f::cross(l1,l2).normalized(), h2 = Vector3f::cross(l2,l3).normalized(), h3 = Vector3f::cross(l3,l4).normalized(), h4 = Vector3f::cross(l4,l1).normalized();
				double tmp[4] = {Vector3f::dot(h1,h2), Vector3f::dot(h2,h3), Vector3f::dot(h3,h4), Vector3f::dot(h4,h1)};
				if (tmp[0] > EPS and tmp[1] > EPS and tmp[2] > EPS and tmp[3] > EPS){
                    return tTmp;
				}
			}
		}
        return 0;
    }

	Photon EmitPhoton() const override{
        double refrN = 1, x,y,z;
        Vector3f power = color / getColorPower();
	    Vector3f pos = position + dirX * ( ran() * 2 - 1 ) + dirY * ( ran() * 2 - 1 );
	    do {
            x = 2 * ran() - 1;
            y = 2 * ran() - 1;
            z = 2 * ran() - 1;
        } while ( x * x + y * y + z * z > 1 || x * x + y * y + z * z < EPS );
        Vector3f dir = Vector3f(x,y,z).normalized();
	    return Photon(pos, dir, power, 0, refrN);
    }

private:
    Vector3f position;
    Vector3f dirX, dirY;
};

class DirectionalRecLight : public Light {
public:
	DirectionalRecLight(const Vector3f &p, const Vector3f &d, const Vector3f &c, const Vector3f &up, double dx, double dy) {
        position = p;
        direction = d.normalized();
        color = c;
        Vector3f tmpX = Vector3f::cross(up, direction).normalized(), tmpY = Vector3f::cross(tmpX, direction).normalized(); 
        //color.print();
        dirX = dx*tmpX, dirY = dy*tmpY;
    }
	~DirectionalRecLight() {}
	
    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = (position + dirX * ( ran() * 2 - 1 ) + dirY * ( ran() * 2 - 1 ) - p);
        dir = dir / dir.length();
        col = color;
    }
    int isHit(const Vector3f &o, const Vector3f &V) const override {
        Vector3f normal = Vector3f::cross(dirX, dirY).normalized();
        //if (Vector3f::dot(V, norm) == 0)   return false;
        double denominator = Vector3f::dot(normal, V);
        if (denominator >= EPS/10 || denominator <= -EPS/10 ){
			//printf("Hit!!\n");
            double dividend = Vector3f::dot(normal, position) -Vector3f::dot(normal, o), tTmp = dividend/denominator;
            if (tTmp > EPS){		
                Vector3f hitPoint = o + tTmp*V;
				Vector3f l1(position + dirX + dirY - hitPoint), l2(position + dirX - dirY - hitPoint), l3(position - dirX - dirY - hitPoint), l4(position - dirX + dirY - hitPoint);
				Vector3f h1 = Vector3f::cross(l1,l2).normalized(), h2 = Vector3f::cross(l2,l3).normalized(), h3 = Vector3f::cross(l3,l4).normalized(), h4 = Vector3f::cross(l4,l1).normalized();
				double tmp[4] = {Vector3f::dot(h1,h2), Vector3f::dot(h2,h3), Vector3f::dot(h3,h4), Vector3f::dot(h4,h1)};
				if (tmp[0] > EPS and tmp[1] > EPS and tmp[2] > EPS and tmp[3] > EPS){
                    return tTmp;
				}
			}
		}
        return 0;
    }

    bool isShadow(const Vector3f &p,const Vector3f &q) const override {
        Vector3f pos = position + dirX * ( ran() * 2 - 1 ) + dirY * ( ran() * 2 - 1);
        double tmp = Vector3f::dot(p-pos, q-pos);
        return (tmp>0);
    }

	Photon EmitPhoton() const override{
        double refrN = 1, x,y,z;
        Vector3f power = color / getColorPower();
	    Vector3f pos = position + dirX * ( ran() * 2 - 1 ) + dirY * ( ran() * 2 - 1 );
        Vector3f dir = direction;
	    return Photon(pos, dir, power, 0, refrN);
    }



private:
    Vector3f position;
    Vector3f direction;
    Vector3f dirX, dirY;
};

#endif // LIGHT_H
