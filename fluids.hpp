#pragma once // Tells the compiler to only include this file once
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>
#include <omp.h>


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

inline void get_macroscopic(int x, int y, double& rho, double& u_x, double& u_y) noexcept { // returns the current total x & y velocities, density of fluid at a point
    rho = 0.0;
    double momentum_x = 0.0;
    double momentum_y = 0.0;

    for (int i = 0; i < 9; ++i) {
        double f = Grid[get_index(x, y, i)];

        rho += f; // literally adding up the amount of stuff in each cell, regardless of direction
        momentum_x += f*cx[i]; // momentum at a point in the x direction == fraction of mass moving in x direction
        momentum_y += f*cy[i];        
    }

    u_x = momentum_x/rho;
    u_y = momentum_y/rho;
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
    double chord = width * 0.4;        
    double x_start = width * 0.25;     
    double y_center = height * 0.5;    
    
    // 2. Angle of Attack (AoA) Setup
    constexpr double AoA_degrees = 15.0; // Pitch nose UP by 15 degrees
    constexpr double pi = 3.14159265358979323846;
    constexpr double AoA_radians = AoA_degrees * pi / 180.0;
    
    // 3. Define the Pivot Point (The Aerodynamic Center at 25% chord)
    double pivot_x = x_start + (chord * 0.25);
    double pivot_y = y_center;
    
    // Get the current node's position relative to the pivot
    double dx = x - pivot_x;
    double dy = y - pivot_y;
    
    // 4. Apply 2D Rotation Matrix
    // We rotate the grid coordinates backwards to map them to the flat airfoil math
    double rot_x = dx * std::cos(AoA_radians) - dy * std::sin(AoA_radians);
    double rot_y = dx * std::sin(AoA_radians) + dy * std::cos(AoA_radians);
    
    double mapped_x = rot_x + pivot_x;
    double mapped_y = rot_y + pivot_y;

    // --- The rest is identical to the previous NACA 4415 math ---
    
    constexpr double thickness = 0.15; 
    constexpr double m = 0.04;         
    constexpr double p = 0.4;          

    // Normalize mapped_x position along the chord (0.0 to 1.0)
    double xc = (mapped_x - x_start) / chord;

    // If pixel is outside the wing's bounding box, skip
    if (xc < 0.0 || xc > 1.0) {
        return false;
    }

    // Calculate the symmetrical half-thickness
    double yt = 5.0 * thickness * (
          0.2969 * std::sqrt(xc) 
        - 0.1260 * xc 
        - 0.3516 * xc * xc 
        + 0.2843 * xc * xc * xc 
        - 0.1015 * xc * xc * xc * xc
    );

    // Calculate the Camber Line
    double yc = 0.0;
    if (xc >= 0.0 && xc <= p) {
        yc = (m / (p * p)) * (2.0 * p * xc - xc * xc);
    } else {
        yc = (m / ((1.0 - p) * (1.0 - p))) * ((1.0 - 2.0 * p) + 2.0 * p * xc - xc * xc);
    }

    // Scale the normalized math back up to your grid pixels
    double half_thickness_pixels = yt * chord;
    double camber_pixels = yc * chord;

    double wing_center_at_x = y_center + camber_pixels;
    
    // Check if the mapped_y coordinate is within the curved envelope
    return std::abs(mapped_y - wing_center_at_x) <= half_thickness_pixels;
}

inline void step_fluid() noexcept{

    std::fill(NextGrid.begin(), NextGrid.end(), 0.0); // reset next grid
    

    #pragma omp parallel for collapse(2) schedule(static) // parallel means runi in parallel, for means split up and don't do the same work,
                                                          // dollapse(2) takes a 2D loop and collapses it into 1D, Static means that it
                                                          //  doesnt split up dynamically, does it befrore a calculation has even start 
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
            Grid[get_index(0,y,i)] = get_equilibrium(i, 1, 0.075, 0.0);
            Grid[get_index(width - 1,y,i)] = Grid[get_index(width - 2, y, i)];
        }
    }
}

inline void write_frame_binary(std::ofstream& file) {
    // 1. Create a memory buffer to hold one entire frame of data
    std::vector<float> frame_buffer(width * height);
    int index = 0;

    // 2. Loop through the grid and fill the buffer (in RAM, which is lightning fast)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double rho, u_x, u_y;
            get_macroscopic(x, y, rho, u_x, u_y);
            
            frame_buffer[index] = static_cast<float>(std::sqrt(u_x*u_x + u_y*u_y));
            index++;
        }
    }
    
    // 3. Write the ENTIRE buffer to the hard drive in exactly ONE function call!
    file.write(reinterpret_cast<const char*>(frame_buffer.data()), frame_buffer.size() * sizeof(float));
}