#pragma once // Tells the compiler to only include this file once
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>
#include <omp>



constexpr int width = 500;
constexpr int height = 500;

constexpr int cx[9] = {0, 1,  0, -1,  0, 1, -1, -1,  1};    // direction vectors *Still* E N W S NE NW SW SE 
constexpr int cy[9] = {0, 0,  1,  0, -1, 1,  1, -1, -1};    // ^^^^^^^^^^^^^^^^^
// store them as seperate arrays as it is computationally effecient for a CPU to access continous memory X, X, X, ... instead of a 2D array. X. Y. X. Y, ... 


constexpr double w[9] = {4.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/36.0, 1.0/36.0, 1.0/36.0, 1.0/36.0};  // Base resting distributexprion
constexpr int opp[9] = {0, 3, 4, 1, 2, 7, 8, 5, 6};


constexpr double tau = 0.55;          // Viscosity - adjustable
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

inline void init_fluid( double initial_rho, double  initial_u_x, double initial_u_y) noexcept {
    for (int x = 0; x < width; ++x){
        for (int y = 0; y < height; ++y){
            for(int i = 0; i<9; ++i){
                Grid[get_index(x, y, i)] = get_equilibrium(i, initial_rho, initial_u_x, initial_u_y);
            }
        }   
    }
}

inline bool is_inside_airfoil(int x, int y) noexcept {
    // 1. Scale geometry relative to your grid dimensions
    double chord = width * 0.4;        // Wing length is 40% of the domain width
    double x_start = width * 0.25;     // Start wing 25% of the way across the screen
    double y_center = height * 0.5;    // Center wing vertically
    
    // 2. NACA 4415 Airfoil parameters
    constexpr double thickness = 0.15; // 15% maximum thickness
    constexpr double m = 0.04;         // 4% maximum camber (This determines the "curve")
    constexpr double p = 0.4;          // Max camber is located at 40% of the chord length

    // Normalize x position along the chord (0.0 to 1.0)
    double xc = (x - x_start) / chord;

    // If pixel is outside the wing's bounding box, skip the heavy math
    if (xc < 0.0 || xc > 1.0) {
        return false;
    }

    // 3. Calculate the symmetrical half-thickness (your original math)
    double yt = 5.0 * thickness * (
          0.2969 * std::sqrt(xc) 
        - 0.1260 * xc 
        - 0.3516 * xc * xc 
        + 0.2843 * xc * xc * xc 
        - 0.1015 * xc * xc * xc * xc
    );

    // 4. Calculate the Camber Line (The asymmetric curve)
    double yc = 0.0;
    if (xc >= 0.0 && xc <= p) {
        yc = (m / (p * p)) * (2.0 * p * xc - xc * xc);
    } else {
        yc = (m / ((1.0 - p) * (1.0 - p))) * ((1.0 - 2.0 * p) + 2.0 * p * xc - xc * xc);
    }

    // 5. Scale the normalized math back up to your grid pixels
    double half_thickness_pixels = yt * chord;
    double camber_pixels = yc * chord;

    // Calculate the actual Y center for this specific X coordinate
    // (Adding camber shifts the center line up or down)
    double wing_center_at_x = y_center + camber_pixels;
    
    // 6. Check if the y-coordinate is within the curved envelope
    return std::abs(y - wing_center_at_x) <= half_thickness_pixels;
}

inline void step_fluid() noexcept{

    std::fill(NextGrid.begin(), NextGrid.end(), 0.0);
    
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {                
            bool is_current_solid = is_inside_airfoil(x, y);
            if (is_current_solid){continue;}
            
            double rho, u_x, u_y;                
            get_macroscopic(x, y, rho, u_x, u_y);




            for (int i = 0; i < 9; ++i) {

                double f = Grid[get_index(x, y, i)];
                double feq = get_equilibrium(i, rho, u_x, u_y);
                
                double f_post = f - omega*(f - feq);
                int nx = x + cx[i];
                int ny = y + cy[i];
                
                bool is_destination_solid = is_inside_airfoil(nx, ny);
                
                if (is_destination_solid) {
                    NextGrid[get_index(x, y, opp[i])] = f_post;
                }
                else if (ny >= 0 && ny < height && nx >= 0 && nx < width){
                    NextGrid[get_index(nx, ny, i)] = f_post; 
                }
                else if (nx >= 0 && nx < width){
                    NextGrid[get_index(x, y, opp[i])] = f_post;
                }
            }
        }
    }
    Grid = NextGrid; 

    for(int y = 0; y< height; ++y){
        for (int i = 0; i < 9; ++i){
            Grid[get_index(0,y,i)] = get_equilibrium(i, 1, 0.05, 0.0);
            Grid[get_index(width - 1,y,i)] = Grid[get_index(width - 2, y, i)];
            if (y < 300 && y > 200){
            Grid[get_index(0,y,i)] = get_equilibrium(i, 1, 0.10, 0.0);
            }
        }
    }
}

inline void write_frame(std::ofstream& file, int step) {
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            double rho, u_x, u_y;
            get_macroscopic(x, y, rho, u_x, u_y);
            
            // We added 'step' as the first column
            file << step << "," << x << "," << y << "," << u_x << "," << u_y << "," << rho << "\n";
        }
    }
}