#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "ray.hpp"
#include "hit.hpp"
#include "image.hpp"
#include <iostream>

#define EPS 1e-6

// TODO: Implement Shade function that computes Phong introduced in class.
class Material {
public:
    Material(){
        diffidx=0, specidx=0, shininess=0;
        reflidx=0;
        refridx=0, refrN=0;
        textcoff=0;
        mColor = Vector3f(0);
        absorb = Vector3f(0);
        textDir = Vector3f(0);
        texture=NULL;
        tw=0, th=0;
    }
    Material(const Vector3f &mColor,const Vector3f &textDir_=Vector3f::ZERO, const Vector3f &absorb = Vector3f::ZERO, double d = 1,
                        double sp = 0, double sh = 0, double reflidx = 0, double refridx = 0, double refrN = 1, char* fName = NULL,double tc = 1) :
            mColor(mColor), absorb(absorb), diffidx(d), specidx(sp), shininess(sh), reflidx(reflidx), refridx(refridx), refrN(refrN), textcoff(tc) {
        
        if (*fName != 0){
            texture = Image::LoadPPM(fName);
            //std::cout << fName << "***"<<texture->Width()<<" "<<texture->Height()<<"\n";
            tw = texture->Width();
            th = texture->Height();
            textDir = textDir_;
        }
        else {
            tw = 0;
            th = 0;
            texture = NULL;
        }
    }

    virtual ~Material() = default;

    virtual Vector3f getColor() const {
        return mColor;
    }
    double getColorPower() const{
        return (mColor.x()+mColor.y()+mColor.z())/3;
    }

    double getDiffidx() const {
        return diffidx;
    }

    double getSpecidx() const {
        return specidx;
    }

    double getReflidx() const {
        return reflidx;
    }

    double getRefridx() const {
        return refridx;
    }

    double getN() const {
        return refrN;
    }

    Vector3f getTextureColor(double u, double v){
        int x = ((int(u*textcoff))%tw+tw)%tw, y = (int(v*textcoff)%th+th)%th;
        Vector3f color = texture->GetPixel(x, y);
        return color;
    }

    inline double clamp(double x){ return x<0 ? 0 : x>1 ? 1 : x; }

    // Basic phong shading, not be used in PM
    Vector3f diffuseShade(const Ray &ray, const Hit &hit, Vector3f &dirToLight, Vector3f &lightColor){
        Vector3f color;
        if (texture == NULL)    color = mColor;
        else{
            color = getTextureColor(hit.getU(), hit.getV());
        }
        Vector3f hitNormed = hit.getNormal().normalized(), dirToLighted = dirToLight.normalized(), rayed = ray.getDirection().normalized();
        Vector3f Ri = 2*Vector3f::dot(hitNormed, dirToLighted)*hitNormed - dirToLighted;
        Ri.normalize();
        double diffuse = clamp(Vector3f::dot(dirToLighted, hitNormed))*diffidx;
        double specular = pow(clamp(Vector3f::dot(-rayed,Ri)), shininess)*specidx;
        Vector3f colorSum = (diffuse + specular)*color;
        Vector3f shaded(lightColor.x()*colorSum.x(), lightColor.y()*colorSum.y(), lightColor.z()*colorSum.z());
        return shaded;
    }

public:
    double diffidx, specidx, shininess;
    double reflidx;
    double refridx, refrN;
    double textcoff;
    Vector3f mColor, absorb, textDir;
    Image *texture;
    int tw, th;
    int sample = rand();
    double drefl = 0;
};


#endif // MATERIAL_H
