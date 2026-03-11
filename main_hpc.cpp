#include <iostream>
#include "fluids.hpp"

int main(){
    std::cout << "Initialising..." <<"\r";
    init_fluid();

    int max_steps = 10000;

    std::cout << "Starting simulation for " << max_steps << " steps..." << std::endl;

    for(int t = 0; t<max_steps; ++t){
        step_fluid();

        if (t % 1000 == 0) {
            std::cout << "\r";
            std::cout << "Step: " << t;
        }
    }
    std::cout << "Simulation complete!" << std::endl;
    return 0;


}