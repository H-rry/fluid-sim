#include <iostream>
#include "fluids.hpp"
#include <SDL2/SDL.h>

int main(){
    double inital_rho = 1.0;
    double inital_u_x = 0.05;
    double inital_u_y = 0.0;

    for (int x = 0; x < width; ++x){
        for (int y = 0; y < height; ++y){
            for(int i = 0; i<9; ++i){
                Grid[get_index(x, y, i)] = get_equilibrium(i, inital_rho, inital_u_x, inital_u_y);
            }
        }   
    }

    int max_steps = 1000;

    for (int t = 0; t < max_steps; ++t) {
        std::fill(NextGrid.begin(), NextGrid.end(), 0.0);
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {                
                double rho, u_x, u_y;                
                get_macroscopic(x, y, rho, u_x, u_y);

                for (int i = 0; i < 9; ++i) {
                    
                    double f = Grid[get_index(x, y, i)];
                    double feq = get_equilibrium(i, rho, u_x, u_y);
                    

                    double f_post = f - omega*(f - feq);
                    int nx = x + cx[i];
                    int ny = y + cy[i];


                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {  // checking edge case
                        NextGrid[get_index(nx, ny, i)] = f_post;        
                   }
                }
            }
        }
        Grid = NextGrid; 
}

    return 0;

}