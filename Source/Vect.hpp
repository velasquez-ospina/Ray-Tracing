#ifndef _VECT_HPP
#define _VECT_HPP

#include <iostream>
#include <cmath>
class Vect{

    private:

        double x, y, z;

    public:

        //Constructors
        Vect():x(0), y(0), z(0){}
        Vect(double px): x(px), y(0), z(0){}
        Vect(double px, double py): x(px), y(py), z(0){}
        Vect(double px, double py, double pz): x(px), y(py), z(pz){}
        ~Vect(){}

        //get functions
        double getVectX() {return x;}
        double getVectY() {return y;}
        double getVectZ() {return z;}

        //Method functions
        double magnitude (){ return sqrt(x*x + y*y + z*z);}
        Vect normalize () {
            double magnitude = sqrt(x*x + y*y + z*z);
            return Vect (x/magnitude,y/magnitude,z/magnitude);
        }
        Vect negative () { return Vect (-x, -y, -z);}

        double dotProduct(Vect v) {
            return x*v.getVectX() + y*v.getVectY() + z*v.getVectZ();
        }
        Vect crossproduct(Vect v){
            return Vect (y*v.getVectZ()- z*v.getVectY(), - x*v.getVectZ() + z*v.getVectX(), x*v.getVectY() - y*v.getVectX());   
        }
        Vect vectAdd (Vect v) {
            return Vect (x + v.getVectX(), y + v.getVectY(), z + v.getVectZ() );
        }
        Vect vectMul (double scalar) {
            return Vect(x*scalar, y*scalar, z*scalar);
        }
};

#endif // _VECT_HPP