# GNodeGUI

GNodeGUI is a C++ graphical node editor library aimed at providing an interface for building and manipulating nodes in a graphical context.
This library is currently used in the [Hesiod](https://github.com/otto-link/Hesiod) GUI, a node-based system for heightmap generation.

![Screenshot_2024-10-09_19-13-17](https://github.com/user-attachments/assets/3362ae46-47ee-4add-b7fd-9f143d8d887c)



## Features

- Node-based graphical interface
- Supports custom node definitions

## Build Instructions

1. Clone the repository:
   ```bash
   git clone --recurse-submodules https://github.com/otto-link/GNodeGUI.git
   ```
2. Build using CMake:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

## License

This project is licensed under the GPL-3.0 license.
