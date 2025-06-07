# Machine Simulation

This project is a simulation of a machine, which includes rendering assemblies, actors, and lights in a 3D scene. The simulation supports loading configurations from an XML file, using Mini-XML for parsing. The rendering is performed using TinyGL.

## Features

- **Hierarchical Machine Model**: Create complex machine structures with assemblies, joints, and actors
- **STL Model Rendering**: Visualize machine components using STL files
- **Lighting System**: Configure multiple light sources with ambient, diffuse, and specular components
- **Machine Control API**: Programmatically control machine joints and assemblies
- **Advanced CAD-like Camera Controls**: Professional camera interaction system with orbit, pan, zoom, and standard view presets
- **On-Screen Display (OSD)**: Overlay text information like machine positions and status
- **Performance Monitoring**: Built-in FPS counter and performance profiling
- **XML-based Configuration**: Define machines, assemblies, and scene properties in XML

## Build Instructions

1. Ensure you have CMake (version 3.10 or higher) installed on your system.
2. Clone the project.
3. Create a build directory and compile:

   ```bash
   mkdir -p build && cd build
   cmake ..
   make
   ```

4. The library will be built as a static library (`libcncvis.a`) that you can link to your projects.

## Machine Model System

The machine simulation uses a hierarchical component system to represent robotic arms, CNC machines, or other mechanical systems:

### Assembly Structure

- **Assemblies**: Represent machine parts that can move together (joints, links, etc.)
- **Actors**: 3D models loaded from STL files that make up the visual representation
- **Hierarchy**: Assemblies can have parent-child relationships, creating kinematic chains
- **Motion Types**: Support for rotational and linear motion along different axes

### Example Assembly Hierarchy

```
base (static)
└── link1 (rotates around Z)
    └── link2 (rotates around Z)
        └── link3 (rotates around Y)
            └── link4 (rotates around Y)
                └── link5 (rotates around Z)
                    └── link6 (rotates around Z)
```

### XML Configuration

The machine structure is defined using XML configuration:

```xml
<config>
  <assemblies>
    <assembly name="base" parent="meca500">
      <origin x="0.0" y="0.0" z="0.0"/>
      <position x="0.0" y="0.0" z="0.0"/>
      <rotation x="0.0" y="0.0" z="0.0"/>
      <color r="0.8" g="0.8" b="0.8"/>
      <motion type="rotational" axis="Z" invert="no"/>
      <home>
        <position x="0.0" y="0.0" z="0.0"/>
        <rotation x="0.0" y="0.0" z="0.0"/>
      </home>
    </assembly>
    <!-- More assemblies -->
  </assemblies>

  <actors>
    <actor name="base_part" assembly="base" stlFile="base.stl">
      <color r="0.8" g="0.8" b="0.8"/>
    </actor>
    <!-- More actors -->
  </actors>

  <lights>
    <light id="GL_LIGHT0" x="200" y="1000" z="1000.0">
      <ambient r="0" g="0" b="0"/>
      <diffuse r="1" g="1" b="1"/>
      <specular r="1" g="1" b="1"/>
    </light>
    <!-- More lights -->
  </lights>
</config>
```

## On-Screen Display (OSD)

The OSD system allows rendering text and UI elements directly on the screen:

### Features

- Text rendering with customizable color and size
- Support for aligned text (left, center, right)
- Background rectangles with alpha blending
- Status displays for machine position and state
- FPS counter

### Usage Example

```c
// Initialize the OSD in your setup code
osdInit(globalFramebuffer);
osdSetDefaultStyle(1.0f, 1.0f, 0.0f, 1.5f, 1); // Yellow text, 1.5x scale

// In your render function
void cncvis_render(void) {
    // After 3D rendering is complete...
    
    // Show machine position
    osdDrawTextf(10, 10, OSD_ALIGN_LEFT, "X: %.3f Y: %.3f Z: %.3f", 
                 machine_x, machine_y, machine_z);
    
    // Show machine status with background
    osdDrawRect(statusX - textWidth - 6, statusY - 2, textWidth + 12, textHeight + 4, 
                0.0f, 0.0f, 0.0f, 0.7f);
    osdDrawTextStyled("RUNNING\n45.2 FPS", statusX, statusY, OSD_ALIGN_RIGHT, &customStyle);
}
```

## CAD-like Camera Controls

The cncvis module provides a complete camera system with CAD-like controls, but leaves the actual input handling to the host application. The camera API allows any front-end (SDL, GLFW, custom UI, etc.) to implement these controls by calling the appropriate cncvis functions:

### Mouse Control API

```c
// Handle mouse motion events
void cncvis_handle_mouse_motion(int dx, int dy);

// Handle mouse wheel events
void cncvis_handle_mouse_wheel(int wheel_delta);

// These functions should be called by the host application when input events occur
```

### Camera Control API

```c
// Create and initialize a camera
ucncCamera* ucncCameraNew(float posX, float posY, float posZ, float upX, float upY, float upZ);

// Pan the camera view
void ucncCameraPan(ucncCamera *camera, float dx, float dy);

// Orbit the camera around the target point
void ucncCameraOrbit(ucncCamera *camera, float deltaYaw, float deltaPitch);

// Zoom the camera view
void ucncCameraZoom(ucncCamera *camera, float zoomDelta);

// Toggle between perspective and orthographic projection
void ucncCameraToggleProjection(ucncCamera *camera);

// Standard view presets
void ucncCameraSetFrontView(ucncCamera *camera);
void ucncCameraSetTopView(ucncCamera *camera);
void ucncCameraSetRightView(ucncCamera *camera);
void ucncCameraSetIsometricView(ucncCamera *camera);
void ucncCameraResetView(ucncCamera *camera);
```

### Recommended Control Scheme

Host applications using cncvis should implement the following control scheme for a standard CAD-like experience:

| Mouse Action | Function | Implementation |
|--------------|----------|----------------|
| Left-click drag | Pan | Call `ucncCameraPan(camera, dx, dy)` |
| Middle-click drag | Orbit | Call `ucncCameraOrbit(camera, dx, dy)` |
| Right-click drag | Zoom | Call `ucncCameraZoom(camera, dy)` |
| Mouse wheel | Zoom | Call `ucncCameraZoom(camera, wheel_delta)` |
| Shift + Actions | Precision | Reduce dx/dy values when Shift is held |

| Key | Function | Implementation |
|-----|----------|----------------|
| F1 | Front view | Call `ucncCameraSetFrontView(camera)` |
| F2 | Top view | Call `ucncCameraSetTopView(camera)` |
| F3 | Right view | Call `ucncCameraSetRightView(camera)` |
| F4 | Isometric view | Call `ucncCameraSetIsometricView(camera)` |
| F5 | Reset view | Call `ucncCameraResetView(camera)` |
| Space | Toggle projection | Call `ucncCameraToggleProjection(camera)` |
| WASD | Move target | Adjust camera->targetX/Y/Z values |
| QE | Move target up/down | Adjust camera->targetY value |
| 1-6 | Select machine parts | Call `ucncUpdateMotionByName()` for selected part |
| Arrow keys | Adjust part position | Call `ucncUpdateMotion()` with appropriate values |

### Example Integration

```c
// In your SDL/GLFW/etc. event handling code:

void handle_mouse_event(int button, int state, int x, int y) {
    static int last_x = 0, last_y = 0;
    
    if (state == MOUSE_DOWN) {
        last_x = x;
        last_y = y;
        return;
    }
    
    int dx = x - last_x;
    int dy = y - last_y;
    
    if (button == MOUSE_LEFT) {
        // Pan camera
        cncvis_handle_mouse_motion(dx, dy);
    }
    else if (button == MOUSE_MIDDLE) {
        // Orbit camera
        ucncCameraOrbit(globalCamera, dx * 0.5f, dy * 0.5f);
    }
    else if (button == MOUSE_RIGHT) {
        // Zoom camera
        ucncCameraZoom(globalCamera, dy * 0.1f);
    }
    
    last_x = x;
    last_y = y;
    
    // After changing camera, render a new frame
    cncvis_render();
}

void handle_key_event(int key) {
    switch(key) {
        case KEY_F1:
            ucncCameraSetFrontView(globalCamera);
            break;
        case KEY_F2:
            ucncCameraSetTopView(globalCamera);
            break;
        case KEY_SPACE:
            ucncCameraToggleProjection(globalCamera);
            break;
        // ... more key handlers ...
    }
    
    // After changing camera, render a new frame
    cncvis_render();
}
```

The cncvis module focuses on providing the camera system functionality, while leaving the specifics of input handling to the host application. This design allows for flexibility in how the camera controls are implemented across different UI frameworks.

## API Reference

### Machine Control API

```c
// Initialize the visualization system
int cncvis_init(const char *configFile);

// Render the current scene
void cncvis_render(void);

// Cleanup resources
void cncvis_cleanup(void);

// Find and update an assembly's motion by name
void ucncUpdateMotionByName(const char *assemblyName, float value);

// Update motion for a specific assembly
void ucncUpdateMotion(ucncAssembly *assembly, float value);

// Reset all assemblies to home position
void ucncSetAllAssembliesToHome(ucncAssembly *assembly);

// Load a new machine configuration
int ucncLoadNewConfiguration(const char *configFile);
```

### Camera API

```c
// Create a new camera with specified position and up vector
ucncCamera* ucncCameraNew(float posX, float posY, float posZ, float upX, float upY, float upZ);

// Toggle between orthographic and perspective projection
void ucncCameraToggleProjection(ucncCamera *camera);

// Pan the camera parallel to view plane
void ucncCameraPan(ucncCamera *camera, float dx, float dy);

// Orbit around target point
void ucncCameraOrbit(ucncCamera *camera, float deltaYaw, float deltaPitch);

// Zoom (adjust FOV or distance)
void ucncCameraZoom(ucncCamera *camera, float zoomDelta);

// Standard CAD view presets
void ucncCameraSetFrontView(ucncCamera *camera);
void ucncCameraSetTopView(ucncCamera *camera);
void ucncCameraSetRightView(ucncCamera *camera);
void ucncCameraSetIsometricView(ucncCamera *camera);
void ucncCameraResetView(ucncCamera *camera);
```

### OSD API

```c
// Initialize the OSD system
void osdInit(ZBuffer* frameBuffer);

// Set default text style
void osdSetDefaultStyle(float r, float g, float b, float scale, int spacing);

// Draw text with various options
void osdDrawText(const char* text, int x, int y, OSDTextAlign align);
void osdDrawTextStyled(const char* text, int x, int y, OSDTextAlign align, const OSDStyle* style);
void osdDrawTextf(int x, int y, OSDTextAlign align, const char* format, ...);

// Draw a rectangle (useful for backgrounds)
void osdDrawRect(int x, int y, int width, int height, float r, float g, float b, float alpha);
```

## Example Usage

```c
// Initialize the visualization system with a machine config
cncvis_init("machines/meca500/config.xml");

// Move joint 1 by 45 degrees
ucncUpdateMotionByName("link1", 45.0f);

// Move joint 2 by -20 degrees
ucncUpdateMotionByName("link2", -20.0f);

// Reset the machine to home position
ucncSetAllAssembliesToHome(globalScene);

// Render the updated scene (includes OSD with position and status)
cncvis_render();

// Cleanup when done
cncvis_cleanup();
```

## Utilities

The module includes various utility functions:

- FPS calculation and display
- Performance profiling
- Screen capture to PNG
- Assembly and light hierarchy inspection
- Background and ground plane rendering

## Dependencies

The cncvis module has the following dependencies, all included as subdirectories:
- TinyGL: Lightweight OpenGL implementation for rendering
- STL IO Library: For loading 3D models from STL files
- Mini-XML: For parsing XML configuration files
- STB Image: Header-only library for image loading/processing
- Standard math library

## Memory Management

The project is designed with careful memory management, and has been verified using Valgrind to ensure no memory leaks occur.

## License

This project is licensed under the MIT License.

## Contributions

Contributions are welcome! Please fork the repository and submit pull requests for any enhancements or bug fixes.

---
