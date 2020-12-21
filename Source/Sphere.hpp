#ifndef _SPHERE_HPP
#define _SPHERE_HPP

#include <iostream>
#include "Vect.hpp"
#include "Color.hpp"
#include "Object.hpp"



class Sphere: public Object{

    private:

        Vect center;
        double radius;
        Color color;

    public:

        //Constructors
        Sphere():center(0,0,0), radius(1), color(0.5,0.5,0.5,0){}
        Sphere(Vect aCenter):center(aCenter), radius(1.0), color(0.5,0.5,0.5,0){}
        Sphere(Vect aCenter, double aRadius):center(aCenter), radius(aRadius), color(1,1,1,0){}
        Sphere(Vect aCenter, double aRadius, Color aColor):center(aCenter), radius(aRadius), color(aColor){}
        ~Sphere(){}

        //Get functions
        Vect getSphereCenter(){return center;}
        double getSphereRadius(){return radius;}
        Color getColor(){return color;}



        //Method functions
        Vect getNormalAt(Vect point){
            //Remember normal always points away from the center of a sphere
            Vect normal_vect = point.vectAdd(center.negative()).normalize();
            return normal_vect;  
        }


        double findIntersection(Ray ray) {
            //returns a scalar value which is the distance between the origin of the ray to the point of intersection
            //So later we can use linear algebra to find the coordenates of the point of intersection and get the normal of the surface of the sphere in that coordenate

            //Estas variables se deberían quitar en un futuro, son redundantes
            Vect ray_origin = ray.getRayOrigin();
            double ray_origin_x = ray_origin.getVectX();
            double ray_origin_y = ray_origin.getVectY();
            double ray_origin_z = ray_origin.getVectZ();
  
            Vect ray_direction = ray.getRayDirection();
            double ray_direction_x = ray_direction.getVectX();
            double ray_direction_y = ray_direction.getVectY();
            double ray_direction_z = ray_direction.getVectZ();

            Vect sphere_center = center;
            double sphere_center_x = center.getVectX();
            double sphere_center_y = center.getVectY();
            double sphere_center_z = center.getVectZ();

            double a= 1;    //normalized
            double b = (2*(ray_origin_x - sphere_center_x)*ray_direction_x) + (2*(ray_origin_y - sphere_center_y)*ray_direction_y) + (2*(ray_origin_z - sphere_center_z)*ray_direction_z);
            double c = pow(ray_origin_x - sphere_center_x, 2) + pow(ray_origin_y - sphere_center_y, 2) + pow(ray_origin_z - sphere_center_z, 2) - (radius*radius);

            double discriminant = b*b - 4*c; //linear algebra

            if (discriminant > 0) {
                //the ray intersects the sphere, but it can do it from two sides

                // the first root
                double root_one = ((-1*b -sqrt(discriminant))/2) - 0.000001;        //the 0.0001 is arbitrary, helps with the accuaracy errors

                if (root_one > 0){
                    // the first root is the smallest positive root
                    return root_one;
                }
                else {
                    //the second root is the smallest positive root
                    double root_two = ((sqrt(discriminant) - b)/2) - 0.000001;
                    return root_two;
                }
            }
            else {
                //the ray missed the sphere
                return -1;
            }
        }
        void ShowMe(){
            std::cout << "This is the sphere in: ( " << center.getVectX() << "," << center.getVectY() << "," << center.getVectZ() << " ) with radius : " << radius << std::endl;
        }

};


#endif //_SPHERE_HPP