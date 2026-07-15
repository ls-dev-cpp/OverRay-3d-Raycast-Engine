# OverRay: 3D Software Raycasting Engine from Scratch (C++)

OverRay is a retro-style 3D rendering engine built entirely from scratch in **C++** using **SDL2** for window/input management and pixel buffer presentation, and **GLM** for linear algebra. 

<img width="628" height="464" alt="DDA" src="https://github.com/user-attachments/assets/afaa94db-d4de-4d17-8866-1d5ddab54b90" />

The engine handles all camera movement, dynamic lighting, and shadow calculations directly on the CPU (software rendering), achieving high performance through parallel computing.

---

## Key Features

* **Pure Software 3D Projection:** Implementation of the Digital Differential Analysis (DDA) algorithm to cast 640 rays (one per column) and find exact wall grid intersections.
* **Continuous Floating-Point Movement:** Smooth WASD movement combined with mouse-look controls and grid-based collision handling.
* **Dynamic RGB Torch & Attenuation:** A flickering light source tied to the player's position, featuring distance-based quadratic falloff to simulate a realistic torch.
* **Real-Time Shadow Ray Casting:** Secondary shadow rays are cast from collision points back to the light source, allowing real-time wall shadows and ambient occlusion in tight corners.
* **Multithreading via OpenMP:** Parallelized column rendering utilizing OpenMP specification to maximize CPU core efficiency and boost frame rates.
* **Procedural Wall Texturing:** Custom procedural textures (brick, wood, and metal) generated on-the-fly and blended pixel-by-pixel with the local light calculations.

---

## Used Tech

* **Language:** C++17
* **Graphics & Windowing:** SDL2
* **Math Library:** GLM (OpenGL Mathematics)
* **Parallelism:** OpenMP
* **Build System:** CMake

---

## How It Works (The Rendering Pipeline)

For every frame, the engine executes the following steps inside the main loop:
1. **Primary DDA Sweep:** Casts 640 rays across the player's FOV to calculate the perpendicular wall distance (`perpWallDist`) to eliminate "fisheye" distortion.
2. **Shadow Ray Tracing:** Before coloring a wall column, a secondary ray is traced from the intersection point back to the light source. If an obstacle is encountered, the point is shaded as "in shadow".
3. **Texture Mapping & Shading:** Calculates the precise fractional wall intersection coordinate (`wallX`) to map texture columns dynamically, scaling them based on distance and multiplying each pixel's RGB channels by the calculated illumination.
4. **VRAM Presentation (SDL2):** The 1D pixel buffer containing the processed ARGB pixel data is pushed directly to an SDL texture on the GPU with `SDL_UpdateTexture` and drawn on screen.

---

## How to Build & Run

### Prerequisites
* A C++ compiler supporting C++17 and OpenMP.
* CMake installed.
* SDL2 and GLM libraries.

### Quick Compile
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
