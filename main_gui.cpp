#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <SDL2/SDL.h>

const int lines_per_frame = width * height; 

inline uint32_t get_color(double u_x, double u_y) {
    double speed = std::sqrt(u_x * u_x + u_y * u_y);
    int val = std::min(255, (int)(speed * 2500.0));
    return (0xFF000000) | (val << 16) | (255 - val);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("LBM Master CSV Playback", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    std::vector<uint32_t> pixels(width * height, 0xFF000000);
    
    // Open our massive file
    std::ifstream file("simulation_data.csv");
    if (!file.is_open()) {
        std::cerr << "Could not open simulation_data.csv! Did you run the HPC build first?" << std::endl;
        return -1;
    }

    std::string line;
    std::getline(file, line); // Throw away the header row

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        bool end_of_file = false;

        // Read exactly one frame's worth of data (250,000 lines)
        for (int i = 0; i < lines_per_frame; ++i) {
            if (!std::getline(file, line)) {
                end_of_file = true;
                break; // We hit the end of the CSV!
            }

            std::stringstream ss(line);
            std::string token;
            int x, y;
            double u_x, u_y;

            // Parse: step, x, y, u_x, u_y
            std::getline(ss, token, ','); // step (we actually ignore this for graphics)
            std::getline(ss, token, ','); x = std::stoi(token);
            std::getline(ss, token, ','); y = std::stoi(token);
            std::getline(ss, token, ','); u_x = std::stod(token);
            std::getline(ss, token, ','); u_y = std::stod(token);
            
            pixels[y * width + x] = get_color(u_x, u_y);
        }

        if (!end_of_file) {
            // Blast the frame to the GPU
            SDL_UpdateTexture(texture, nullptr, pixels.data(), width * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
            
            SDL_Delay(33); // 30 FPS delay
            
        } else {
            // Movie is over! Loop it back to the beginning.
            file.clear(); // Clear the "End of File" error flag
            file.seekg(0); // Rewind file back to byte 0
            std::getline(file, line); // Skip the header again
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}