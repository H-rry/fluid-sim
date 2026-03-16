#include <iostream>
#include "fluids_gpu.hpp"
#include <sstream>
#include <fstream>
#include <chrono>

int main(){
    double initial_rho = 1.0;
    double initial_u_x = 0.00;
    double initial_u_y = 0.0;
    init_fluid(initial_rho, initial_u_x, initial_u_y);


    std::ofstream outfile("simulation_data.bin", std::ios::out | std::ios::binary);

    int max_steps = 10000;
    int save_frequency = 50;
    std::cout << "Starting simulation for " << max_steps << " steps..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for(int t = 0; t<max_steps; ++t){
        if (t % save_frequency == 0) {
            #pragma omp target update from(Grid[0:total_nodes]) // pulls data from gpu to the cpu for write
            write_frame_binary(outfile); 
        }
        step_fluid();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Elapsed time: " << duration.count() << "s" << std::endl;
    
    outfile.close();
    
    return 0;
}

