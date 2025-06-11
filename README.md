# Virtual Museum

# Demonstration Video

//VIDEO GELECEK

*Click the thumbnail above to watch the demonstration video.*

# Documentation

All formal documentation related to this project is organized under the `docs/` directory. This includes:

**[Project Report](./docs/VIRTUAL MUSEUM REPORT.pdf)**  
The complete academic report detailing the design, implementation, testing, and evaluation of the Virtual Museum system.

Class Diagram  
UML class diagram showing the system's object-oriented structure.

Software Architecture  
Software architecture diagram illustrating subsystem interactions.

Method Descriptions  
Tabulated documentation of key class methods and their responsibilities.



# Project Overview

Virtual Museum is an interactive 3D application developed using C++ and OpenGL 3.3, aiming to simulate a virtual museum environment. In this project, users navigate a 3D room containing five Blender-modeled artifacts using a mobile robot equipped with a scanning arm.

Key features include:
- Realistic lighting setup with one main directional light, two ambient point lights, and five artifact-specific spotlights
- Manual and automatic robot navigation
- Scanning system with pop-up information displays
- ImGui-based user interface for robot and lighting control

The project was developed as part of a computer graphics course by students from Çukurova University, Computer Engineering Department:

- Yunus Emre Erten – 2021556030  
- Veysel Genç – 2021556031  
- Zeliha İnan – 2021556037

All models, textures, and source code were created and implemented by the team from scratch, adhering to open-source software principles and academic integrity standards.

# Installation and Execution Instructions

1. Clone the repository:

```bash
git clone https://github.com/yuemco/virtual-adana-museum-v2-team-7.git
```

2. Open the `virtual-adana-museum-v2-team-7.sln` file using Visual Studio 2022.

3. Make sure the following libraries are linked properly in the project:
   - GLAD
   - GLFW
   - GLM
   - ImGui

4. Set the build configuration to `Debug x64` and run the project. 

# Project Directory Structure

```txt
virtual-adana-museum-v2-team-7/
├── assets/               → Models and textures
├── docs/                 → Diagrams
├── Libraries/            → External dependencies
│   ├── include/
│   ├── lib/
│   └── imgui/
├── main.cpp
├── Project1.sln
├── Project1.vcxproj
├── Project1.vcxproj.filters
├── Project1.vcxproj.user
├── README.md             
```

# Controls

**Keyboard:**

- W / A / S / D: Move the robot manually
- E / Q: Rotate the robot manually
- Mouse Movement: Rotate the camera
- Shift + Mouse Movement: Panning 
- Scroll: Zoom in/out  

## Control Panel Overview

### Robot Controls

- **Auto Mode Checkbox**  
  Enables or disables autonomous movement mode. When checked, the robot moves and performs actions without manual input.

- **Manual Movement Buttons**  
  - `Left`, `Right`, `Forward`, `Back`: Manually moves the robot in the respective direction.
  - `Rotate Left (Q)`, `Rotate Right (E)`: Rotates the robot on its vertical axis using keyboard shortcuts or button clicks.

- **Arm Angle Slider**  

  A horizontal slider in the user interface allows control over the vertical angle of the robot’s arm. The slider value ranges from 0.000 to 1.000, which is internally mapped to an actual angle between 0° and 100°. When the arm angle is within the range of 60° to 100°, the robot is allowed to perform a scanning operation. If the arm angle is below 60°, scanning is disabled, simulating a lowered or idle arm position. This mechanism enhances the realism of the robot’s behavior by explicitly requiring the arm to be raised before initiating a scan.
---

### Light Settings

- **Light 1 Intensity / Light 2 Intensity / Main Light Intensity**  
  These sliders adjust the brightness level of the corresponding light sources in the scene. The values are floating-point numbers between 0.000 (off) and 1.000 (maximum intensity).

- **Light 1 Color / Light 2 Color / Main Light Color**  
  These buttons open a color picker to select the RGB color of each light source. This allows dynamic adjustment of the visual environment and scene ambiance.

---

### Camera Settings

- **Camera Mode Dropdown**  
  Allows switching between different camera perspectives:
  - `Free`: The user can freely navigate the camera in the scene.
  - `Follow`: The camera automatically follows the robot from a third-person perspective.
  - `Scanner`: Switches from a close-up (from the robot's perspective) to a fixed camera view for detailed object scanning.

# Visual Design Documentation
### Software Architecture
![sa_diagram](https://github.com/user-attachments/assets/cc6d3099-8603-4ad9-bc90-6aacc1af4ade)

### Class Diagram
![class_diagram](https://github.com/user-attachments/assets/ab297831-23d5-4aeb-899e-c10314c32227)

### Method Descriptions
![Method Descriptions_1 pdf](https://github.com/user-attachments/assets/76848dd8-cf69-4f4a-bf37-208709d0776f)

# Functional Features

## 1. Interactive 3D Museum Room Environment
- A complete 3D museum room is implemented using OpenGL 3.3 Core Profile.
- The environment includes textured walls and floors, with proper projection and view matrix configurations.
- Depth testing and back-face culling are enabled for realistic 3D rendering.
- Camera controls support multiple viewing modes including free navigation, robot follow view, and scanner view.

## 2. Five Unique Artifacts Modeled in Blender
- Five distinct 3D models were created in Blender and exported as .obj files along with .mtl and .png texture files.
- The models are dynamically loaded using TinyObjLoader and rendered with full texture and normal support.
- Each artifact is placed in a fixed position within the scene using individual transformation matrices.

## 3. Realistic Lighting System
- A main directional light positioned above the scene provides uniform base illumination simulating sunlight or overhead lighting.
- Two point lights, located on the sides of the room, contribute to ambient lighting by emitting light in all directions within a limited radius.
- Each of the five artifacts is illuminated by a dedicated spotlight with adjustable parameters (direction, cutoff angle, and attenuation).
- All lighting computations are implemented in the fragment shader using the Phong lighting model (ambient, diffuse, specular).
- Spotlights can be dynamically toggled through the user interface and respond to robot proximity during scanning interactions.

## 4. Autonomous and Manual Robot Navigation
- A robot model created in Blender is integrated into the scene and can navigate through the environment.
- The robot supports both manual control using keyboard (WASD) and automatic movement along a predefined path.
- Movement and orientation are handled using transformation matrices.
- Collision detection prevents the robot from passing through walls or overlapping with artifacts.
- The robot's scale and speed are calibrated for proportional and realistic movement.

## 5. Artifact Scanning and Popup Info Display
- When the robot approaches an artifact within a certain range and angle, a scanning sequence is triggered.
- A red laser beam is rendered from the robot arm to the artifact using GL_LINES.
- After a successful scan, a popup window displays artifact information such as name, description, and origin.
- The scanning process is proximity-based and depends on both distance and orientation to the target.

## 6. Graphical User Interface with ImGui
- The application integrates ImGui to provide a responsive and user-friendly interface.
- The UI offers the following functionalities:
  - Toggle between Manual and Auto movement modes.
  - Enable or disable individual spotlights for each artifact.
  - Trigger scan operation manually if conditions are met.
  - Display or hide popup information windows.
  - Return the robot to its initial spawn location with one click.
- The user interface updates in real-time and is rendered within the OpenGL context.

## Technologies and Tools Used

### Programming and Graphics
- **Language:** C++
- **Graphics API:** OpenGL 3.3 Core Profile
- **Math Library:** GLM – for vector and matrix operations
- **Window & Input Handling:** GLFW  
  License: Zlib/libpng
- **OpenGL Function Loader:** GLAD  
  License: MIT
- **Shader Management and Rendering:** Custom implementations based on LearnOpenGL references

### User Interface
- **UI Framework:** ImGui – real-time interface for robot control and lighting  
  License: MIT

### 3D Modeling and Texturing
- **3D Modeling Tool:** Blender – used to create and export all 3D models in `.obj` format
- **Texture Editing:** Krita and GIMP – for preparing texture images and maps
- **Model Loader:** TinyObjLoader – for loading .obj files into OpenGL  
  License: MIT
- **Image Loader:** stb_image – header-only library for texture loading  
  License: Public Domain

### Diagram and Documentation
- **Diagram Design:** Canva – used to create architecture and UML diagrams
- **Documentation Format:** Markdown (README.md) with embedded visuals

### Development Environment
- **IDE:** Visual Studio 2022
- **Operating System:** Windows 10
- **Version Control:** Git (local) & GitHub (remote repository)

### External Code References
- Initialization, shader handling, and camera system partially adapted from:
  - [LearnOpenGL](https://learnopengl.com/)
  - [ImGui Examples](https://github.com/ocornut/imgui/tree/master/examples)
  - Open-source OpenGL boilerplate projects on GitHub (acknowledged in repository)

# Testing Summary

### Functional Testing

#### Robot Navigation and Scanning
- The robot navigates the 3D museum space without collision or logical path errors.
- Manual movement via WASD keys was tested across 10 different points; response time remained under ~50ms.
- In automatic mode, the robot followed the predefined path and performed scans at each artifact correctly in 9 out of 10 test runs (1 failure due to early user interruption).
- Scanning popups were triggered at a consistent average delay of **~140ms** after proximity and orientation checks passed.

#### 3D Model Integrity
- All 5 3D models were tested for successful load via TinyObjLoader; no missing geometry or faces encountered.
- Blender-exported models maintained vertex normals and face integrity.
- Texture mapping was visually verified; all models showed proper UV alignment without flickering or shifting during camera movement.

#### Lighting System
- Spotlights on each artifact were toggled 10 times each via the ImGui interface; no cross-trigger or delay observed.
- Directional light and spotlight intensity values remained stable over 30-minute test sessions.
- Visual consistency was verified during robot motion and scanning transitions.

---

### Usability Testing

#### User Interface
- ImGui-based interface elements (sliders, buttons, dropdowns) were tested for responsiveness. All inputs registered within 1 frame (approx. 16ms at 60 FPS).
- Three test users (with no prior experience) were able to:
  - Complete one full scanning cycle: within 10-15 seconds
  - The time it takes to complete a full tour (scanning all 5 objects) in auto mode: within 1 minute and 2 seconds

#### Scanning Workflow
- Manual scanning rejected 100% of invalid attempts (robot too far or not facing the object).
- Auto mode completed full scanning sequences in 10/10 tests, with popup display appearing immediately after scan confirmation.
- Popups were auto-dismissed after reset and did not persist beyond their intended interaction window.

---

### Performance

#### Frame Rate and Responsiveness
- Benchmarked on a system with **GTX 1650, Intel i5-10300H, 8GB RAM**.
- Observed average FPS: **59.8**, minimum: **57.3**, maximum: **61.1** (measured over a 3-minute walkthrough).
- No freezing or lag occurred during lighting adjustments or object rendering.

#### Load Times and Resource Usage
- Scene initialized (models + textures) in **~2.1 seconds**.
- GPU memory usage peaked at **~710MB**, system RAM at **~460MB**, verified using Task Manager and GPU-Z.
- Shader programs were compiled at startup (~30ms), reused efficiently throughout rendering.

---

# Team Contributions

**Yunus Emre Erten (2021556030):**  
- Designed and implemented the 3D museum room using OpenGL  
- Developed global and per-object spotlight systems  
- Assisted with integration and system testing  

**Veysel Genç (2021556031):**  
- Implemented robot navigation logic  
- Developed the scanning and popup system  
- Tested and debugged robot-artifact interactions  

**Zeliha İnan (2021556037):**  
- Modeled all five artifacts in Blender based on Adana Museum references  
- Exported and integrated models into OpenGL scene  
- Prepared project documentation, diagrams, and method references  

# Project Timeline

- Weeks 1–2: Museum room setup and lighting  
- Weeks 3–5: Blender modeling and OpenGL integration  
- Weeks 6–7: Robot and scanning system development  
- Weeks 8–9: Full system integration and internal testing  
- Weeks 10–11: Documentation and UI enhancements  
- Week 12: Final submission and video presentation

# Final Notes

This README serves as the complete technical and functional documentation for the Virtual Museum Project developed for the Computer Graphics course. All project components including modeling, rendering, robotic interaction, and UI design were developed in compliance with academic integrity standards and course requirements.
Any external use of this project or its assets must credit the authors.


