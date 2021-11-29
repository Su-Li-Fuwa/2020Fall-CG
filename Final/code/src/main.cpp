#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"
#include "photonMapping.hpp"

#include <string>

#define MAXDEPTH 20 // define for basic shading

using namespace std;

Vector3f groudColor;

// Basic shading using phong shading, not be used in PM
Vector3f basicShade(const Ray &ray, Light* light, Group* baseGroup, double oriN, int depth) {
    Hit hit, between;
    Vector3f shaded = Vector3f(groudColor);
    bool isIntersect = baseGroup->intersect(ray, hit, 0);
    if (isIntersect){
        Vector3f L, lightColor;
        if (depth > MAXDEPTH){
            return Vector3f(hit.getMaterial()->getColor());
        }
        light->getIllumination(ray.pointAtParameter(hit.getT()), L, lightColor);
        L = L.normalized();
        shaded = hit.getMaterial()->diffuseShade(ray, hit, L, lightColor);
        Ray rayToLight = Ray(ray.pointAtParameter(hit.getT())+0.01*L, L);
        bool isShadowIn = baseGroup->intersect(rayToLight, between, 0);
        if ( isShadowIn ){
            bool isShadow = light->isShadow(ray.pointAtParameter(hit.getT())+0.01*L, rayToLight.pointAtParameter(between.getT()));
            if (isShadow)  shaded = Vector3f(0,0,0);
        }

        Vector3f hitNormed = hit.getNormal().normalized(), hitPoint = ray.pointAtParameter(hit.getT()), nRayed = -ray.getDirection().normalized();
        Vector3f reflDir = 2*Vector3f::dot(hitNormed, nRayed)*hitNormed - nRayed;
        Ray reflRay = Ray(hitPoint+0.01*hitNormed, reflDir);
        if (hit.getMaterial()->getReflidx() > 0){
            shaded += hit.getMaterial()->getReflidx()*basicShade(reflRay, light, baseGroup, oriN, depth+1);
            return shaded;
        }

        Vector3f refrDirVer = (-nRayed - Vector3f::dot(hitNormed, -nRayed)*hitNormed)*oriN/hit.getMaterial()->getN();
        double nnt = oriN/hit.getMaterial()->getN(), ddn = Vector3f::dot(-nRayed, hitNormed), cos2t=1-nnt*nnt*(1-ddn*ddn);
        cos2t = 1 - (refrDirVer.length()*refrDirVer.length());
        if (cos2t<0 && hit.getMaterial()->getRefridx() > 0)
            return shaded + basicShade(reflRay, light, baseGroup, oriN, depth+1);

        Vector3f refrDir = refrDirVer - sqrt(cos2t)*hitNormed;
        refrDir = refrDir.normalized();
        Ray refrRay = Ray(hitPoint-0.01*hitNormed, refrDir);
        double a=hit.getMaterial()->getN() - oriN, b=hit.getMaterial()->getN() + oriN, R0=a*a/(b*b), c = 1+ddn;
        double Re=R0+(1-R0)*c*c*c*c*c,Tr=1-Re,P=.25+.5*Re,RP=Re/P,TP=Tr/(1-P);
        if (hit.getMaterial()->getRefridx() > 0){
            shaded += Tr*hit.getMaterial()->getRefridx()*basicShade(refrRay, light, baseGroup, hit.getMaterial()->getN(), depth+1)+
            Re*hit.getMaterial()->getRefridx()*basicShade(reflRay, light, baseGroup, oriN, depth+1);
        }
    }
    return shaded;
}

int main(int argc, char *argv[]) {
    for (int argNum = 1; argNum < argc; ++argNum) {
        std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
    }

    if (argc != 3) {
        cout << "Usage: ./bin/PM <input scene file> <output bmp file>" << endl;
        return 1;
    }
    string inputFile = argv[1];
    string outputFile = argv[2];  // only bmp is allowed.

    SceneParser sceneParser(argv[1]);
    PhotonMapping photonMapping(&sceneParser);
    photonMapping.buildMap();

    groudColor = sceneParser.getBackgroundColor();
    Camera* camera = sceneParser.getCamera();
    Image renderedImg(camera->getWidth(), camera->getHeight());

    // backward tracing
    Vector3f finalColor(0);
    #pragma omp parallel for schedule(dynamic, 1) private(finalColor)
    for (int x = 0; x < camera->getWidth(); ++x) {
        printf("%d\n", x);
        for (int y = 0; y < camera->getHeight(); ++y) {
            if (!camera->isDF){
                Ray camRay = camera->generateRay(Vector2f(x, y));
                finalColor = photonMapping.backwardTracing(camRay, 1);
                renderedImg.SetPixel(x, y, finalColor);
            }
            else{
                finalColor = Vector3f(0);
                Ray camRay = camera->generateRay(Vector2f(x, y));
                Vector3f camCenter = camera->center, focusPoint = camRay.pointAtParameter(camera->focusLen);
                Vector3f dirX = Vector3f::cross(camera->direction, camera->up).normalized();
                Vector3f dirY = Vector3f::cross(camera->direction, dirX).normalized();
                for (int i=0;i<camera->lenSampleNum; i++){
                    double dx=( ran() * 2 - 1 ), dy = ( ran() * 2 - 1 ), square = dx*dx+dy*dy;
                    dx /= sqrt(square), dy /= square;
                    Vector3f newO = camCenter + camera->lenRadius*dx*dirX + camera->lenRadius*dy*dirY;
                    Ray newRay = Ray(newO, (focusPoint-newO).normalized());
                    finalColor += photonMapping.backwardTracing(newRay, 1);
                }
                renderedImg.SetPixel(x, y, finalColor/camera->lenSampleNum);
            }
        }
    }
    renderedImg.SaveImage(argv[2]);
    cout << "Finished!" << endl;
    return 0;
}

