#ifndef _OBJECT_HPP
#define _OBJECT_HPP

#include <iostream>
#include "Vect.hpp"
#include "Ray.hpp"
#include "Color.hpp"



class Object{

    public:

        //Constructors
        Object(){}
        ~Object(){}

        //Method functions
        virtual Color getColor () {return Color ( 0.0, 0.0, 0.0, 0);} // this is meant for shadows
        virtual Vect getNormalAt(Vect intersection_position){
            return Vect (0,0,0);
        }       
        virtual double findIntersection(Ray ray) = 0;
        virtual void ShowMe() = 0;

};
#endif //_OBJECT_HPP