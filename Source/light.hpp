#ifndef _LIGHT_HPP
#define _LIGHT_HPP

#include <iostream>
#include "Vect.hpp"
#include "Source.hpp"
#include "Color.hpp"

class light : public Source{

    private:

        Vect position;
        Color color;

    public:

        //Constructors
        light():position(0,0,0), color(1,1,1,0){}
        light(Vect aposition): position(aposition), color(1,1,1,0){}
        light(Vect aposition, Color acolor): position(aposition), color(acolor){}
        ~light(){}
        
        //Method functions
        virtual Vect getLightPosition() {return position;}
        virtual Color getLightColor() {return color;}

};
#endif // _LIGHT_HPP