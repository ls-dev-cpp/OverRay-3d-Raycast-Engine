#pragma once
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include "map.h"
#include <iostream>

class Player {
public:
    glm::vec2 position = glm::vec2(6.0f, 6.0f);
    float angle = 0.0f;

    float moveSpeed = 4.0f;
    float rotSpeed = 3.0f;

    void update(float deltaTime, const Map& map) {
        const uint8_t* keystate = SDL_GetKeyboardState(NULL);

        glm::vec2 moveDir(cos(angle), sin(angle));
        glm::vec2 strafeDir(-sin(angle), cos(angle));

        glm::vec2 nextPos = position;

        // WASD
        if (keystate[SDL_SCANCODE_W]) {
            nextPos += moveDir * moveSpeed * deltaTime;
        }
        if (keystate[SDL_SCANCODE_S]) {
            nextPos -= moveDir * moveSpeed * deltaTime;
        }
        if (keystate[SDL_SCANCODE_A]) {
            nextPos -= strafeDir * moveSpeed * deltaTime;
        }
        if (keystate[SDL_SCANCODE_D]) {
            nextPos += strafeDir * moveSpeed * deltaTime;
        }

        // Collision
        if (!map.isWall(static_cast<int>(nextPos.x), static_cast<int>(position.y))) {
            position.x = nextPos.x;
        }
        if (!map.isWall(static_cast<int>(position.x), static_cast<int>(nextPos.y))) {
            position.y = nextPos.y;
        }
    }
};