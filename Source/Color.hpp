#ifndef _COLOR_HPP
#define _COLOR_HPP

#include <iostream>


class Color{

    private:

        double red, green, blue, special; //the special value will represent reflectiveness of the surface

    public:

    //Constructors
    Color():red(0.5),green(0.5),blue(0.5){}
    Color(double ared, double agreen, double ablue, double aspecial):red(ared), green(agreen), blue(ablue), special(aspecial){}

    //Get functions
    double getColorRed() {return red;}
    double getColorGreen() {return green;}
    double getColorBlue() {return blue;}
    double getColorSpecial() {return special;}

    //Set functions
    double setColorRed(double ared) {red = ared;}
    double setColorGreen(double agreen) {green = agreen;}
    double setColorBlue(double ablue) {blue = ablue;}
    double setColorSpecial(double aspecial) {special = aspecial;}

    double brightness(){
        return (red +  green + blue)/3;
    }

    //scales color values by a factor
    Color colorScalar (double scalar){
        return Color (red*scalar, green*scalar, blue*scalar, special);
    }

    //combines two colors together
    Color colorAdd(Color color){
        return Color(red + color.getColorRed(), green + color.getColorGreen(), blue + color.getColorBlue(), special);
    }

    Color colorMultiply (Color color){
        return Color(red * color.getColorRed()/2, green * color.getColorGreen()/2, blue*color.getColorBlue()/2, special);    
    }

    Color colorAverage(Color color){
        return Color(red + color.getColorRed()/2, green + color.getColorGreen()/2, blue+color.getColorBlue()/2, special);
    }
};
#endif //COLOR_HPP