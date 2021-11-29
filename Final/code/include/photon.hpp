#ifndef PHOTON_H
#define PHOTON_H

#include <Vector3f.h>

#define ran() ( double( rand() % RAND_MAX ) / RAND_MAX )



// Basic photon class
class Photon {
public:
	Vector3f pos, dir, power, absorb;
    double currentN;
	int plane;
    Photon(){
        pos = Vector3f(0), dir = Vector3f(0), power = Vector3f(0);
        absorb = Vector3f(0);
        currentN = 1;
    }
    Photon(Vector3f &pos_, Vector3f &dir_, Vector3f &power_, int plane_=0, double currentN_ = 1, Vector3f absorb_ = Vector3f(0)):
        pos(pos_), dir(dir_), power(power_), plane(plane_), currentN(currentN_), absorb(absorb_) {}
};

class PhotonBeenFound {
public:
	Vector3f pos;
	int maxToFound;
    int foundNum;

	bool heapDone;
	double* squreDis;
	Photon** photons;

	PhotonBeenFound(){
        maxToFound = foundNum = 0;
        heapDone = false;
        squreDis = NULL;
        photons = NULL;
    }
	~PhotonBeenFound(){
        delete[] squreDis;
	    delete[] photons;
    }
};

class PhotonMap {
public:
    PhotonMap(int numPhotons, int size){
        emit_photons = numPhotons;
        max_photons = size;
        stored_photons = 0;
	    photons = new Photon[size + 1];
	    box_min = Vector3f( INFINITY , INFINITY , INFINITY );
	    box_max = Vector3f( -INFINITY , -INFINITY , -INFINITY );
    }
	
    void updateBox(Photon photon){
        box_min = Vector3f( std::min( box_min.x(), photon.pos.x()), std::min( box_min.y(), photon.pos.y()),
                            std::min( box_min.z(), photon.pos.z()));
        box_max = Vector3f( std::max( box_max.x(), photon.pos.x()), std::max( box_max.y(), photon.pos.y()),
                            std::max( box_max.z(), photon.pos.z()));
    }

	//int GetStoredPhotons() { return stored_photons; }
    void midCut( Photon* porg , int start , int end , int medium , int axisPlane ) {
        int l = start , r = end;
        while ( l < r ) {
            double key = porg[r].pos[axisPlane];
            int i = l - 1 , j = r;
            for ( ; ; ) {
                while ( porg[++i].pos[axisPlane] < key );
                while ( porg[--j].pos[axisPlane] > key && j > l );
                if ( i >= j ) break;
                std::swap( porg[i] , porg[j] );
            }

            std::swap( porg[i] , porg[r] );
            if ( i >= medium ) r = i - 1;
            if ( i <= medium ) l = i + 1;
        }
    }
    
    void storePhoton(Photon photon){
        if ( stored_photons >= max_photons ) return;
        photons[stored_photons] = photon;
        stored_photons += 1;
        updateBox(photon);
    }

    void partBalence (Photon* pCopy , int index , int start , int end ) {
        if ( start == end ) {
            photons[index] = pCopy[start];
            return ;
        }
        
        int planeAxis;
        if (box_max.x() - box_min.x() > box_max.y() - box_min.y() && box_max.x() - box_min.x() > box_max.z() - box_min.z())
            planeAxis = 0; 
        else if ( box_max.y() - box_min.y() > box_max.z() - box_min.z() ) planeAxis = 1;
        else    planeAxis = 2;
        
        int medium = 1;
        while ( 4 * medium <= end - start + 1 ) 
            medium <<= 1;
        if ( 3 * medium <= end - start + 1 ) medium = medium * 2 + start - 1;
            else medium = end + 1 - medium;

        midCut( pCopy , start , end , medium , planeAxis );
        photons[index] = pCopy[medium];
        photons[index].plane = planeAxis;

        if ( start < medium ) {
            double maxRecord = box_max[planeAxis];
            box_max[planeAxis] = photons[index].pos[planeAxis];
            partBalence( pCopy , index * 2 , start , medium - 1 );
            box_max[planeAxis] = maxRecord;
        }

        if ( medium < end ) {
            double minRecord = box_min[planeAxis];
            box_min[planeAxis] = photons[index].pos[planeAxis];
            partBalence( pCopy , index * 2 + 1 , medium + 1 , end );
            box_min[planeAxis] = minRecord;
        }
    }

    void kdTreeBalance(){
        Photon* photon_copy = new Photon[stored_photons + 1];
        for ( int i = 0 ; i <= stored_photons ; i++ )
            photon_copy[i] = photons[i];
        partBalence( photon_copy , 1 , 1 , stored_photons );
        delete[] photon_copy;
    }

    Vector3f getIrradiance( Vector3f hitPoint, Vector3f hitNorm , double max_dist , int toFound ) {
        Vector3f ret(0);

        PhotonBeenFound np;
        np.pos = hitPoint;
        np.maxToFound = toFound;
        np.squreDis = new double[toFound + 1];
        np.photons = new Photon*[toFound + 1];
        np.squreDis[0] = max_dist * max_dist;

        findPhoton( &np , 1 );
        if ( np.foundNum <= 8 ) return Vector3f(0);

        for ( int i = 1 ; i <= np.foundNum ; i++ )
            if ( Vector3f::dot(hitNorm, np.photons[i]->dir) < 0 ) ret += np.photons[i]->power;
        
        ret *=  4 / (emit_photons * np.squreDis[0]);
        return ret;
    }

    void findPhoton( PhotonBeenFound* np , int index ) {
        if ( index > stored_photons ) return;
        Photon *photon = &photons[index];

        if ( index * 2 <= stored_photons ) {
            double dist = np->pos[photon->plane] - photon->pos[photon->plane];
            if ( dist >= 0 ) {
                findPhoton( np , index * 2 + 1 );
                if ( dist * dist < np->squreDis[0] ) 
                    findPhoton( np , index * 2 );
            }
            else {
                findPhoton( np , index * 2 );
                if ( dist * dist < np->squreDis[0] ) 
                    findPhoton( np , index * 2 + 1 );
            } 
        }

        double squreDis = (photon->pos - np->pos).squaredLength();
        if ( squreDis > np->squreDis[0] ) return;

        if ( np->foundNum < np->maxToFound ) {
            np->foundNum++;
            np->squreDis[np->foundNum] = squreDis;
            np->photons[np->foundNum] = photon;
        } 
        else {
            if ( np->heapDone == false ) {
                for ( int i = np->foundNum >> 1 ; i >= 1 ; i--) {
                    int tmp_idx = i;
                    Photon* tmp_photon = np->photons[i];
                    double tmp_squreDis = np->squreDis[i];
                    while ( ( tmp_idx << 1 ) <= np->foundNum ) {
                        int j = tmp_idx << 1;
                        if ( j + 1 <= np->foundNum && np->squreDis[j] < np->squreDis[j + 1] ) j++;
                        if ( tmp_squreDis >= np->squreDis[j] ) break;
                        
                        np->photons[tmp_idx] = np->photons[j];
                        np->squreDis[tmp_idx] = np->squreDis[j];
                        tmp_idx = j;
                    }
                    np->photons[tmp_idx] = tmp_photon;
                    np->squreDis[tmp_idx] = tmp_squreDis;
                }
                np->heapDone = true;
            }

            int tmp_idx = 1;
            while ( ( tmp_idx << 1 ) <= np->foundNum ) {
                int j = tmp_idx << 1;
                if ( j + 1 <= np->foundNum && np->squreDis[j] < np->squreDis[j + 1] ) j++;
                if ( squreDis > np->squreDis[j] ) break;

                np->photons[tmp_idx] = np->photons[j];
                np->squreDis[tmp_idx] = np->squreDis[j];
                tmp_idx = j;
            }
            np->photons[tmp_idx] = photon;
            np->squreDis[tmp_idx] = squreDis;
            np->squreDis[0] = np->squreDis[1];
        }
    }

    int max_photons , stored_photons , emit_photons;
	Photon* photons;
	Vector3f box_min , box_max;
};

#endif