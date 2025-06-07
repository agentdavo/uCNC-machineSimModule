# cncvis: Lightweight CNC Machine Visualization

`cncvis` is a compact C library for rendering and animating CNC machines.
It parses an XML description of the machine, loads STL files for the
individual parts and draws them using **TinyGL**. The project is self
contained – TinyGL, a simple STL loader and the Mini-XML parser are
included under `cncvis/`.

## Features

- **Hierarchical assemblies** – build kinematic chains from movable
  assemblies and actors.
- **STL model support** – load models using the included STL IO library.
- **Lighting** – configure multiple OpenGL style lights.
- **Camera API** – orbit, pan, zoom and preset views.
- **On-screen display** – draw text and simple shapes over the 3D view.
- **XML configuration** – define machines and scene properties in one file.

## Directory layout

```
cncvis/
  tinygl/     - software OpenGL implementation
  libstlio/   - minimal STL loading library
  mxml/       - Mini-XML parser
  stb/        - subset of stb headers (image loading, etc.)
  machines/   - example machine configurations and STL models
```

## Building

The library is built using CMake and requires a C compiler with C99
support. The following commands build `libcncvis.a` and several TinyGL
sample programs:

```bash
cmake -S cncvis -B build
cmake --build build
```

This produces a static library and a few TinyGL based demos under `build/`.

## Configuration files

Each machine is described by an XML file. Assemblies form a tree where
children inherit the transformation of their parent. Actors reference
STL files and are attached to assemblies. Lights define the scene
illumination. See `machines/meca500/config.xml` for a complete example.

### Configuration reference

```
<config>
  <assemblies>
    <assembly name="base" parent="NULL">
      <origin x="0" y="0" z="0"/>
      <position x="0" y="0" z="0"/>
      <rotation x="0" y="0" z="0"/>
      <motion type="rotational" axis="Z" invert="no"/>
      <home>
        <position x="0" y="0" z="0"/>
        <rotation x="0" y="0" z="0"/>
      </home>
    </assembly>
    ...
  </assemblies>
  <actors>
    <actor name="base_part" assembly="base" stlFile="base.stl"/>
    ...
  </actors>
  <lights>
    <light id="GL_LIGHT0">
      <position x="200" y="1000" z="1000" w="1"/>
      <ambient r="0" g="0" b="0"/>
      <diffuse r="1" g="1" b="1"/>
      <specular r="1" g="1" b="1"/>
    </light>
    ...
  </lights>
</config>
```

- **Assemblies** describe movable joints or rigid groups and may reference
  other assemblies to form a hierarchy.
- **Actors** attach an STL model to an assembly and optionally specify a color.
- **Lights** follow the TinyGL style definitions and support spotlights
  and attenuation parameters.

## Using the API

Include `cncvis/api.h` and link with `libcncvis.a`. A minimal program
looks like:

```c
#include "cncvis/api.h"

int main(void) {
    if (cncvis_init("machines/meca500/config.xml") != 0)
        return 1;         // load machine and prepare renderer

    // render a single frame (normally done every loop)
    cncvis_render();

    cncvis_cleanup();
    return 0;
}
```

### Controlling the simulation

Use `ucncUpdateMotionByName()` to move joints by name or
`ucncUpdateMotion()` when you already have a pointer to an assembly.
After updating the motion values, call `cncvis_render()` again to redraw
the scene. The frame buffer contents can be accessed with
`ucncGetZBufferOutput()` for integration into GUI frameworks.

### Camera utilities

The library provides CAD-style controls:
`ucncCameraPan`, `ucncCameraOrbit`, `ucncCameraZoom`, `ucncCameraSetFrontView`
and related helpers. Map these to your input events (e.g. SDL or GLFW)
to implement interactive viewing.

## License

This project is released under the MIT License. See `LICENSE` for
full details.
