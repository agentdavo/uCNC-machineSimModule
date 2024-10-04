```markdown
# CNC Machine Rendering Framework

A high-performance CNC machine rendering framework for robot arms, implemented in C using GNU99 and the enhanced TinyGL by Fabrice Bellard. This framework enables you to load STL models, assemble them hierarchically, and render high-quality images with dynamic camera controls and comprehensive performance profiling.

## 🚀 Features

- **Modular Architecture:** Structured organization for Actors, Assemblies, and Camera.
- **STL File Support:** Load and render STL files using `libstlio`.
- **Hierarchical Assembly:** Easily assemble complex robot arms from individual parts.
- **Dynamic Camera Controls:** Orbit the camera around the scene to create smooth animations.
- **Performance Profiling:** Detailed breakdown of rendering times for optimization.
- **Frame Buffer Reuse:** Efficient rendering by reusing the framebuffer across frames.
- **Image Export:** Save rendered frames as high-quality PNG images using `stb_image_write`.
- **Multithreading Support:** Utilize OpenMP for accelerated rendering and post-processing.
- **Enhanced TinyGL Features:**
  - **SIMD Acceleration:** Improved vertex processing speed with `alignas` support.
  - **Additional OpenGL Functions:** Extended functionality including `glDeleteList`, `glSetEnableSpecular`, `glDrawText`, and more.
  - **Post-Processing:** Fast, multithreaded post-processing with `glPostProcess()`.
  - **Buffer Management:** Server-side buffers with `glGenBuffers`, `glBindBuffer`, and related functions.
  - **Improved Safety:** Comprehensive `glGetError()` functionality and memory leak fixes.
  - **Customizable Texture Sizes:** Compile-time options to set texture sizes as powers of two.

## 📋 Requirements

- **Compiler:** GCC supporting GNU99.
- **Libraries:**
  - [TinyGL](https://github.com/C-Chads) by Fabrice Bellard (Enhanced Fork by C-Chads)
  - [libstlio](https://github.com/Linden/libstlio) for STL file handling
  - [stb_image_write](https://github.com/nothings/stb) for image exporting
  - [OpenMP](https://www.openmp.org/) for multithreading support
- **Headers and Libraries:** Ensure all necessary headers and libraries are installed and accessible.

## 🛠 Installation

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/agentdavo/uCNC-machineSimModule.git
   cd cnc-rendering-framework
   ```

2. **Install Dependencies:**
   - **TinyGL:**
     Follow the installation instructions from the [TinyGL repository](https://github.com/fogleman/TinyGL). Ensure you are using the enhanced fork by C-Chads for additional features.
     
   - **libstlio:**
     Follow the installation instructions from the [libstlio repository](https://github.com/Linden/libstlio).
     
   - **stb_image_write:**
     The `stb_image_write.h` file is included in the repository. Ensure it's in the include path.

3. **Compile TinyGL:**
   ```bash
   cd path/to/TinyGL/src
   gcc -O3 -fopenmp -c *.c
   ar rcs libTinyGL.a *.o
   cp libTinyGL.a ../lib
   ```

4. **Compile the CNC Rendering Framework:**
   ```bash
   cd ../../
   gcc -O3 -fopenmp -o render_robot render_robot.c -L./lib -lTinyGL -lm
   ```
   - `-O3`: Enables high-level optimizations.
   - `-fopenmp`: Enables OpenMP for multithreading support.
   - `-L./lib`: Links against the TinyGL library.
   - `-lm`: Links the math library required for mathematical functions.

## 🎮 Usage

Execute the compiled program with optional command-line arguments to customize the rendering process.

```bash
./render_robot [totalFrames] [rotationSpeed] [radius] [elevation]
```

### 📄 Command-Line Arguments

1. **`totalFrames`** (optional):  
   Number of frames to render for a full 360-degree rotation.  
   *Default:* `36`

2. **`rotationSpeed`** (optional):  
   Degrees to rotate the camera per frame.  
   *Default:* `10.0`

3. **`radius`** (optional):  
   Distance of the camera from the origin (center of the robot arm).  
   *Default:* `400.0`

4. **`elevation`** (optional):  
   Height of the camera above the ground plane.  
   *Default:* `100.0`

### 📚 Examples

- **Default Settings:**
  ```bash
  ./render_robot
  ```
  Renders 36 frames, rotating 10 degrees per frame, with a camera radius of 400 units and elevation of 100 units.

- **Custom Settings:**
  ```bash
  ./render_robot 60 6 500 150
  ```
  Renders 60 frames, rotating 6 degrees per frame, with a camera radius of 500 units and elevation of 150 units.

## 🖼 Output

- **Rendered Images:**  
  PNG images named `meca500_robot_frame_001.png` to `meca500_robot_frame_036.png` (or according to the specified `totalFrames`).

- **Profiling Information:**  
  Detailed timing information is printed to the console for each frame, including camera setup, scene rendering, image saving, and total frame time. After all frames are rendered, a summary of the profiling statistics is displayed.

![Example Image](meca.png)

### 📊 Sample Console Output

```
Frame 001: meca500_robot_frame_001.png | Camera Setup: 0.25 ms | Scene Render: 50.30 ms | Image Save: 30.45 ms | Total: 81.00 ms
Frame 002: meca500_robot_frame_002.png | Camera Setup: 0.20 ms | Scene Render: 50.10 ms | Image Save: 30.50 ms | Total: 80.80 ms
...
=== Performance Profiling Summary ===
Total Frames Rendered: 36

Camera Setup Time (ms):
  Total: 9.00
  Average: 0.25
  Min: 0.20
  Max: 0.30

Scene Render Time (ms):
  Total: 1800.00
  Average: 50.00
  Min: 49.50
  Max: 50.50

Image Save Time (ms):
  Total: 1096.20
  Average: 30.45
  Min: 30.00
  Max: 31.00

Total Frame Time (ms):
  Total: 2700.00
  Average: 75.00
  Min: 74.50
  Max: 75.50
====================================
```

## 🧹 Cleanup

The program automatically frees allocated memory and closes the framebuffer upon completion. Ensure that all STL files are correctly loaded to prevent memory leaks.

## 🐞 Troubleshooting

- **Failed to Load STL Files:**
  - Ensure that the STL files are present in the working directory.
  - Verify file permissions and paths.

- **Framebuffer Initialization Failure:**
  - Check that TinyGL is correctly installed and configured.
  - Ensure that your system meets the necessary requirements for framebuffer operations.

- **Image Saving Issues:**
  - Verify that the `stb_image_write.h` is correctly included and accessible.
  - Ensure there is sufficient disk space and write permissions in the output directory.

- **Multithreading Issues:**
  - Ensure that the compiler supports OpenMP (`-fopenmp` flag).
  - Verify that your system's hardware supports multithreading if leveraging OpenMP features.

## 📝 Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your enhancements or bug fixes.

## 📄 License

This project is licensed under the [MIT License](LICENSE).

## 📚 Additional Resources

- **TinyGL Documentation:**  
  Refer to the [TinyGL README](https://github.com/C-Chads) for more detailed information on TinyGL's features and usage.

- **libstlio Documentation:**  
  Visit the [libstlio repository](https://github.com/Linden/libstlio) for detailed instructions on handling STL files.

- **stb_image_write Documentation:**  
  Explore the [stb_image_write repository](https://github.com/nothings/stb) for more information on image exporting capabilities.

## 🔧 Enhancements Using Enhanced TinyGL C-Chads Features

Leveraging the enhanced features of TinyGL can further optimize and expand the capabilities of the CNC Machine Rendering Framework:

- **SIMD Acceleration:**
  - Enable `alignas` in `zfeatures.h` to take advantage of SIMD optimizations, significantly improving vertex processing speed.

- **Multithreading with OpenMP:**
  - Utilize OpenMP to parallelize rendering tasks, such as `glDrawPixels` and `glPostProcess()`, to maximize performance on multi-core processors.

- **Advanced Buffer Management:**
  - Implement server-side buffers using `glGenBuffers`, `glBindBuffer`, and related functions to manage vertex and texture data more efficiently.

- **Post-Processing Effects:**
  - Use `glPostProcess()` to apply custom post-processing effects to rendered frames, enhancing visual quality or adding specific graphical features.

- **Extended OpenGL Functions:**
  - Incorporate additional OpenGL functions like `glDrawText`, `glSetEnableSpecular`, and `glGetTexturePixmap` to add more functionality, such as text rendering and advanced lighting controls.

- **Error Handling and Debugging:**
  - Enable comprehensive `glGetError()` functionality to aid in debugging and ensure rendering correctness.

### 📌 Example: Using `glDrawText` for Annotations

Enhance your rendered images by adding text annotations directly onto the framebuffer.

```c
// Before rendering the scene
glColor3f(1.0f, 1.0f, 1.0f); // Set text color to white
glDrawText("Robot Arm", 10, 10, 0xFFFFFFFF);
```

### 📌 Example: Implementing Custom Post-Processing

Apply a custom post-processing effect to invert the colors of the rendered frame.

```c
GLuint invertColors(GLint x, GLint y, GLuint pixel, GLushort z) {
    unsigned char r = (pixel >> 24) & 0xFF;
    unsigned char g = (pixel >> 16) & 0xFF;
    unsigned char b = (pixel >> 8) & 0xFF;
    unsigned char a = pixel & 0xFF;
    return (255 - r) << 24 | (255 - g) << 16 | (255 - b) << 8 | a;
}

int main(int argc, char *argv[]) {
    // ... [Initialization code]

    // Set the post-processing function
    glPostProcess(invertColors);

    // ... [Rendering loop]
}
```

## 📈 Performance Optimization Tips

- **Enable OpenMP:**  
  Ensure that the `-fopenmp` flag is used during compilation to activate multithreading features.

- **SIMD Optimizations:**  
  Enable `alignas` in `zfeatures.h` to allow TinyGL to utilize SIMD instructions, boosting vertex processing speed.

- **Minimize State Changes:**  
  Reduce the number of OpenGL state changes (e.g., binding buffers, setting colors) within the rendering loop to enhance performance.

- **Batch Rendering Calls:**  
  Group similar rendering operations together to take full advantage of TinyGL's optimized rasterizer and avoid redundant computations.

## 🤝 Acknowledgments

- **[Fabrice Bellard](https://bellard.org/):** Creator of the original TinyGL.
- **[C-Chads](https://github.com/C-Chads):** Maintainers of the enhanced TinyGL fork with additional features and optimizations.
- **[stb Libraries](https://github.com/nothings/stb):** For providing `stb_image_write.h`.
- **[libstlio](https://github.com/Linden/libstlio):** For facilitating STL file handling.

## 🗺 Roadmap

- **Texture Filtering:**  
  Implement mipmapping and various texture filtering techniques for higher-quality textures.

- **Advanced Lighting Models:**  
  Expand lighting capabilities beyond specular to include more realistic shading effects.

- **User Interface Integration:**  
  Incorporate more advanced GUI features using the included OpenIMGUI standard.

- **Extended OpenGL Support:**  
  Gradually implement additional OpenGL 1.1 features to increase compatibility and functionality.

- **Documentation Enhancements:**  
  Provide more comprehensive documentation and examples to aid new users in utilizing the framework's full potential.

---
