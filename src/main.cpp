#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <Windows.h>
#include <omp.h>
#include "map.h"
#include "player.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int TEX_WIDTH = 64;
const int TEX_HEIGHT = 64;

// Return a value between 0.0 and 1.0 (dark and bright)
float calculateShadow(const glm::vec2& start, const glm::vec2& lightPos, const Map& map) {
    glm::vec2 dir = lightPos - start;
    float distanceToLight = glm::length(dir);
    if (distanceToLight < 0.01f) return 1.0f;

    glm::vec2 rayDir = glm::normalize(dir);

    int mapX = static_cast<int>(start.x);
    int mapY = static_cast<int>(start.y);

    glm::vec2 deltaDist(
        (rayDir.x == 0) ? 1e30f : std::abs(1.0f / rayDir.x),
        (rayDir.y == 0) ? 1e30f : std::abs(1.0f / rayDir.y)
    );

    int stepX, stepY;
    glm::vec2 sideDist;

    if (rayDir.x < 0) {
        stepX = -1;
        sideDist.x = (start.x - mapX) * deltaDist.x;
    }
    else {
        stepX = 1;
        sideDist.x = (mapX + 1.0f - start.x) * deltaDist.x;
    }

    if (rayDir.y < 0) {
        stepY = -1;
        sideDist.y = (start.y - mapY) * deltaDist.y;
    }
    else {
        stepY = 1;
        sideDist.y = (mapY + 1.0f - start.y) * deltaDist.y;
    }

    float travelDist = 0.0f;
    while (travelDist < distanceToLight) {
        if (sideDist.x < sideDist.y) {
            travelDist = sideDist.x;
            sideDist.x += deltaDist.x;
            mapX += stepX;
        }
        else {
            travelDist = sideDist.y;
            sideDist.y += deltaDist.y;
            mapY += stepY;
        }

        if (map.isWall(mapX, mapY)) {

            if (travelDist < distanceToLight - 0.05f) {
                return 0.15f;
            }
        }
    }
    return 1.0f;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;

    SDL_Window* window = SDL_CreateWindow(
        "OverRay (Raycasting 3D test)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN
    );

    SDL_SetRelativeMouseMode(SDL_TRUE);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    std::vector<uint32_t> pixelBuffer(SCREEN_WIDTH * SCREEN_HEIGHT, 0);

    Map map;
    Player player;

    bool running = true;
    SDL_Event event;

    Uint32 lastTime = SDL_GetTicks();

    float time = 0.0f;

    // Texture Generation
    std::vector<std::vector<uint32_t>> textures(4, std::vector<uint32_t>(TEX_WIDTH * TEX_HEIGHT));

    for (int y = 0; y < TEX_HEIGHT; ++y) {
        for (int x = 0; x < TEX_WIDTH; ++x) {
            // Tex 1: bricks
            if (y % 16 == 0 || (x % 32 == 0 && (y / 16) % 2 == 0) || ((x + 16) % 32 == 0 && (y / 16) % 2 != 0)) {
                textures[1][y * TEX_WIDTH + x] = 0xFF444444;
            }
            else {
                textures[1][y * TEX_WIDTH + x] = 0xFFCC4444;
            }

            // Tex 2: some kind of... fence?
            if (x == 0 || x == TEX_WIDTH - 1 || y == 0 || y == TEX_HEIGHT - 1 || x == y || x == TEX_HEIGHT - y) {
                textures[2][y * TEX_WIDTH + x] = 0xFF888888;
            }
            else {
                textures[2][y * TEX_WIDTH + x] = 0xFF222222;
            }

            // Tex 3: "wood"
            if (y % 8 == 0) {
                textures[3][y * TEX_WIDTH + x] = 0xFF4A2F13;
            }
            else {
                textures[3][y * TEX_WIDTH + x] = 0xFF8B5A2B;
            }
        }
    }

    // MAIN LOOP

    while (running) {
        // Delta
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        time += 0.05f;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }

            // Mouse camera move
            if (event.type == SDL_MOUSEMOTION) {
                float mouseSensitivity = 0.002f;
                player.angle += event.motion.xrel * mouseSensitivity;
            }
        }
        
        // Update player
        player.update(deltaTime, map);

        // Floor + Ceiling colors
        std::fill(pixelBuffer.begin(), pixelBuffer.begin() + (SCREEN_WIDTH * SCREEN_HEIGHT / 2), 0xFF020202); // Cielo casi negro
        std::fill(pixelBuffer.begin() + (SCREEN_WIDTH * SCREEN_HEIGHT / 2), pixelBuffer.end(), 0xFF080812);   // Suelo azul oscuro (combina con ambientColor)

        glm::vec2 dir(cos(player.angle), sin(player.angle));

        // Camera Plane
        glm::vec2 plane(-dir.y * 0.66f, dir.x * 0.66f);

        #pragma omp parallel for schedule(dynamic)

        for (int x = 0; x < SCREEN_WIDTH; ++x) {
        
            float cameraX = 2.0f * x / static_cast<float>(SCREEN_WIDTH) - 1.0f;
            glm::vec2 rayDir = dir + plane * cameraX;

            int mapX = static_cast<int>(player.position.x);
            int mapY = static_cast<int>(player.position.y);

            
            glm::vec2 deltaDist(
                (rayDir.x == 0) ? 1e30f : std::abs(1.0f / rayDir.x),
                (rayDir.y == 0) ? 1e30f : std::abs(1.0f / rayDir.y)
            );

            
            int stepX, stepY;
            
            glm::vec2 sideDist;

            if (rayDir.x < 0) {
                stepX = -1;
                sideDist.x = (player.position.x - mapX) * deltaDist.x;
            }
            else {
                stepX = 1;
                sideDist.x = (mapX + 1.0f - player.position.x) * deltaDist.x;
            }

            if (rayDir.y < 0) {
                stepY = -1;
                sideDist.y = (player.position.y - mapY) * deltaDist.y;
            }
            else {
                stepY = 1;
                sideDist.y = (mapY + 1.0f - player.position.y) * deltaDist.y;
            }

            // DDA
            bool hit = false;
            int side = 0; // X == 0; Y == 1

            while (!hit) {
                // Next cell
                if (sideDist.x < sideDist.y) {
                    sideDist.x += deltaDist.x;
                    mapX += stepX;
                    side = 0;
                }
                else {
                    sideDist.y += deltaDist.y;
                    mapY += stepY;
                    side = 1;
                }

                if (map.isWall(mapX, mapY)) {
                    hit = true;
                }
            }

            // 3D Projection
            float perpWallDist;
            if (side == 0) perpWallDist = (sideDist.x - deltaDist.x);
            else           perpWallDist = (sideDist.y - deltaDist.y);

            if (perpWallDist <= 0.0f) perpWallDist = 0.05f;

            // perspective
            int lineHeight = static_cast<int>(SCREEN_HEIGHT / perpWallDist);

            int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
            if (drawStart < 0) drawStart = 0;
            int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
            if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

            // impact point
            glm::vec2 intersectionPoint;
            if (side == 0) {
                intersectionPoint.x = mapX + (stepX > 0 ? 0.0f : 1.0f);
                intersectionPoint.y = player.position.y + perpWallDist * rayDir.y;
            }
            else {
                intersectionPoint.x = player.position.x + perpWallDist * rayDir.x;
                intersectionPoint.y = mapY + (stepY > 0 ? 0.0f : 1.0f);
            }

            // light pos
            glm::vec2 lightPos(player.position);

            // Shadows

            float distToLight = glm::distance(intersectionPoint, lightPos);
            float radius = 7.5f + (sin(time) * 1.5f);
            float attenuation = (std::max)(0.0f, 1.0f - (distToLight / radius));
            attenuation = attenuation * attenuation; // Caída de luz suave

            float shadowFactor = calculateShadow(intersectionPoint, lightPos, map);

            float directLight = attenuation * shadowFactor;

            // Light Color
            
            glm::vec3 lightColor(1.0f, 0.55f, 0.1f);

            // Ambient light
            glm::vec3 ambientColor(0.03f, 0.03f, 0.08f);

            // Ambient + Direct light
            glm::vec3 totalIllumination = ambientColor + (lightColor * directLight);
            totalIllumination.r = (std::clamp)(totalIllumination.r, 0.0f, 1.0f);
            totalIllumination.g = (std::clamp)(totalIllumination.g, 0.0f, 1.0f);
            totalIllumination.b = (std::clamp)(totalIllumination.b, 0.0f, 1.0f);

            // Texture coords
            float wallX;
            if (side == 0) {
                wallX = player.position.y + perpWallDist * rayDir.y;
            }
            else {
                wallX = player.position.x + perpWallDist * rayDir.x;
            }
            wallX -= floor(wallX);

            int texX = static_cast<int>(wallX * static_cast<float>(TEX_WIDTH));
           
            if (side == 0 && rayDir.x > 0) texX = TEX_WIDTH - texX - 1;
            if (side == 1 && rayDir.y < 0) texX = TEX_WIDTH - texX - 1;

            // Wall Types
            int wallType = map.getWallType(mapX, mapY);
            
            if (wallType < 1 || wallType > 3) wallType = 1;

            float sideShading = (side == 1) ? 0.85f : 1.0f;

            // Draw Vertical Texture
            float step = 1.0f * TEX_HEIGHT / lineHeight;
            float texPos = (drawStart - SCREEN_HEIGHT / 2 + lineHeight / 2) * step;

            for (int y = drawStart; y < drawEnd; ++y) {
                
                int texY = static_cast<int>(texPos) & (TEX_HEIGHT - 1);
                texPos += step;

                // raw color
                uint32_t rawColor = textures[wallType][texY * TEX_WIDTH + texX];

                uint8_t rawR = (rawColor >> 16) & 0xFF;
                uint8_t rawG = (rawColor >> 8) & 0xFF;
                uint8_t rawB = rawColor & 0xFF;

                // Multiply illumination * texture color * side shade
                uint8_t r = static_cast<uint8_t>(rawR * totalIllumination.r * sideShading);
                uint8_t g = static_cast<uint8_t>(rawG * totalIllumination.g * sideShading);
                uint8_t b = static_cast<uint8_t>(rawB * totalIllumination.b * sideShading);

                // Send to pixel buffer
                pixelBuffer[y * SCREEN_WIDTH + x] = (255 << 24) | (r << 16) | (g << 8) | b;
            }
        }

        // draw
        SDL_UpdateTexture(screenTexture, NULL, pixelBuffer.data(), SCREEN_WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(screenTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}