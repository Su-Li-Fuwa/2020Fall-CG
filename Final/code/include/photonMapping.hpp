#ifndef PHOTONMAPPING_H
#define PHOTONMAPPING_H

#include "scene_parser.hpp"
#include "group.hpp"
#include "hit.hpp"
#include "ray.hpp"
#include "photon.hpp"

#define MAX_TRACING_DEPTH 8

const int MAX_DREFL_DEP = 2;

Vector3f rotation(const Vector3f &target, const Vector3f &axis, double theta ) {
	double retx, rety, retz;
    double targetx = target.x(), targety = target.y(), targetz = target.z();
    double axisx = axis.x(), axisy = axis.y(), axisz = axis.z(); 
	double cost = cos( theta );
	double sint = sin( theta );
	retx += targetx * ( axisx * axisx + ( 1 - axisx * axisx ) * cost );
	retx += targety * ( axisx * axisy * ( 1 - cost ) - axisz * sint );
	retx += targetz * ( axisx * axisz * ( 1 - cost ) + axisy * sint );
	rety += targetx * ( axisy * axisx * ( 1 - cost ) + axisz * sint );
	rety += targety * ( axisy * axisy + ( 1 - axisy * axisy ) * cost );
	rety += targetz * ( axisy * axisz * ( 1 - cost ) - axisx * sint );
	retz += targetx * ( axisz * axisx * ( 1 - cost ) - axisy * sint );
	retz += targety * ( axisz * axisy * ( 1 - cost ) + axisx * sint );
	retz += targetz * ( axisz * axisz + ( 1 - axisz * axisz ) * cost );
	return Vector3f(retx, rety, retz);
}

class PhotonMapping {
public:
	PhotonMapping(SceneParser* _scene) { 
        scene =_scene; 
        baseGroup = scene->getGroup(); 
    }

    bool forwardDiff(Hit *hit , Photon photon, int depth , double* pr){
        Material* material = hit->getMaterial();
        double randP = material->getDiffidx()*material->getColorPower(), tmp = ran() * ( *pr );
        if ( randP <=  tmp) {
            *pr -= randP;
            return false;
        }
        Vector3f hitNormed = hit->getNormal().normalized(), 
                 normVer = Vector3f::cross(hitNormed, Vector3f(1.1,0.2,0.23)).normalized();
        double theta = acos( sqrt( ran() ) ), phi = ran() * 2 * M_PI; 
        
        // Randomly select new dir to forward 
        photon.dir = rotation(rotation(hitNormed, normVer, theta), hitNormed, phi).normalized();
        photon.pos += HITPOINTOUTER*photon.dir;
        photon.power = photon.power * material->getColor() / material->getColorPower();
        forwardTracing( photon , depth + 1);
        return true;
    }

	bool forwardRefl(Hit *hit , Photon photon, int depth , double* pr){
        Material* material = hit->getMaterial();
        double randP = material->getReflidx() * material->getColorPower(), tmp = ran() * ( *pr );
        if ( randP <=  tmp) {
            *pr -= randP;
            return false;
        }
        Vector3f hitNormed = hit->getNormal().normalized(), nRayed = -photon.dir.normalized();
        Vector3f reflDir = (2*Vector3f::dot(hitNormed, nRayed)*hitNormed - nRayed).normalized();
        
        photon.dir = reflDir;
        photon.pos += HITPOINTOUTER*photon.dir;
        
        photon.power = photon.power * material->getColor() / material->getColorPower();
        forwardTracing( photon , depth + 1 );
        return true;
    }

	bool forwardRefr(Hit *hit , Photon photon, int depth , double* pr){
        Material* material = hit->getMaterial();
        double randP = material->getRefridx();
        if ( photon.currentN != 1 ) {
            Vector3f absor = photon.absorb*(-hit->getT()*photon.dir.length());
            Vector3f trans = Vector3f( exp( absor.x() ) , exp( absor.y()) , exp( absor.z()));
            double tPower = (trans.x()+trans.y()+trans.z())/3;
            randP *= tPower;
            photon.power = photon.power * trans / tPower;
        }

        if ( randP <= ran() * ( *pr ) ) {
            *pr -= randP;
            return false;
        }

        Vector3f hitNormed = hit->getNormal().normalized(), nRayed = -photon.dir.normalized();
        double tmpN;
        if (photon.currentN <= 1+EPS)   tmpN = 1/hit->getMaterial()->getN();
        else    tmpN = hit->getMaterial()->getN();

        Vector3f refrDirVer = (-nRayed - Vector3f::dot(hitNormed, -nRayed)*hitNormed)*tmpN;
        double nnt = tmpN, ddn = Vector3f::dot(-nRayed, hitNormed), cos2t=1-nnt*nnt*(1-ddn*ddn);

        if (cos2t < EPS){
            Vector3f reflDir = (2*Vector3f::dot(hitNormed, nRayed)*hitNormed - nRayed).normalized();
            photon.pos += HITPOINTOUTER*hitNormed;
            photon.dir = reflDir;
            photon.pos += HITPOINTOUTER*photon.dir;
            //cos2t = EPS;
        }
        else{
            double cosI = -Vector3f::dot(hitNormed, photon.dir);
	        Vector3f refrDir =  photon.dir * nnt + hitNormed * ( nnt * cosI - sqrt( cos2t ) );
            refrDir = refrDir.normalized();
            if (photon.currentN <= 1+EPS) photon.currentN = hit->getMaterial()->getN();
            else    photon.currentN = 1;
            photon.absorb = hit->getMaterial()->absorb;
            photon.dir = refrDir;
            photon.pos += HITPOINTOUTER*photon.dir;
        }
        forwardTracing( photon , depth + 1 );
        return true;
    }

	void forwardTracing(Photon photon, int depth){
        if ( depth > MAX_TRACING_DEPTH ) return;
        Hit hit;
        Ray pRay = Ray(photon.pos, photon.dir);
        bool isIntersect = baseGroup->intersect(pRay, hit, 0);
        if (isIntersect){
            Vector3f hitNormed = hit.getNormal().normalized(),
                     hitPoint = pRay.pointAtParameter(hit.getT());
            photon.pos = hitPoint;

            if (hit.getMaterial()->getDiffidx() > EPS){
			    map->storePhoton(photon);
            }

            double prob = 1;
            if ( !forwardDiff( &hit , photon , depth , &prob ))
                if ( !forwardRefl( &hit , photon , depth , &prob ))
                    forwardRefr( &hit , photon , depth , &prob );
        }
    }
        
    void buildMap() {
        int n = scene->emitPhoton;
        map = new PhotonMap(n, scene->maxInMap);
        double total_power = 0;
        for (int li = 0 ; li < scene->getNumLights(); ++li)
            total_power += scene->getLight(li)->getColorPower();

        double photon_power = total_power / n;
        int emited_photons = 0;
        for (int li = 0 ; li < scene->getNumLights(); ++li) {
            long iter = long(scene->getLight(li)->getColorPower()/photon_power);

            #pragma omp parallel for schedule(dynamic, 1)
            for (long Ii=0; Ii <= iter; Ii ++){
                emited_photons += 1;
                if ( ( emited_photons % 10000 ) == 0 ) std::cout << "Emited photons: " << emited_photons << std::endl;
                Photon photon = scene->getLight(li)->EmitPhoton();
                photon.power = photon.power*total_power;
                forwardTracing( photon , 1 );
            }
	    }
        std::cout << "balence tree" << std::endl;
	    map->kdTreeBalance();
        std::cout << "Map has been built!" << std::endl;
    }

    Vector3f backwardDiff( Hit *hit, Ray *r) {
        Vector3f color(0);
        if (hit->getMaterial()->texture == NULL)    color = hit->getMaterial()->getColor();
        else    color = hit->getMaterial()->getTextureColor(hit->getU(), hit->getV());
        Vector3f ret = color * scene->getBackgroundColor() * hit->getMaterial()->getDiffidx(),
                next = color * map->getIrradiance(r->pointAtParameter(hit->getT()), hit->getNormal().normalized(), scene->sample_dist, scene->sample_photons ) * hit->getMaterial()->getDiffidx();
        ret += next;
        return ret;
    }
    
    Vector3f backwardRefl( Hit *hit, Ray *r, int depth) {
        Vector3f hitNormed = hit->getNormal().normalized(), nRayed = -r->getDirection().normalized();
        Vector3f reflDir = (2*Vector3f::dot(hitNormed, nRayed)*hitNormed - nRayed).normalized();

        return backwardTracing(Ray(r->pointAtParameter(hit->getT())+HITPOINTOUTER*reflDir, reflDir), 
                depth + 1) * hit->getMaterial()->getColor() * hit->getMaterial()->getReflidx();
    }

    Vector3f backwardRefr( Hit *hit, Ray *r, int depth, double cN = 1, Vector3f cAb = Vector3f(0)) {
        Vector3f hitPoint = r->pointAtParameter(hit->getT());
        Vector3f hitNormed = hit->getNormal().normalized(), nRayed = -r->getDirection().normalized();
        double tmpN;
        if (cN <= 1+EPS)   tmpN = 1/hit->getMaterial()->getN();
        else    tmpN = hit->getMaterial()->getN();
        Vector3f refrDirVer = (-nRayed - Vector3f::dot(hitNormed, -nRayed)*hitNormed)*tmpN;
        double nnt = tmpN, ddn = Vector3f::dot(-nRayed, hitNormed), cos2t=1-nnt*nnt*(1-ddn*ddn);
        
        Vector3f dir(0), newAb=cAb;
        double newN = cN;

        if (cos2t < EPS){
            Vector3f reflDir = (2*Vector3f::dot(hitNormed, nRayed)*hitNormed - nRayed).normalized();
            dir = reflDir;
            hitPoint += HITPOINTOUTER*reflDir;
        }

        else{
            double cosI = -Vector3f::dot(hitNormed, -nRayed);
	        Vector3f refrDir =  -nRayed * nnt + hitNormed * ( nnt * cosI - sqrt( cos2t ) );
            refrDir = refrDir.normalized();
            if (newN <= 1 + EPS)  newN = hit->getMaterial()->getN();
            else newN = 1;
            newAb = hit->getMaterial()->absorb;
            dir = refrDir;
            hitPoint += HITPOINTOUTER*refrDir;
        }
        
        
        Vector3f rcol = backwardTracing( Ray(hitPoint, dir), depth + 1, newN, newAb);
        if ( cN <= 1+EPS ) return rcol * hit->getMaterial()->getRefridx();
        Vector3f absor = cAb*(hit->getT()*-r->getDirection().length());
        Vector3f trans = Vector3f( exp( absor.x() ) , exp( absor.y()) , exp( absor.z()));
        return rcol * trans * hit->getMaterial()->getRefridx();
    }

    Vector3f backwardTracing(Ray r, int depth, double currentN = 1, Vector3f cAbsorb = Vector3f(0)){
        if ( depth > MAX_TRACING_DEPTH ) return Vector3f(0);
        Vector3f ret ;
        if (depth == 1) ret = scene->getBackgroundColor();
        else ret = Vector3f(0);
        Hit hit;
        bool isIntersect = baseGroup->intersect(r, hit, 0);

        double tmpT = 1e6;
        int lIdx = -1;
        for (int li = 0 ; li < scene->getNumLights(); ++li) {
            int temp = scene->getLight(li)->isHit(r.getOrigin(),r.getDirection());
            if (temp != 0 and temp<tmpT)    {
                lIdx = li;
                tmpT = temp;
            }
        }
        if ((lIdx != -1) and (!isIntersect or hit.getT() > tmpT))
            ret += Vector3f(1);

        if ( isIntersect ) {
            if ( hit.getMaterial()->getDiffidx() > EPS ) ret += backwardDiff( &hit, &r);
            if ( hit.getMaterial()->getReflidx() > EPS ) ret += backwardRefl( &hit, &r, depth);
            if ( hit.getMaterial()->getRefridx() > EPS ) ret += backwardRefr( &hit, &r, depth, currentN, cAbsorb);
        }
        if ( depth == 1 ) ret = Vector3f( std::min( ret[0] , double(1.0) ) , std::min( ret[1] , double(1.0)) , std::min( ret[2] , double(1.0) ) );
        return ret;        
    }

    SceneParser* scene;
    Group* baseGroup;
    PhotonMap* map=NULL;
};

#endif