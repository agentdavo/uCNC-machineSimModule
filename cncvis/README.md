# Machine Simulation

This project is a simulation of a machine, which includes rendering assemblies, actors, and lights in a 3D scene. The simulation supports loading configurations from an XML file, using Mini-XML for parsing. The rendering is performed using TinyGL.

## Features

- **Assembly and Actor Management**: Hierarchical structure for assemblies and actors, loaded from an XML configuration.
- **Lighting**: Supports multiple light sources with ambient, diffuse, and specular components.
- **Camera Controls**: Dynamic camera controls to orbit around the scene, allowing for custom angles and distances.
- **Rendering**: Utilizes TinyGL to render frames with depth testing and lighting enabled.
- **XML Configuration**: Supports loading assemblies, actors, and lights from an XML configuration file.

## Build Instructions

1. Ensure you have installed Mini-XML on your system.
2. Clone the project.
3. In the project directory, compile the source files:

   ```bash
   make
   ```

4. Run the simulation:

   ```bash
   ./render_robot
   ```

## Configuration

The configuration file is an XML file (`config.xml`) that defines assemblies, actors, and lights. Below is a sample structure:

```xml
<config>
  <assemblies>
    <assembly name="meca500" parent="NULL">
      <origin x="0.0" y="0.0" z="0.0"/>
      <position x="0.0" y="0.0" z="0.0"/>
      <rotation x="0.0" y="0.0" z="0.0"/>
      <color r="1.0" g="1.0" b="1.0"/>
    </assembly>
    <!-- More assemblies here -->
  </assemblies>

  <actors>
    <actor name="base_part" assembly="base" stlFile="base.stl">
      <color r="0.8" g="0.8" b="0.8"/>
    </actor>
    <!-- More actors here -->
  </actors>

  <lights>
    <light id="GL_LIGHT0" x="-500.0" y="500.0" z="1000.0">
      <ambient r="0.1" g="0.1" b="0.1"/>
      <diffuse r="0.5" g="0.5" b="0.5"/>
      <specular r="0.8" g="0.8" b="0.8"/>
    </light>
  </lights>
</config>
```

## Memory Management

The project is designed with careful memory management, and has been verified using Valgrind to ensure no memory leaks occur.

## Known Issues

- None reported. Feel free to open issues if you encounter bugs or have feature requests.

## License

This project is licensed under the MIT License.

## Contributions

Contributions are welcome! Please fork the repository and submit pull requests for any enhancements or bug fixes.

---
