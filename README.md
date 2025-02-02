# huedra
Huedra is a prototype Game Engine made with as few dependencies as possible. In it's current state, only Windows and Vulkan is supported for rendering. This includes a dynamic render graph system with creation for render targets, buffers and textures as resources. Multiple windows/swap chains are supported with automatic presentation, layout transitions and CPU-GPU synchronization.

## Features
* General Rendering interface (graphics and compute pipeline), using Vulkan backend (with the possibility for other APIs in the future) 
* Window manager - multiple windows, parenting
* Keyboard Input
* Mathematics (Vector, Matrix, Quaternion)
* Import of meshes: .obj, .gltf, .glb
* Import of textures: .png
* Import/Export of .json files

## Future Features
* Scene graph with ECS
* Physics
* Audio
* Input (Mouse, Controller)
* Additional rendering features
* Additional graphics APIs (DX12, Metal)
* Additional platforms (Linux, Mac)
* IO of more file formats (.fbx, .jpg)

## Building
As it currently stands, huedra is built using cmake and C++23. As the project is still experimental, official build instructions are not provided. The project has until now only been tested using clang 18.1.6 and ninja 1.12.1 on Windows in Visual Studio Code. Dependencies needed by cmake to compile correctly are currently the [Vulkan SDK](https://vulkan.lunarg.com) and [Python3](https://www.python.org/downloads/).
