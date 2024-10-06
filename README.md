# Universal CNC Machine Renderer

This project is a generic and flexible CNC machine renderer that allows you to visualize various CNC machines, such as the Meca500 robot, VMCs (Vertical Machining Centers), 5-axis machines, and more. By using a simple tab-delimited configuration file, you can define the components of a machine, their hierarchy, and transformations without modifying the core code.

## Features

- **Modular Code Structure**: Organized into separate modules for better maintainability and reusability.
- **Generic Machine Configuration**: Define any CNC machine using a simple tab-delimited text file.
- **Dynamic Camera Controls**: Render multiple frames with dynamic camera movement around the machine.
- **Lightweight Dependencies**: Uses standard C libraries and minimal external dependencies.
- **Supports STL Models**: Load and render components from STL files.
- **API for External Control**: External applications can interact with the renderer to manipulate machine components dynamically.

## Directory Structure

- `main.c`: Entry point of the application.
- `actor.h` / `actor.c`: Defines the `ucncActor` structure and related functions.
- `loader.h` / `loader.c`: Handles loading machines from configuration files.
- `renderer.h` / `renderer.c`: Contains rendering functions.
- `camera.h` / `camera.c`: Manages the camera.
- `light.h` / `light.c`: Manages lighting.
- `api.h` / `api.c`: Provides an API for external applications to control the renderer.
- `test_app.c`: Example external application demonstrating API usage.
- `meca500_config.txt`: Sample configuration file for the Meca500 robot.
- `Makefile`: Build script to compile the project.
- `stb/stb_image_write.h`: Header for image saving functionality.
- `TinyGL/`: Directory containing the TinyGL graphics library.
- `libstlio/`: Directory containing the STL file loading library.

## Prerequisites

- **C Compiler**: GCC or any compatible C compiler.
- **Libraries**:
  - [TinyGL](https://github.com/C-Chads/TinyGL): A small OpenGL subset implementation.
  - [libstlio](https://github.com/gabrielfaleiros/libstlio): A library for reading STL files.
  - [stb_image_write](https://github.com/nothings/stb): A single-file image writing library.

## Building the Project

### 1. Clone or Download Required Libraries

- **TinyGL**: Clone or download the TinyGL library and place the `TinyGL` directory in the project root.
- **libstlio**: Clone or download the libstlio library and place the `libstlio` directory in the project root.
- **stb_image_write.h**: Download `stb_image_write.h` and place it in a directory named `stb` in the project root.

### 2. Place the STL Files

Ensure that all required STL files for your machine (e.g., `meca500_base.stl`, `link1.stl`, etc.) are in the project root or specify the correct paths in the configuration file.

### 3. Build the Project Using Makefile

Open a terminal in the project directory and run:

```bash
make
```

This command will:

- Compile all source files (`.c` files) into object files (`.o` files).
- Create static libraries for TinyGL (`libtinygl.a`) and libstlio (`libstlio.a`).
- Link the object files and static libraries to create the executable `renderer` and `test_app`.

**Note:** If you encounter any errors, ensure that all include paths and library paths are correct, and that the required libraries are properly installed or placed in the specified directories.

### 4. Clean the Build (Optional)

To remove the compiled object files and the executables, run:

```bash
make clean
```

## Running the Program

### Render the Machine

Execute the compiled renderer binary:

```bash
./renderer
```

The program will render frames of the machine specified in the configuration file (`meca500_config.txt` by default) from different angles and save them as PNG images (e.g., `meca500_robot_frame_001.png`, `meca500_robot_frame_002.png`, etc.).

### Example Command-Line Arguments

You can override default parameters by providing command-line arguments:

```bash
./renderer [totalFrames] [rotationSpeed] [radius] [elevation]
```

- `totalFrames`: Number of frames to render (default: 36).
- `rotationSpeed`: Degrees to rotate the camera per frame (default: 10.0).
- `radius`: Distance of the camera from the origin (default: 400.0).
- `elevation`: Height of the camera (default: 100.0).

**Example:**

```bash
./renderer 60 6 500 150
```

This command will render 60 frames, rotating the camera 6 degrees per frame, with a radius of 500 units and an elevation of 150 units.

## API Usage

The project provides an API that allows external applications to interact with the renderer, enabling dynamic manipulation of machine components (axes) such as changing their positions and rotations.

### API Functions

#### Initialization and Shutdown

- **Initialize the Rendering System**

  ```c
  int ucncInitialize(const char *configFilePath);
  ```

  - **Parameters:**
    - `configFilePath`: Path to the machine configuration file.
  - **Returns:**
    - `1` on success, `0` on failure.

- **Shutdown the Rendering System**

  ```c
  void ucncShutdown();
  ```

#### Rendering

- **Render the Current Scene**

  ```c
  void ucncRender(const char *outputFilename);
  ```

  - **Parameters:**
    - `outputFilename`: Filename for the output image (e.g., "output.png").

#### Axis Manipulation

- **Set the Rotation of an Axis**

  ```c
  int ucncSetAxisRotation(const char *axisName, float rotation[3]);
  ```

  - **Parameters:**
    - `axisName`: Name of the axis (e.g., "J1", "X", "Z1").
    - `rotation`: Array containing rotation angles in degrees around the X, Y, and Z axes.
  - **Returns:**
    - `1` on success, `0` on failure.

- **Get the Rotation of an Axis**

  ```c
  int ucncGetAxisRotation(const char *axisName, float rotation[3]);
  ```

  - **Parameters:**
    - `axisName`: Name of the axis.
    - `rotation`: Array to store the current rotation angles.
  - **Returns:**
    - `1` on success, `0` on failure.

- **Set the Position of an Axis**

  ```c
  int ucncSetAxisPosition(const char *axisName, float position[3]);
  ```

  - **Parameters:**
    - `axisName`: Name of the axis.
    - `position`: Array containing the new position coordinates relative to the parent.
  - **Returns:**
    - `1` on success, `0` on failure.

- **Get the Position of an Axis**

  ```c
  int ucncGetAxisPosition(const char *axisName, float position[3]);
  ```

  - **Parameters:**
    - `axisName`: Name of the axis.
    - `position`: Array to store the current position coordinates.
  - **Returns:**
    - `1` on success, `0` on failure.

#### Camera Control

- **Set Camera Parameters**

  ```c
  void ucncSetCameraPosition(float position[3]);
  void ucncSetCameraTarget(float target[3]);
  void ucncSetCameraUp(float up[3]);
  ```

  - **Parameters:**
    - `position`: Array containing the new camera position.
    - `target`: Array containing the new camera target point.
    - `up`: Array containing the new camera up vector.

### Example External Application

Below is an example of an external application (`test_app.c`) that utilizes the API to manipulate the Meca500 robot's joints and render the scene.

```c
// test_app.c

#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // Initialize the rendering system with the configuration file
    if (!ucncInitialize("meca500_config.txt")) {
        fprintf(stderr, "Failed to initialize rendering system.\n");
        return EXIT_FAILURE;
    }

    // Set initial rotations for axes
    float rotationJ1[3] = {0.0f, 0.0f, 45.0f};  // Rotate J1 by 45 degrees around Z-axis
    float rotationJ2[3] = {0.0f, -30.0f, 0.0f}; // Rotate J2 by -30 degrees around Y-axis

    // Apply rotations using the API
    if (!ucncSetAxisRotation("J1", rotationJ1)) {
        fprintf(stderr, "Failed to set rotation for axis J1.\n");
    }

    if (!ucncSetAxisRotation("J2", rotationJ2)) {
        fprintf(stderr, "Failed to set rotation for axis J2.\n");
    }

    // Render the scene to an image
    ucncRender("meca500_rotated.png");

    printf("Rendered image saved as 'meca500_rotated.png'\n");

    // Shutdown the rendering system
    ucncShutdown();

    return EXIT_SUCCESS;
}
```

### Compiling the Example Application

The `Makefile` has been updated to include the `test_app`. To compile it, run:

```bash
make test_app
```

This command will create an executable named `test_app`.

### Running the Example Application

Execute the compiled example application:

```bash
./test_app
```

This will:

1. Initialize the rendering system using `meca500_config.txt`.
2. Set the rotations for axes "J1" and "J2".
3. Render the scene and save it as `meca500_rotated.png`.
4. Shutdown the rendering system.

## Configuration File Format

The machine is defined in a tab-delimited text file. Each line represents an actor (component) of the machine.

### Fields

- **Name**: Unique identifier of the actor.
- **Parent**: Name of the parent actor. Use `NULL` or leave empty if there is no parent.
- **STLFile**: Filename of the STL model for the actor.
- **OriginX**, **OriginY**, **OriginZ**: Local origin coordinates.
- **PosX**, **PosY**, **PosZ**: Position relative to the parent.
- **RotX**, **RotY**, **RotZ**: Rotation in degrees around the X, Y, and Z axes.
- **ColorR**, **ColorG**, **ColorB**: RGB color components (values between 0.0 and 1.0).
- **IsAxis**: Indicates if the actor represents an axis (1 for yes, 0 for no).
- **AxisName**: The name of the axis (e.g., "X", "Y", "Z1", "J1").

### Sample (`meca500_config.txt`)

```
# Meca500 Configuration
# Name	Parent	STLFile	OriginX	OriginY	OriginZ	PosX	PosY	PosZ	RotX	RotY	RotZ	ColorR	ColorG	ColorB	IsAxis	AxisName
Base	NULL	meca500_base.stl	0.0	0.0	0.0	0.0	0.0	0.0	0.0	0.0	0.0	1.0	1.0	1.0	0	NULL
Link1	Base	link1.stl	0.0	0.0	135.0	0.0	0.0	0.0	0.0	0.0	0.0	0.9	0.9	0.9	1	J1
Link2	Link1	link2.stl	0.0	0.0	135.0	0.0	0.0	0.0	0.0	0.0	0.0	0.8	0.8	0.8	1	J2
Link3	Link2	link3.stl	135.0	0.0	135.0	0.0	0.0	0.0	0.0	0.0	0.0	0.7	0.7	0.7	1	J3
Link4	Link3	link4.stl	173.0	0.0	50.0	0.0	0.0	0.0	0.0	0.0	0.0	0.6	0.6	0.6	1	J4
Link5	Link4	link5.stl	173.0	0.0	15.0	0.0	0.0	0.0	0.0	0.0	0.0	0.5	0.5	0.5	1	J5
SpindleAssy	Link5	spindle_assy.stl	173.0	0.0	-55.0	0.0	0.0	0.0	0.0	0.0	0.0	0.4	0.4	0.4	1	J6
```

**Notes:**

- The **IsAxis** field is set to `1` for actors that represent movable axes (e.g., "J1", "J2", etc.) and `0` otherwise.
- **AxisName** provides a unique identifier for each axis, which is used in the API to manipulate them.

## Customizing and Extending

To render a different machine or extend the functionality, follow these steps:

### 1. Create STL Models

Prepare STL files for each component of your machine. Ensure that each component is properly modeled and optimized for rendering.

### 2. Create a Configuration File

Define the components, hierarchy, and transformations in a new tab-delimited text file. Use the `meca500_config.txt` as a template.

### 3. Update the Configuration Path

Modify the path in `main.c` or utilize the API in an external application to specify the new configuration file.

### 4. Use the API for Dynamic Control

External applications can use the provided API to dynamically change the positions and rotations of machine components. Refer to the **API Usage** section for details and example applications.

## Dependencies

- **TinyGL**: For rendering 3D graphics. Place the `TinyGL` directory in the project root.
- **libstlio**: For loading STL files. Place the `libstlio` directory in the project root.
- **stb_image_write.h**: For saving rendered images as PNG files. Place `stb_image_write.h` in the `stb` directory.

## License

This project is open-source and available under the [MIT License](LICENSE).

## Acknowledgments

- **TinyGL**: [https://github.com/C-Chads/TinyGL](https://github.com/C-Chads/TinyGL)
- **libstlio**: [https://github.com/gabrielfaleiros/libstlio](https://github.com/gabrielfaleiros/libstlio)
- **stb_image_write**: [https://github.com/nothings/stb](https://github.com/nothings/stb)

---

**Notes:**

- **API Integration**: The API is designed to be simple and straightforward. External applications need to include `api.h` and link against the compiled `api.c` along with other required modules.
  
- **Axis Naming**: Ensure that axis names are unique within the configuration file to prevent conflicts when manipulating them via the API.

- **Thread Safety**: The current API implementation is not thread-safe. If you plan to use it in a multi-threaded environment, consider adding appropriate synchronization mechanisms.

- **Extending the API**: You can add more functions to the API as needed, such as functions to animate movements, change colors, or manipulate other properties of the machine components.

- **Error Handling**: API functions return `1` on success and `0` on failure. You can extend this to use more detailed error codes or messages if necessary.

- **External Language Bindings**: If you need to interact with the API from other programming languages (e.g., Python, Java), consider creating bindings or using inter-process communication (IPC) mechanisms.

---