#pragma once // Tells the compiler to only include this file once
#include <vector>
#include <cmath>
#include <algorithm>


constexpr int width = 500;
constexpr int height = 500;

constexpr int cx[9] = {0, 1,  0, -1,  0, 1, -1, -1,  1};    // direction vectors *Still* E N W S NE NW SW SE 
constexpr int cy[9] = {0, 0,  1,  0, -1, 1,  1, -1, -1};    // ^^^^^^^^^^^^^^^^^
// store them as seperate arrays as it is computationally effecient for a CPU to access continous memory X, X, X, ... instead of a 2D array. X. Y. X. Y, ... 


constexpr double w[9] = {4.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/36.0, 1.0/36.0, 1.0/36.0, 1.0/36.0};  // Base resting distributexprion

constexpr int opp[9] = {0, 3, 4, 1, 2, 7, 8, 5, 6};

constexpr double tau = 0.6;          // Viscosity - adjustable
constexpr double omega = 1.0 / tau;  // Collision frequency - computationaly cheaper to multiply than divide

constexpr int total_nodes = width * height * 9; // This is the total number of values that are needed to describe the system

std::vector<double> Grid(total_nodes, 0.0);
std::vector<double> NextGrid(total_nodes, 0.0);

inline int get_index(int x,int y,int q) noexcept{    // helper function to take our cartesian coords and "flatten" them, a 1D array is more computationally efficient
    return x*height*9 + y*9 + q;
} 

inline void get_macroscopic(int x, int y, double& rho, double& u_x, double& u_y) noexcept {
    rho = 0.0;
    double momentum_x = 0.0;
    double momentum_y = 0.0;

    for (int i = 0; i < 9; ++i) {
        double f = Grid[get_index(x, y, i)];

        rho += f;
        momentum_x += f*cx[i];
        momentum_y += f*cy[i];        
    }

    double rho_inverse = 1.0/rho;

    u_x = rho_inverse*momentum_x;
    u_y = rho_inverse*momentum_y;
}

inline double get_equilibrium(int i, double rho, double u_x, double u_y) noexcept {
    double cu = cx[i] * u_x + cy[i] * u_y; 
    double u_squared = u_x*u_x + u_y*u_y;

    return w[i] * rho * (1.0 + 3.0*cu + 4.5*cu*cu - 1.5*u_squared);
}

inline void init_fluid() noexcept {
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
}


inline void step_fluid() noexcept{

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
