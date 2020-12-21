#ifndef _RAY_HPP
#define _RAY_HPP

#include <iostream>
#include "Vect.hpp"
class Ray{
    
    private:

        Vect origin, direction;

    public:

        //Constructors
        Ray():origin(0,0,0), direction(1,0,0){}
        Ray(Vect adirection): origin(0,0,0), direction(adirection){}
        Ray(Vect anorigin, Vect adirection): origin(anorigin), direction(adirection){}
        ~Ray(){}

        //Get functions
        Vect getRayOrigin() {return origin;}
        Vect getRayDirection() {return direction;}

        //Method functions

        void ShowMe(){
            std::cout << "This is the sphere in: ( " << origin.getVectX() << "," << origin.getVectY() << "," << origin.getVectZ()
             << " ) with the direction : " << direction.getVectX() << "," << direction.getVectY() << "," << direction.getVectZ() << std::endl;
        }




};
#endif //_RAY_HPP