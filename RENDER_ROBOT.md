## **Overview**

The application is designed to render a 3D scene of a CNC machine (specifically, a Meca500 robot arm) using TinyGL, a lightweight OpenGL implementation. The code is organized into several modular components:

1. **Actors**: Represent individual 3D models loaded from STL files.
2. **Assemblies**: Hierarchical structures that group actors and other assemblies.
3. **Scene**: The global assembly that contains all elements to be rendered.
4. **Camera**: Defines the viewpoint from which the scene is rendered.
5. **Lights**: Illuminate the scene to enhance visual realism.

---

## **Key Components**

### **1. Actors (`ucncActor`)**

An **actor** represents a single 3D object in the scene. It encapsulates the following:

- **Transformation Properties**:
  - **Origin** (`originX`, `originY`, `originZ`): The local origin point of the actor.
  - **Position** (`positionX`, `positionY`, `positionZ`): The actor's position in world coordinates.
  - **Rotation** (`rotationX`, `rotationY`, `rotationZ`): The actor's rotation around each axis in degrees.

- **Visual Properties**:
  - **Color** (`colorR`, `colorG`, `colorB`): The RGB color of the actor.

- **Geometry Data**:
  - **STL Data Buffer** (`stlObject`): The raw data loaded from an STL file.
  - **Triangle Count** (`triangleCount`): Number of triangles in the STL model.
  - **Stride** (`stride`): The size of each triangle data structure in bytes.

**Functions Associated with Actors**:

- **Creation**: `ucncActor* ucncActorNew(const char *stlFile);`
  - Loads an STL file and initializes an actor with default transformation and visual properties.

- **Rendering**: `void ucncActorRender(ucncActor *actor);`
  - Applies transformations and renders the actor's geometry using OpenGL functions.

- **Cleanup**: `void ucncActorFree(ucncActor *actor);`
  - Frees the memory allocated for the actor, including the STL data buffer.

---

### **2. Assemblies (`ucncAssembly`)**

An **assembly** is a hierarchical structure that can contain multiple actors and other assemblies. This allows for complex models to be constructed from simpler components.

- **Transformation Properties**:
  - Similar to actors, assemblies have origin, position, and rotation properties that affect all contained actors and assemblies.

- **Components**:
  - **Actors Array** (`actors`): An array of pointers to `ucncActor` objects.
  - **Assemblies Array** (`assemblies`): An array of pointers to child `ucncAssembly` objects.
  - **Counts**: `actorCount` and `assemblyCount` track the number of actors and assemblies.

**Functions Associated with Assemblies**:

- **Creation**: `ucncAssembly* ucncAssemblyNew();`
  - Initializes an empty assembly with default transformation properties.

- **Adding Actors**: `void ucncAssemblyAddActor(ucncAssembly *assembly, ucncActor *actor);`
  - Adds an actor to the assembly's actors array.

- **Adding Assemblies**: `void ucncAssemblyAddAssembly(ucncAssembly *parent, ucncAssembly *child);`
  - Adds a child assembly to the parent assembly's assemblies array.

- **Rendering**: `void ucncAssemblyRender(ucncAssembly *assembly);`
  - Applies transformations and recursively renders all contained actors and assemblies.

- **Cleanup**: `void ucncAssemblyFree(ucncAssembly *assembly);`
  - Recursively frees all actors and assemblies within the assembly.

---

### **3. Scene**

The **scene** is the top-level assembly that encompasses all elements to be rendered. It is represented by the global variable:

```c
ucncAssembly *globalScene = NULL;
```

- The scene is initialized in the `main` function.
- Assemblies and actors are added to the scene to build the complete 3D model.
- The scene is rendered by calling `ucncAssemblyRender(globalScene);` during the rendering process.

---

### **4. Camera (`ucncCamera`)**

The **camera** defines the viewpoint from which the scene is rendered.

- **Position and Orientation**:
  - **Position** (`positionX`, `positionY`, `positionZ`): The location of the camera in world space.
  - **Target** (`targetX`, `targetY`, `targetZ`): The point in world space the camera is looking at.
  - **Up Vector** (`upX`, `upY`, `upZ`): Defines the upward direction relative to the camera's orientation.

- **Zoom Level** (`zoomLevel`): A scaling factor to zoom in or out of the scene.

**Functions Associated with Camera**:

- **Creation**: `ucncCamera* ucncCameraNew(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);`
  - Initializes the camera with specified position and target.

- **Applying Transformations**: `void ucncCameraApply(ucncCamera *camera);`
  - Computes the view matrix based on the camera's position and orientation and applies it to the OpenGL context.

- **Helper Functions**:
  - **Normalization**: `void normalize(float v[3]);` Normalizes a 3D vector.
  - **Cross Product**: `void cross_product(const float a[3], const float b[3], float result[3]);` Computes the cross product of two vectors.

- **Cleanup**: `void ucncCameraFree(ucncCamera *camera);`
  - Frees the memory allocated for the camera.

---

### **5. Lights (`ucncLight`)**

Lights illuminate the scene to create realistic shading and highlights.

- **Properties**:
  - **Light ID** (`lightId`): Specifies which OpenGL light to use (e.g., `GL_LIGHT0`).
  - **Position** (`position[4]`): The position of the light in the scene.
  - **Ambient Color** (`ambient[4]`): The ambient color component of the light.
  - **Diffuse Color** (`diffuse[4]`): The diffuse color component.
  - **Specular Color** (`specular[4]`): The specular color component.

**Functions Associated with Lights**:

- **Creation**: `ucncLight* ucncLightNew(GLenum lightId, float posX, float posY, float posZ, float ambient[], float diffuse[], float specular[]);`
  - Initializes a new light with specified properties.

- **Adding Light to Scene**: `void addLight(ucncLight *light);`
  - Enables the light in the OpenGL context and sets its properties.

- **Updating Light Properties**: `void setLight(ucncLight *light);`
  - Updates the light's properties in the OpenGL context (useful if the light moves or changes during runtime).

---

## **Rendering Process**

The rendering process involves several steps to display the scene from the camera's perspective with proper lighting:

1. **Clearing Buffers**:
   - The color and depth buffers are cleared using `glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);`.

2. **Applying Camera Transformations**:
   - The camera's view matrix is computed and applied using `ucncCameraApply(globalCamera);`.

3. **Setting Up Lighting**:
   - Lighting is enabled, and the light source is added to the scene using `addLight(globalLight);`.

4. **Rendering the Ground Plane**:
   - A simple ground plane is rendered using `CreateGround(float sizeX, float sizeY);`.

5. **Rendering the Scene**:
   - The global scene assembly is rendered recursively using `ucncAssemblyRender(globalScene);`.
     - This involves rendering all actors and child assemblies with their respective transformations.

6. **Saving the Framebuffer**:
   - The rendered image is saved to a file using `saveFramebufferAsImage(globalFramebuffer, outputFilename, framebufferWidth, framebufferHeight);`.

---

## **Main Program Flow**

1. **Initialization**:
   - Framebuffer, camera, scene, and light are initialized.
   - The Meca500 robot parts are loaded as actors from STL files and organized into assemblies.
   - Origins and positions are set for each assembly to correctly position the robot parts.

2. **Rendering Loop**:
   - For each frame in the total number of frames:
     - **Camera Update**:
       - The camera's position is updated to orbit around the scene for dynamic viewing.
     - **Scene Rendering**:
       - The scene is rendered from the current camera position.
     - **Profiling**:
       - Timing information is recorded for performance analysis.

3. **Cleanup**:
   - All allocated memory for actors, assemblies, camera, and light is freed.
   - The OpenGL context and framebuffer are closed.

---

## **Additional Components**

### **Ground Plane (`CreateGround`)**

- **Function**: `void CreateGround(float sizeX, float sizeY);`
- **Purpose**: Renders a flat plane to represent the ground in the scene.
- **Implementation**:
  - Uses `glBegin(GL_QUADS)` to draw a quadrilateral.
  - Applies a normal facing upwards for correct lighting.
  - Sets a light grey color for the ground.

### **Performance Profiling**

- **Structures**:
  - `FrameTiming`: Records timing information for each frame.
  - `ProfilingStats`: Aggregates timing data across all frames.

- **Functions**:
  - `initProfilingStats(ProfilingStats *stats);`: Initializes profiling statistics.
  - `updateProfilingStats(ProfilingStats *stats, FrameTiming *frameTiming);`: Updates stats with data from the current frame.
  - `printProfilingStats(ProfilingStats *stats, int totalFrames);`: Prints a summary of the performance metrics.

- **Usage**:
  - Used to measure and optimize the performance of the rendering process.

---

## **OpenGL Integration**

- **Vertex and Normal Specification**:
  - Vertices are specified using `glVertex3f`.
  - Normals are specified using `glNormal3f` for correct lighting calculations.

- **Material Properties**:
  - Actors set material properties using `glMaterialfv` to interact properly with lighting.
  - Material properties include ambient, diffuse, specular colors, and shininess.

- **Transformation Functions**:
  - `glTranslatef`, `glRotatef`, and `glScalef` are used to apply transformations to actors and assemblies.

- **Matrix Stack**:
  - `glPushMatrix` and `glPopMatrix` are used to save and restore transformation states when rendering hierarchical objects.

- **Lighting Functions**:
  - `glEnable(GL_LIGHTING)` and `glEnable(GL_LIGHT0)` activate lighting.
  - `glLightfv` sets the light's properties.

---

## **Understanding the Hierarchical Structure**

- **Actors and Assemblies**:
  - Each actor represents a single part of the robot arm.
  - Assemblies group these actors into meaningful components (e.g., links, joints).
  - Assemblies can contain other assemblies, allowing for nested hierarchies.

- **Transformation Hierarchy**:
  - Transformations applied to an assembly affect all its child actors and assemblies.
  - This mimics real-world mechanical relationships (e.g., moving a robot arm joint moves all subsequent parts).

- **Scene Composition**:
  - The global scene is the root assembly containing all other assemblies.
  - By organizing the robot arm into assemblies, we can easily manipulate complex movements and positions.

---

## **Camera Dynamics**

- **Orbiting Camera**:
  - The camera orbits around the origin to provide a full 360-degree view of the scene.
  - This is achieved by updating the camera's position in a circular path using trigonometric functions (`cos` and `sin`).

- **Elevation and Radius**:
  - The camera's elevation and distance from the origin can be adjusted to change the viewing angle and zoom level.

---

## **Lighting Effects**

- **Light Position**:
  - The light is positioned above and to the side of the scene to create shadows and highlights.

- **Color Components**:
  - **Ambient Light**: Simulates indirect light scattered in the environment.
  - **Diffuse Light**: Simulates direct light shining on surfaces.
  - **Specular Light**: Creates shiny highlights on reflective surfaces.

- **Material Interaction**:
  - The actors' material properties determine how they reflect light.
  - By adjusting material properties, you can simulate different surface characteristics (e.g., matte vs. glossy).

---

## **Usage Notes**

- **STL Files**:
  - Ensure that the STL files for the robot parts are present in the working directory.

- **Adjusting Parameters**:
  - The number of frames, rotation speed, camera radius, and elevation can be adjusted via command-line arguments.

- **Extensibility**:
  - The code is modular, allowing you to add more actors, assemblies, or modify existing ones.
  - Additional lights can be added to the scene by creating more `ucncLight` objects.

---