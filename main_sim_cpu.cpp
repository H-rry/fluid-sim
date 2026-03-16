#include <iostream>
#include "fluids_cpu.hpp"
#include <sstream>
#include <fstream>
#include <chrono>


int width;
int height;
float wind_speed;
double tau;
double omega;
int total_nodes;

double* Grid;     
double* NextGrid;
int max_steps;
int save_frequency;


int main(int argc, char** argv){
 
    width = std::stoi(argv[1]);
    height = std::stoi(argv[2]);
    wind_speed = std::stod(argv[3]);
    tau = std::stod(argv[4]);
    omega = 1.0 / tau;
    total_nodes = width * height * 9;
    Grid = new double[total_nodes];
    NextGrid = new double[total_nodes];
    int max_steps = argv[5];
    int save_frequency = argv[6];

    double initial_rho = 1.0;
    double initial_u_x = 0.0;
    double initial_u_y = 0.0;
    init_fluid(initial_rho, initial_u_x, initial_u_y);


    std::ofstream outfile("simulation_data.bin", std::ios::out | std::ios::binary);

    
    std::cout << "Starting simulation for " << max_steps << " steps..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for(int t = 0; t<max_steps; ++t){
        if (t % save_frequency == 0) {
            write_frame_binary(outfile); 
        }
        step_fluid();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Elapsed time: " << duration.count() << "s" << std::endl;
    
    outfile.close();

    delete[] Grid;
    delete[] NextGrid;
    
    return 0;
}

