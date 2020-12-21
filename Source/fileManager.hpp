#ifndef _FILE_MANAGER_HPP
#define _FILE_MANAGER_HPP

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

using namespace std;
using namespace json11;


/**
 * Structure qui contient les trois parametres d'un RGB
 * r = rouge
 * g = green
 * b = blue
 */
struct RGBType{
    double r;
    double g;
    double b; 
};
/**
 * Gets the string of the .json with the 3 parameters of a vector and creates a vector with that information.
 * @param str String with the information of the vector.
 */
Vect stringToVector(string str)
{
	stringstream ss(str);
	double x, y, z;
	ss >> x >> y >> z;
	Vect v(x, y, z);
	return v;
}
/**
 * Gets the string of the .json with the information of the color of an object or a light source and tournes it into a color.
 * @param str String with the information of the color
 */
Color stringToColor(string str)
{
	stringstream ss(str);
	double r, g, b, s;
	ss >> r >> g >> b >> s;
	Color color = {(double)r, (double)g, (double)b, (double)s};
	return color;
}

/**
 * Class that can save the information of the camera, lights and objects found in the .json.
 */
class Scene{
    public:
    std::vector <Source*> lights;
    std::vector <Object*> sceneObjects;
    Camera *scene_camera; 

};
/** 
 * Reads the .json with the scene and determines all the objects, light sources and the camera.
 */
Scene jsonReader (string path){
    std::vector <Source*> lights;
    std::vector <Object*> sceneObjects;
    Scene S;

    lights.clear();
    sceneObjects.clear();
    
    ifstream t(path);
	string str((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
	string err;
	Json scene_json = Json::parse(str, err);
    //read array of lights
	Json::array lights_json = scene_json["lights"].array_items();
	for (size_t i = 0; i < lights_json.size(); i++){
        
		Vect position = stringToVector(lights_json[i]["position"].string_value());
		Color color = stringToColor(lights_json[i]["color"].string_value());

		string type = lights_json[i]["type"].string_value();
		light *L = new light (position, color);

		lights.push_back(dynamic_cast<Source*>(L));
	}

    //reads array of objects
    Json::array objects_json = scene_json["objects"].array_items();
	//std::cout <<"num de objetos : " << objects_json.size() << '\n';
    for (size_t i = 0; i < objects_json.size(); i++){

		Color color = stringToColor(objects_json[i]["color"].string_value());
		string type = objects_json[i]["type"].string_value();

		if (type == "sphere"){
			Vect center = stringToVector(objects_json[i]["center"].string_value());
			float radius = objects_json[i]["radius"].number_value();

			Sphere *S = new Sphere (center, radius, color);
			sceneObjects.push_back(dynamic_cast<Object*>(S));
		}
        else if (type == "plan"){
            Vect normal = stringToVector(objects_json[i]["normal"].string_value());
            double distance = objects_json[i]["distance"].number_value();
            Plane *P = new Plane(normal, distance, color);
            sceneObjects.push_back(dynamic_cast<Object*>(P));
        }
		
    }

    //read array of camera
	Json::array camera_json = scene_json["camera"].array_items();
    //std::cout <<"num de luzes : " << lights_json.size() << '\n';
	for (size_t i = 0; i < camera_json.size(); i++){
        
        //coordinate system
        Vect O (0,0,0);
        Vect X (1,0,0);
        Vect Y (0,1,0);
        Vect Z (0,0,1);

		Vect campos = stringToVector(camera_json[i]["position"].string_value());
		Vect look_at = stringToVector(camera_json[i]["lookat"].string_value());

        Vect diff_btw (campos.getVectX() - look_at.getVectX(),campos.getVectY() - look_at.getVectY(),campos.getVectZ()-look_at.getVectZ());
        Vect camdir = diff_btw.negative().normalize();
        Vect camright = Y.crossproduct(camdir).normalize();
        Vect camdown = camright.crossproduct(camdir);

        Camera *scene_cam = new Camera(campos, camdir, camright, camdown);
        S.scene_camera = scene_cam;
	}



    S.lights = lights;
    S.sceneObjects = sceneObjects;
    
    
    return S;

}

/** Saves the pixels in a file
 *  @param filename the name of the file where the pixels will be saved
 *  @param w the width of the image
 *  @param h the height of the image
 *  @param dpi dots per inch rate of the image
 *  @param data the array with the pixels
 * */
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

}
#endif //_FILE_MANAGER_HPP