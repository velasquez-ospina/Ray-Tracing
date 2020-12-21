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

#include "Plane.hpp"
#include "Vect.hpp"
#include "Ray.hpp"
#include "Camera.hpp"
#include "Color.hpp"
#include "light.hpp"
#include "Source.hpp"
#include "Object.hpp"
#include "Sphere.hpp"

using namespace std;

struct RGBType{
    double r;
    double g;
    double b; 
};

void savebmp (const char *filename, int w, int h, int dpi, RGBType *data ){
    FILE *f;
    int k = w*h;
    int s= 4*k;
    int filesize = 54 +s;

    double factor = 39.375;
    int m = static_cast<int>(factor);

    int ppm = dpi*m;

    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0,0,0, 54, 0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,24,0};


    bmpfileheader[ 2] = (unsigned char)(filesize);
    bmpfileheader[ 3] = (unsigned char)(filesize>>8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);

    bmpinfoheader[ 4] = (unsigned char)(w);
    bmpinfoheader[ 5] = (unsigned char)(w>>8);
    bmpinfoheader[ 6] = (unsigned char)(w>>16);
    bmpinfoheader[ 7] = (unsigned char)(w>>24);

    bmpinfoheader[ 8] = (unsigned char)(h);
    bmpinfoheader[ 9] = (unsigned char)(h>>8);
    bmpinfoheader[ 10] = (unsigned char)(h>>16);
    bmpinfoheader[ 11] = (unsigned char)(h>>24);

    bmpinfoheader[ 21] = (unsigned char)(s);
    bmpinfoheader[ 22] = (unsigned char)(s>>8);
    bmpinfoheader[ 23] = (unsigned char)(s>>16);
    bmpinfoheader[ 24] = (unsigned char)(s>>24);

    bmpinfoheader[ 25] = (unsigned char)(ppm);
    bmpinfoheader[ 26] = (unsigned char)(ppm>>8);
    bmpinfoheader[ 27] = (unsigned char)(ppm>>16);
    bmpinfoheader[ 28] = (unsigned char)(ppm>>24);

    bmpinfoheader[ 29] = (unsigned char)(ppm);
    bmpinfoheader[ 30] = (unsigned char)(ppm>>8);
    bmpinfoheader[ 31] = (unsigned char)(ppm>>16);
    bmpinfoheader[ 32] = (unsigned char)(ppm>>24);

    f = fopen(filename,"wb");

    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);

    for (int i = 0; i<k; i++) {

        double red = (data[i].r)*255;
        double green = (data[i].g)*255;
        double blue = (data[i].b)*255;

        unsigned char color[3] = {(unsigned char)floor(blue),(unsigned char)floor(green),(unsigned char)floor(red)};

        fwrite(color,1,3,f);

    }

    //fclose(f);
}

int winningObjectIndex(vector<double> object_intersections){
    // return the index of the winning intersection
    int index_of_minimum_value;

    //prevent unnecessary calculations
    if (object_intersections.size() == 0) {
        // if there are no intersections
        return -1;
    }
    else if(object_intersections.size() == 1){
            if (object_intersections.at(0) > 0){
                //if that intersection is greater than zero then its our index of minimum value
                return 0; //remember 0 is the index, not the value
            }
            else {
                //otherwise the only intersection is negative
                return -1;
            }
    }
    else {

        //otherwise there is more than one intersection

        //ESTO SE PUEDE MEJORAR...

        //first find the maximum value
        double max=0;
        for (int i = 0; i < object_intersections.size(); i++){
            if (max < object_intersections.at(i)) {
                max = object_intersections.at(i);
            }
        }
        //then starting from the maximum value find the minimum positive value
        if(max > 0) {
            //we only want positive intersections
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

Color getColorAt(Vect intersection_position, Vect intersecting_ray_direction, vector<Object*> scene_objects, int index_of_winning_object, vector<Source*> light_sources, double accuracy, double ambientlight){
    
    Color winning_object_color = scene_objects.at(index_of_winning_object)->getColor();
    Vect winning_object_normal = scene_objects.at(index_of_winning_object)->getNormalAt(intersection_position);
    
    Color final_color = winning_object_color.colorScalar(ambientlight);

    for(int light_index = 0; light_index < light_sources.size(); light_index++){
        
        Vect light_direction = light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize();

        float cosine_angle = winning_object_normal.dotProduct(light_direction);

        if(cosine_angle > 0){
            //test for shadows
            bool shadowed = false;
            Vect distance_to_light = light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize();
           
            float distance_to_light_magnitude = distance_to_light.magnitude();
            Ray shadow_ray (intersection_position, light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize());
            
            vector<double> secondary_intersections;
            
            for (int object_index = 0; object_index<scene_objects.size() && shadowed == false; object_index++){
                secondary_intersections.push_back(scene_objects.at(object_index)->findIntersection(shadow_ray));
            } 
            //creates shadows in objects
            for (int c = 0; c < secondary_intersections.size(); c++){
                if (secondary_intersections.at(c) > accuracy){
                    if (secondary_intersections.at(c) <= distance_to_light_magnitude){
                        shadowed = true;
                    }
                }
                break;
            }

            if(shadowed == false){
                final_color = final_color.colorAdd(winning_object_color.colorMultiply(light_sources.at(light_index)->getLightColor()).colorScalar(cosine_angle));

                if (winning_object_color.getColorSpecial() > 0 && winning_object_color.getColorSpecial() <= 1){
                    //special  [0-1]
                    double dot1 = winning_object_normal.dotProduct(intersecting_ray_direction.negative());
                    Vect scalar1 = winning_object_normal.vectMul(dot1);
                    Vect add1 = scalar1.vectAdd(intersecting_ray_direction);
                    Vect scalar2 = add1.vectMul(2);
                    Vect add2 = intersecting_ray_direction.negative().vectAdd(scalar2);
                    Vect reflection_direction = add2.normalize();
                    double specular = reflection_direction.dotProduct(light_direction);
                    if (specular > 0){
                        specular = pow(specular, 10);
                        final_color = final_color.colorAdd(light_sources.at(light_index)->getLightColor().colorScalar(specular*winning_object_color.getColorSpecial()));
                    }
                }
            }
        }
    }

    return final_color;
}

int thisone;

int main(int argc, char *argv[]){

    int Width=640, Height=480, dpi = 72;
    int n = Width*Height;
    double aspect_ratio= double(Width)/(double)Height;
    double ambientlight = 0.2;
    double accuracy = 0.000001;
    RGBType *pixels = new RGBType[n];

    //coordinate system
    Vect O (0,0,0);
    Vect X (1,0,0);
    Vect Y (0,1,0);
    Vect Z (0,0,1);



    //Camera deffinition
    Vect campos (3, 1.5, -4);
    Vect look_at (0,0,0);
    Vect diff_btw (campos.getVectX() - look_at.getVectX(),campos.getVectY() - look_at.getVectY(),campos.getVectZ()-look_at.getVectZ());
    Vect camdir = diff_btw.negative().normalize();
    Vect camright = Y.crossproduct(camdir).normalize();
    Vect camdown = camright.crossproduct(camdir);

    Camera scene_cam (campos, camdir, camright, camdown);
 
    //Colors
    Color white_light (1.0,1.0,1.0,0);
    Color pretty_green (0.5,1,0.5,0.3);
    Color gray (0.5, 0.5, 0.5, 0);
    Color black (0,0,0,0);
    Color maroon (0.5, 0.25, 0,0);

    //Light definition
    Vect light_position (-7,10,-10);
    light scene_light (light_position, white_light);
    vector <Source*> light_sources;
    light_sources.push_back(dynamic_cast<Source*>(&scene_light));

    //scene objects

    Sphere scene_sphere (Y, 1, pretty_green);
    Plane scene_plane (Y, 0, maroon);

    //vector to store the objects of the scene nad loop through them
    vector<Object*> scene_objects;

    //this is how we add objects to the vector
    scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere));
    scene_objects.push_back(dynamic_cast<Object*>(&scene_plane));

    double xamnt, yamnt;    //this variables are to allow the rays to go to the sides where the camera is pointing
    // Nested loops to give each pixel a color
    for (int x = 0; x < Width; x++)     
    {
        for (int y = 0; y < Height; y++)
        {
            thisone = y*Width + x;      //determine coordenates of an individual pixel

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

            Vect cam_ray_origin = scene_cam.getCameraPosition();    //ray's origin is the same of camera's
            Vect cam_ray_direction = camdir.vectAdd(camright.vectMul(xamnt - 0.5).vectAdd(camdown.vectMul(yamnt - 0.5))).normalize();

            Ray cam_ray (cam_ray_origin, cam_ray_direction);
            //cam_ray.ShowMe();
            vector<double> intersections;   //vector that stores the intersections
            //Loop through each of the objects in our scene and determine if there are any intersections
            for (int index = 0; index < scene_objects.size(); index++){
                intersections.push_back(scene_objects.at(index)->findIntersection(cam_ray));    //Asks if there are any intersections between the objects and the ray and stores it
                //cout << scene_objects.at(index)->findIntersection(cam_ray) << endl;
            }

            //now we need to find the closest object to the camera
            int index_of_winning_object = winningObjectIndex(intersections);
            //cout << index_of_winning_object << " ";

            if (index_of_winning_object == -1) {
                //if the ray miss, put the color of the background which is black
                pixels[thisone].r = 0;
                pixels[thisone].g = 0;
                pixels[thisone].b = 0;
            }
            else {
                 //index coresponds to an object in our scene
                if (intersections.at(index_of_winning_object)>accuracy){
                    Vect intersection_position = cam_ray_origin.vectAdd( cam_ray_direction.vectMul(intersections.at( index_of_winning_object ) ));

                    Vect intersecting_ray_direction = cam_ray_direction;

                    Color intersection_color = getColorAt(intersection_position, intersecting_ray_direction, scene_objects, index_of_winning_object, light_sources, accuracy, ambientlight);
                    
                    pixels[thisone].r = intersection_color.getColorRed();
                    pixels[thisone].g = intersection_color.getColorGreen();
                    pixels[thisone].b = intersection_color.getColorBlue();
                }
            }
        }
        
    }
    
    //Save the image as "Scene.bmp"
    savebmp("Scene.bmp",Width,Height,dpi,pixels);

    return 0;
}
