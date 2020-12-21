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
#include "RayTracer.hpp"

/**Ray Tracing project for IN204
 * ENSTA 2020
 * Authors: Joao Pedro Araujo Ferreira Campos, David Velasquez Ospina
 * 
 * This project reads a scene described in a .json and then utilises a ray-tracing rendering technique to get the color of each pixel and save the result in Scene.bmp
*/
int main(int argc, char *argv[])
{
    std::chrono::time_point < std::chrono::system_clock > start_calcul, end_calcul, start_program, end_program;
    
    argh::parser cmdl(argv);
    std::string path;
    cmdl({"-s", "--scene"}, "scene.json") >> path;


    const int Width=600, Height=400, dpi = 72;
    float accuracy = 0.00001;
    int aadepth = 4;            //anti-aliasing depth
    float aathreshold = 0.1;            //anti-aliasing threshold

    int n = Width*Height;

    RGBType *pixels = new RGBType[n];


    MPI_Init(&argc,&argv);
    start_program = std::chrono::system_clock::now();

    int size, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;

    int local_Height = Height/size;
    int firstrow = (size-(size-rank)) * local_Height;
    int endrow = firstrow + local_Height;
    int local_n = local_Height*Width;
    RGBType *local_pixels = new RGBType[local_n];

    start_calcul = std::chrono::system_clock::now();

    local_pixels = calculatePixels(Width,Height,aadepth,accuracy,firstrow,endrow,path);

    end_calcul = std::chrono::system_clock::now();
    std::chrono::duration < double >elapsed_seconds = end_calcul - start_calcul;
    std::cout << "Temps de calcul: " << elapsed_seconds.count() << " secondes, tache : " << rank << "\n";


    float red_pixels[local_n];
    float blue_pixels[local_n];
    float green_pixels[local_n];


    for(int i=0;i<local_n;i++){

        red_pixels[i] = local_pixels[i].r;
        blue_pixels[i] = local_pixels[i].b;
        green_pixels[i] = local_pixels[i].g;
    }

    if (rank == 0){
    
        float total_red_pixels[n];
        float total_blue_pixels[n];
        float total_green_pixels[n];

        //Get all the pixels
        MPI_Gather( red_pixels,local_n, MPI_FLOAT,
                total_red_pixels, local_n, MPI_FLOAT,
                0,
                MPI_COMM_WORLD               
               );
        MPI_Gather( green_pixels,local_n, MPI_FLOAT,
                total_green_pixels, local_n, MPI_FLOAT,
                0,
                MPI_COMM_WORLD               
               );
        MPI_Gather( blue_pixels,local_n, MPI_FLOAT,
                total_blue_pixels, local_n, MPI_FLOAT,
                0,
                MPI_COMM_WORLD               
               );
        for(int i=0; i < n; i++){
            pixels[i].r = total_red_pixels[i];
            pixels[i].b = total_blue_pixels[i];
            pixels[i].g = total_green_pixels[i];

        }

        //save the image
        savebmp("Scene.bmp",Width,Height,dpi,pixels);
        end_program = std::chrono::system_clock::now();
        std::chrono::duration < double >total_time = end_program - start_program;
        std::cout << "Temps de calcul: " << total_time.count() << " secondes\n";

    }else{
        MPI_Gather( red_pixels,local_n, MPI_FLOAT,
                NULL, local_n, MPI_FLOAT,
                0,
                MPI_COMM_WORLD               
               );
        MPI_Gather( green_pixels,local_n, MPI_FLOAT,
                NULL, local_n, MPI_FLOAT,
                0,
                MPI_COMM_WORLD               
               );
        MPI_Gather( blue_pixels,local_n, MPI_FLOAT,
                NULL, local_n, MPI_FLOAT,
                0,
                MPI_COMM_WORLD               
               );
    }

    delete local_pixels;
    delete pixels;

    MPI_Finalize();
    return 0;
}
