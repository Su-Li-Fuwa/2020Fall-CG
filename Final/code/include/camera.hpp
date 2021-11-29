#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>


class Camera {
public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH, int isDF=0, 
        double lenRadius=0, int lenSampleNum=0, double focusLen=0) {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up);
        this->horizontal.normalize();
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
        this->isDF = isDF;
        this->lenRadius = lenRadius;
        this->lenSampleNum = lenSampleNum;
        this->focusLen = focusLen;
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point) = 0;
    virtual ~Camera() = default;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    // Intrinsic parameters
    int width;
    int height;

    // parameters for depth of field
    int isDF = 0;
    double lenRadius;
    int lenSampleNum;
    double focusLen;
};

class PerspectiveCamera : public Camera {

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH, 
            double angle,int isDF,double lenRadius,int lenSampleNum,double focusLen) : Camera(center, direction, up, imgW, imgH,
             isDF, lenRadius, lenSampleNum, focusLen) {
        this->angle = angle;
        double a = 0.5*double(imgH)/tan(angle/2);
        zH = a;//sqrt(a*a-(double(imgW)/2)*(double(imgW)/2)); 
        // angle is in radian.
    }

    Ray generateRay(const Vector2f &point) override {
        // normalized
        Vector3f direc = (point.x()-double(width)/2)*horizontal + (point.y()-double(height)/2)*up + zH*direction;
        direc.normalize();
        return Ray(center, direc);
    }
private:
    double angle;
    double zH;
};

#endif //CAMERA_H
