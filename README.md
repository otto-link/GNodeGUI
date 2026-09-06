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

## Optional node chrome

`GN_STYLE->node.bevel_width` enables an inset bevel when greater than zero,
using `color_bevel_top` and `color_bevel_bottom` for the upper and lower halves
of the card outline. Width is in scene units and scales with canvas zoom.
The bevel stays inside the outer hover/selection border. Set these fields
before creating the graph, using colours from the host application's theme.

`color_port_caption` controls port-label ink independently of the node title.
The defaults preserve flat cards and white port labels: bevel width is zero,
both bevel colours are transparent, and port-caption colour is white.

`rounding_radius` continues to define the shared header/body card silhouette.
Embedded controls own their own radii. Slider-thumb gradients belong to the
embedded widget renderer; GNodeGUI does not draw slider thumbs.

With `GNODEGUI_ENABLE_TESTS=ON`, build `node_style_test` and run
`bin/node_style_test -platform offscreen` for pixel checks of disabled bevels,
bevel bounds and colours, selection visibility, and port-label ink isolation.

## License

This project is licensed under the GPL-3.0 license.
