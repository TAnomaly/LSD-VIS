# LSD-Vis Music Sequencer

A visual music sequencer built with OpenGL, GLFW, and GLSL. Users can create musical sequences by clicking on note blocks, which can be played back in sequence.

## Features

- 8 note blocks representing a C major scale (C4 to C5)
- Visual feedback with GLSL shader animations
- Click to add notes to the sequence
- Press Space to play the sequence
- Interactive GUI with real-time visual feedback

## Dependencies

- GLFW 3.3+
- OpenGL 3.3+
- CMake 3.10+

## Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Controls

- Left Mouse Click: Add note to sequence
- Space: Play sequence
- Esc: Exit application

## Project Structure

- `main.c`: Core application logic
- `shaders/vertex.glsl`: Vertex shader
- `shaders/fragment.glsl`: Fragment shader for visual effects
- `CMakeLists.txt`: Build configuration 