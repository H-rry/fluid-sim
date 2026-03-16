#pragma once // Tells the compiler to only include this file once
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>
#include <omp.h>


extern int width;
extern int height;
extern float wind_speed;

extern double tau;         // Relaxation time (LBM analogous Viscosity - adjustable) v = c^2(tau - 0.5)     (c is the lattic speed of sound)
extern double omega;       // Collision frequency - computationaly cheaper to multiply than divide

extern int total_nodes;    // This is the total number of values that are needed to describe the system
extern double* Grid;
extern double* NextGrid;

#pragma omp declare target // telling the GPU to add it to its memory

constexpr int cx[9] = {0, 1,  0, -1,  0, 1, -1, -1,  1};    // direction vectors *Still* E N W S NE NW SW SE 
constexpr int cy[9] = {0, 0,  1,  0, -1, 1,  1, -1, -1};    // ^^^^^^^^^^^^^^^^^
// store them as seperate arrays as it is computationally effecient for a CPU to access continous memory X, X, X, ... instead of a 2D array. X. Y. X. Y, ... 

constexpr double w[9] = {4.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/36.0, 1.0/36.0, 1.0/36.0, 1.0/36.0};  // Base resting distributexprion
constexpr int opp[9] = {0, 3, 4, 1, 2, 7, 8, 5, 6};

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

inline double get_equilibrium(int i, double rho, double u_x, double u_y) noexcept { // Returns where the fluid wants to be, the "ideal" state
    double cu = cx[i] * u_x + cy[i] * u_y; 
    double u_squared = u_x*u_x + u_y*u_y;

    return w[i] * rho * (1.0 + 3.0*cu + 4.5*cu*cu - 1.5*u_squared);
}

#pragma omp end declare target // GPU does not care about initialisation

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
    constexpr double AoA_degrees = 5.0; // Pitch nose UP by 15 degrees
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

    #pragma omp parallel for    // parallelised resetting of nextgrid
    for (int i = 0; i < total_nodes; ++i) NextGrid[i] = 0.0;    

    #pragma omp target teams distribute parallel for collapse(2)
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {                
            bool is_current_solid = is_inside_airfoil(x, y);
            if (is_current_solid){continue;}
            
            double rho, u_x, u_y;                
            get_macroscopic(x, y, rho, u_x, u_y);


            for (int i = 0; i < 9; ++i) {

                double f = Grid[get_index(x, y, i)];
                double feq = get_equilibrium(i, rho, u_x, u_y);
                
                double f_post = f - omega*(f - feq);    // edits the next value so it's like f -> a closer value to the equilibrium
                int nx = x + cx[i];
                int ny = y + cy[i];
                
                bool is_destination_solid = is_inside_airfoil(nx, ny);
                
                if (is_destination_solid) { // adds the airfoil
                    NextGrid[get_index(x, y, opp[i])] = f_post;
                }
                else if (ny >= 0 && ny < height && nx >= 0 && nx < width){
                    NextGrid[get_index(nx, ny, i)] = f_post; 
                }
                else if (nx >= 0 && nx < width){    // gets here iff on the edge, wind tunnel, closes top and bottom
                    NextGrid[get_index(x, y, opp[i])] = f_post;
                }
            }
        }
    }
    double* temp = Grid;    // move the pointer of NextGrid to be off Grid2
    Grid = NextGrid;
    NextGrid = temp;

    // Boundary conditions
    #pragma omp target teams distribute parallel for

    for(int y = 0; y< height; ++y){ // adds slow wind from left to right
        for (int i = 0; i < 9; ++i){
            Grid[get_index(0,y,i)] = get_equilibrium(i, 1, wind_speed, 0.0);
            Grid[get_index(width - 1,y,i)] = Grid[get_index(width - 2, y, i)];
        }
    }

}

inline void write_frame_binary(std::ofstream& file) {
    std::vector<float> frame_buffer(width * height);    // creates a buffer the size of the data for each save-step
    int index = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double rho, u_x, u_y;
            get_macroscopic(x, y, rho, u_x, u_y);
            
            frame_buffer[index] = static_cast<float>(std::sqrt(u_x*u_x + u_y*u_y));
            index++;
        }
    }
    
    file.write(reinterpret_cast<const char*>(frame_buffer.data()), frame_buffer.size() * sizeof(float));    // turns buffer data into bin, size is (buffer size)*(float size)
}


