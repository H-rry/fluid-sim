#include <iostream>
#include "fluids.hpp"
#include <sstream>
#include <fstream>



int main(){
    std::cout << "Initialising..." <<"\r";
    double initial_rho = 1.0;
    double initial_u_x = 0.00;
    double initial_u_y = 0.0;
    init_fluid(initial_rho, initial_u_x, initial_u_y);


    std::ofstream outfile("simulation_data.csv");
    outfile << "step,x,y,u_x,u_y,rho\n"; // Header row


    int max_steps = 10000;
    int save_frequency = 50;
    std::cout << "Starting simulation for " << max_steps << " steps..." << std::endl;

    for(int t = 0; t<max_steps; ++t){

        if (t % save_frequency == 0) {
            std::cout << "Saving step: " << t << " to master file..." << std::endl;
            write_frame(outfile, t); 
        }
        step_fluid();

    }
    std::cout << "Simulation complete!" << std::endl;
    return 0;
}

