#ifndef _RAY_TRACER_HPP
#define _RAY_TRACER_HPP


#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>
#include <chrono>

#include "Plane.hpp"
#include "Vect.hpp"
#include "Ray.hpp"
#include "Camera.hpp"
#include "Color.hpp"
#include "light.hpp"
#include "Source.hpp"
#include "Object.hpp"
#include "Sphere.hpp"
#include "json11.hpp"
#include "argh.hpp"
#include "fileManager.hpp"

using namespace std;
using namespace json11;

/** return the index of the closest intersection. Returns -1 if there are no intersections
 *  @param object_intersections the vector with all the intersections found for that pixel
 */
int findClosestIntersectionIndex(vector<double> object_intersections){

    int index_of_minimum_value;

    //prevent unnecessary calculations
    if (object_intersections.size() == 0) {
        // if there are no intersections
        return -1;
    }
    else if(object_intersections.size() == 1){
            if (object_intersections.at(0) > 0){
                //if greater than zero then its the index of minimum value
                return 0;
            }
            else {
                //otherwise the only intersection is negative
                return -1;
            }
    }
    else {

        //otherwise there is more than one intersection
        //first find the maximum value
        double max=0;
        for (int i = 0; i < object_intersections.size(); i++){
            if (max < object_intersections.at(i)) {
                max = object_intersections.at(i);
            }
        }
        //then starting from the maximum value find the minimum positive value
        if(max > 0) {
            //only positive intersections
            index_of_minimum_value = 0;
            for (int index = 0; index < object_intersections.size(); index++){
                
                if (object_intersections.at(index) <= max && object_intersections.at(index)>0) {
                
                    max = object_intersections.at(index);
                    index_of_minimum_value = index;

                }
            }
            return index_of_minimum_value;
        }
        else {
            //all the intersections were negative

            return -1;
        }
    }

}

/** Calculate the color of the pixel, taking in count reflections and shadows
 *  @param intersectionPoint Vector with the position of the intersection within the ray and the object
 *  @param intersectionRayDirection Direction of the ray 
 *  @param sceneObjects Vector with the objects in the scene
 *  @param closestObjectIndex index of the position where is the closest object in the objects's vector
 *  @param lights vector with the all the light sources
 *  @param accuracy 
 *  @param ambientlight scalar that determines the intensity of the light in the scene
 *  @return the color of the pixel
 * 
 */
Color calculateColor(Vect intersectionPoint, Vect intersectingRayDirection, vector<Object*> sceneObjects, int closestObjectIndex, vector<Source*> lights, double accuracy, double ambientlight){
    
    Color closestObjectColor = sceneObjects.at(closestObjectIndex)->getColor();
    Vect closestObjectNormal = sceneObjects.at(closestObjectIndex)->getNormalAt(intersectionPoint);
    
    //If the color has the special atribute 2 the color is a black and white floor pattern
    if (closestObjectColor.getColorSpecial() == 2){
        //checkered/tile floor pattern

        int square = (int)floor(intersectionPoint.getVectX()) + (int)floor(intersectionPoint.getVectZ()); 

        if (square % 2 == 0) {
            //black tile
            closestObjectColor.setColorRed(0);
            closestObjectColor.setColorGreen(0);
            closestObjectColor.setColorBlue(0);
        }
        else{
            closestObjectColor.setColorRed(1);
            closestObjectColor.setColorGreen(1);
            closestObjectColor.setColorBlue(1); 
        }
    }

    Color finalColor = closestObjectColor.colorScalar(ambientlight);

    if (closestObjectColor.getColorSpecial() > 0 && closestObjectColor.getColorSpecial() <= 1){
        // reflection from objects with specular intensity

        double dot1 = closestObjectNormal.dotProduct(intersectingRayDirection.negative());
        Vect scalar1 = closestObjectNormal.vectMul(dot1);
        Vect add1 = scalar1.vectAdd(intersectingRayDirection);
        Vect scalar2 = add1.vectMul(2);
        Vect add2 = intersectingRayDirection.negative().vectAdd(scalar2);
        Vect reflection_direction = add2.normalize();

        Ray reflectedRay (intersectionPoint, reflection_direction);

        // determine what the ray intersects with first

        vector<double> reflectionIntersections;

        for (int refletIndex = 0; refletIndex < sceneObjects.size(); refletIndex++){
            reflectionIntersections.push_back(sceneObjects.at(refletIndex)->findIntersection(reflectedRay));
        }

        int indexOfFirstObject = findClosestIntersectionIndex(reflectionIntersections);

        if (indexOfFirstObject != -1) {
            //if the reflection rays didn't miss
            if (reflectionIntersections.at(indexOfFirstObject) > accuracy){
                 //determine the position and direction at the point of intersection with the reflection ray
                 //the ray only affects the color of it reflected off something

                 Vect reflectionIntersectionPoint = intersectionPoint.vectAdd(reflection_direction.vectMul(reflectionIntersections.at(indexOfFirstObject)));
                 Vect reflectionIntersectionDirection = reflection_direction;

                 Color reflection_intersectionColor = calculateColor(reflectionIntersectionPoint, reflectionIntersectionDirection, sceneObjects, indexOfFirstObject, lights, accuracy, ambientlight);
                 //we can get reflexions in the reflexions... recursive calls... more calcul time

                 finalColor = finalColor.colorAdd(reflection_intersectionColor.colorScalar(closestObjectColor.getColorSpecial()));
            }
        }

    }

    for(int lightIndex = 0; lightIndex < lights.size(); lightIndex++){
        Vect light_direction = lights.at(lightIndex)->getLightPosition().vectAdd(intersectionPoint.negative()).normalize();


        float cosine_angle = closestObjectNormal.dotProduct(light_direction);

        if(cosine_angle > 0){
            bool shadowed = false;
            Vect distance_to_light = lights.at(lightIndex)->getLightPosition().vectAdd(intersectionPoint.negative()).normalize();
            float distance_to_light_magnitude = distance_to_light.magnitude();
            Ray shadow_ray (intersectionPoint, lights.at(lightIndex)->getLightPosition().vectAdd(intersectionPoint.negative()).normalize());
            vector<double> secondary_intersections;
            for (int object_index = 0; object_index<sceneObjects.size() && shadowed == false; object_index++){
                secondary_intersections.push_back(sceneObjects.at(object_index)->findIntersection(shadow_ray));
            } 

            for (int c = 0; c < secondary_intersections.size(); c++){
                if (secondary_intersections.at(c) > accuracy){
                    if (secondary_intersections.at(c) <= distance_to_light_magnitude){
                        shadowed = true;
                    }
                }
                break;
            }

            if(shadowed == false){
                finalColor = finalColor.colorAdd(closestObjectColor.colorMultiply(lights.at(lightIndex)->getLightColor()).colorScalar(cosine_angle));

                if (closestObjectColor.getColorSpecial() > 0 && closestObjectColor.getColorSpecial() <= 1){

                    //calculate the reflection ray's direction

                    double dot1 = closestObjectNormal.dotProduct(intersectingRayDirection.negative());
                    Vect scalar1 = closestObjectNormal.vectMul(dot1);
                    Vect add1 = scalar1.vectAdd(intersectingRayDirection);
                    Vect scalar2 = add1.vectMul(2);
                    Vect add2 = intersectingRayDirection.negative().vectAdd(scalar2);
                    Vect reflection_direction = add2.normalize();
                    double specular = reflection_direction.dotProduct(light_direction);
                    if (specular > 0){
                        specular = pow(specular, 10);
                        finalColor = finalColor.colorAdd(lights.at(lightIndex)->getLightColor().colorScalar(specular*closestObjectColor.getColorSpecial()));
                    }
                }
            }
        }
    }
    if(finalColor.getColorBlue() >1.0 || finalColor.getColorGreen() >1.0 || finalColor.getColorRed() >1.0){
        finalColor.setColorRed(1.0);
        finalColor.setColorGreen(1.0);
        finalColor.setColorBlue(1.0);
    }
    return finalColor;
}


/**
 * Returns an array with the color of each pixel
 *  @param Width the width of the image
 *  @param Height the height of the image
 *  @param aaadepth the anti-aliasing depth
 *  @param accuracy 
 *  @param firstRow the first row of the pixels that the process must calculate
 *  @param endRow the end row of the pixels that the process must calculate
 *  @param path the path of the .json with the scene
 *  @return the array with all the pixels
 */
RGBType* calculatePixels(int Width, int Height,int aadepth, int accuracy, int firstRow,int endRow, string path){
    int n = Width*Height;
    float aspect_ratio= float(Width)/(float)Height;
    RGBType *pixels = new RGBType[n];
    double ambientlight = 0.4;

    

    int thisone, aa_index;
    double xamnt, yamnt;    //this variables are to allow the rays to go to the sides where the camera is pointing
    double tempRed, tempGreen, tempBlue;

    //coordinate system
    Vect O (0,0,0);
    Vect X (1,0,0);
    Vect Y (0,1,0);
    Vect Z (0,0,1);
 

    vector <Source*> lights;
    vector<Object*> sceneObjects;
    Scene S = jsonReader(path);
    lights = S.lights;
    sceneObjects = S.sceneObjects;
    Camera *scene_cam = S.scene_camera;
    Vect camdir = scene_cam->getCameraDirection();
    Vect camdown = scene_cam->getCameraDown();
    Vect camright = scene_cam->getcameraRight();
    Vect campos = scene_cam->getCameraPosition();

    for (int y = firstRow; y < endRow; y++)
    {

        for (int x = 0; x < Width; x++)     
        {
            thisone = (y - firstRow)*Width + x;      //determine coordenates of an individual pixel

            // start with a blank pixel
            double tempRed[aadepth*aadepth];
            double tempGreen[aadepth*aadepth];
            double tempBlue[aadepth*aadepth];



            for (int aax = 0; aax < aadepth; aax++)
            {
                for (int aay = 0; aay< aadepth; aay++)
                {

                    aa_index = aay*aadepth + aax;   //similar to thisone

                    srand(time(0));
                    //create the ray from the camera to this pixel
                    if(aadepth == 1){

                    //start with no anti-aliasing
                    
                        if (Width > Height){
                            // the image is wider than it is tall
                            xamnt = ((x+0.5)/Width)*aspect_ratio - (((Width-Height)/(double)Height)/2);
                            yamnt = ((Height - y) + 0.5)/Height;
                        }
                        else if (Height > Width){
                            // the image is taller than it is wide
                            xamnt = (x + 0.5)/Width;
                            yamnt = (((Height - y) + 0.5)/Height)/aspect_ratio - (((Height - Width)/(double)Width)/2);
                        }
                        else {
                            // the image is square
                            xamnt = (x + 0.5)/Width;
                            yamnt = ((Height - y) + 0.5)/Height;
                        }
                    }else{
                        //anti-aliasing
                        if (Width > Height){
                            // the image is wider than it is tall
                            xamnt = ((x + (double)aax/((double)aadepth -1))/Width)*aspect_ratio - (((Width-Height)/(double)Height)/2);
                            yamnt = ((Height - y) + (double)aax/((double)aadepth -1))/Height;
                        }
                        else if (Height > Width){
                            // the image is taller than it is wide
                            xamnt = (x + (double)aax/((double)aadepth -1))/Width;
                            yamnt = (((Height - y) + (double)aax/((double)aadepth -1))/Height)/aspect_ratio - (((Height - Width)/(double)Width)/2);
                        }
                        else {
                            // the image is square
                            xamnt = (x + (double)aax/((double)aadepth -1))/Width;
                        }

                    }
                    Vect cameraRayOrigin = scene_cam->getCameraPosition();    //ray's origin is the same of camera's
                    Vect cameraRayDirection = camdir.vectAdd(camright.vectMul(xamnt - 0.5).vectAdd(camdown.vectMul(yamnt - 0.5))).normalize();

                    Ray cam_ray (cameraRayOrigin, cameraRayDirection);
                    vector<double> intersections;   //vector that stores the intersections
                    //Loop through each of the objects in our scene and determine if there are any intersections
                    for (int index = 0; index < sceneObjects.size(); index++){
                        intersections.push_back(sceneObjects.at(index)->findIntersection(cam_ray));    //Asks if there are any intersections between the objects and the ray and stores it
                    }

                    //now we need to find the closest object to the camera
                    int closestObjectIndex = findClosestIntersectionIndex(intersections);

                    if (closestObjectIndex == -1) {
                        //if the ray miss, put the color of the background which is black
                        tempRed[aa_index] = 0;
                        tempGreen[aa_index] = 0;
                        tempBlue[aa_index] = 0;
                    }
                    else {
                        //index coresponds to an object in our scene
                        if (intersections.at(closestObjectIndex)>accuracy){
                            Vect intersectionPoint = cameraRayOrigin.vectAdd( cameraRayDirection.vectMul(intersections.at( closestObjectIndex ) ));

                            Vect intersectingRayDirection = cameraRayDirection;

                            Color intersectionColor = calculateColor(intersectionPoint, intersectingRayDirection, sceneObjects, closestObjectIndex, lights, accuracy, ambientlight);
                            
                            tempRed[aa_index] = intersectionColor.getColorRed();
                            tempGreen[aa_index] = intersectionColor.getColorGreen();
                            tempBlue[aa_index] = intersectionColor.getColorBlue();
                        }
                    }
                }
                
            }

            //average the pixel color
            double totalred = 0;
            double totalgreen = 0;
            double totalblue = 0;

            for(int ired = 0; ired < aadepth*aadepth; ired++)
            {
                totalred = totalred + tempRed[ired];
            }
            for(int igreen = 0; igreen < aadepth*aadepth; igreen++)
            {
                totalgreen = totalgreen + tempGreen[igreen];
            }
            for(int iblue = 0; iblue < aadepth*aadepth; iblue++)
            {
                totalblue = totalblue + tempGreen[iblue];
            }

            double avgRed = totalred/(aadepth*aadepth);
            double avggreen = totalgreen/(aadepth*aadepth);
            double avgblue = totalblue/(aadepth*aadepth);

            pixels[thisone].r = avgRed;
            pixels[thisone].g = avggreen;
            pixels[thisone].b = avgblue;
        }
    }
    return pixels;
}

#endif //_RAY_TRACER_HPP