# Virtual Museum

# Demonstration Video

//VIDEO GELECEK

*Click the thumbnail above to watch the demonstration video.*

# Project Overview

The Virtual Museum is a computer graphics project developed using C++ and OpenGL 3.3 Core Profile. It aims to simulate a virtual museum environment where users can interact with five 3D-modeled museum artifacts through a mobile robot equipped with a scanning arm. The project features a 3D museum room, realistic lighting (including global and spotlights), robot navigation and interaction, and a user interface for controlling robot behavior and lighting.

The project was completed by the following students from Çukurova University, Computer Engineering Department:

- Yunus Emre Erten – 2021556030  
- Veysel Genç – 2021556031  
- Zeliha İnan – 2021556037

# Technologies Used

- Programming Language: C++
- Graphics Library: OpenGL 3.3 Core Profile
- 3D Modeling: Blender
- UI Framework: ImGui
- Math Library: GLM
- Context and Input: GLFW
- Shader Management: GLAD
- IDE: Visual Studio 2022
- Version Control: Git, GitHub

# Project Directory Structure

virtual-adana-museum-v2-team-7/
<pre lang="md"><code>```txt virtual-adana-museum-v2-team-7/ ├── assets/ │ ├── models/ → 3D object files │ ├── font/ ├── Libraries/ │ ├── include/ │ ├── lib/ │ ├── imgui/ ├── docs/ → Diagrams └── README.md → This documentation file ```</code></pre> 

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
- A global directional light provides base illumination for the room.
- Each artifact is illuminated by a separate spotlight with configurable properties (direction, cutoff angle, attenuation).
- Lighting is implemented in the fragment shader using the Phong lighting model.
- Spotlight activation is dynamic and can change based on robot interaction.

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

# Installation and Execution Instructions

1. Clone the repository:

git clone [ https://github.com/yuemco/virtual-adana-museum-v2-team-7.git](https://github.com/yuemco/virtual-adana-museum-v2-team-7.git)
cd virtual-adana-museum-v2-team-7

2. Open the `virtual-adana-museum-v2-team-7.sln` file using Visual Studio 2022.

3. Make sure the following libraries are linked properly in the project:
   - GLAD
   - GLFW
   - GLM
   - ImGui

4. Set the build configuration to `Debug x64` and run the project. 

# Controls

**Keyboard:**

- W / A / S / D: Move the robot manually
- E / Q: Rotate the robot manually
- Mouse Movement: Rotate the camera
- Shift + Mouse Movement: Panning 
- Scroll: Zoom in/out  

**ImGui User Interface:**

## Control Panel Overview

This interface provides a comprehensive control panel for interacting with a robot and its environment in a 3D simulation environment. The panel is divided into three main sections: Robot Controls, Light Settings, and Camera Settings.

---

### Robot Controls

- **Auto Mode Checkbox**  
  Enables or disables autonomous movement mode. When checked, the robot moves and performs actions without manual input.

- **Manual Movement Buttons**  
  - `Left`, `Right`, `Forward`, `Back`: Manually moves the robot in the respective direction.
  - `Rotate Left (Q)`, `Rotate Right (E)`: Rotates the robot on its vertical axis using keyboard shortcuts or button clicks.

- **Arm Angle Slider**  
  A horizontal slider that controls the vertical angle of the robot’s arm. The value ranges from 0.000 to a maximum defined in the implementation (e.g., 1.000). This simulates scanning or grabbing motions.

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
  - `Scanner`: Switches to a close-up or fixed camera view intended for detailed object scanning.


# Software Architecture and Diagrams

**Class Diagram:**  
![class_diagram](https://github.com/user-attachments/assets/ab297831-23d5-4aeb-899e-c10314c32227)

**Software Architecture Diagram:**  
![sa_diagram](https://github.com/user-attachments/assets/cc6d3099-8603-4ad9-bc90-6aacc1af4ade)

**Method Descriptions:**  
![Method Descriptions_1 pdf](https://github.com/user-attachments/assets/76848dd8-cf69-4f4a-bf37-208709d0776f)


# Testing Summary

**Functional Testing:**  
- Robot navigates without error and accurately triggers popups  
- Each model loads with correct geometry and texture  
- Lighting toggles work on per-object basis  

**Usability Testing:**  
- Interface is intuitive for controlling robot and lighting  
- Automatic and manual scanning flows are user-friendly  

**Performance:**  
- Stable FPS maintained on mid-level systems  
- Optimized shaders and model sizes for real-time rendering  

# Licensing and Compliance

- All 3D models and texture images were created from scratch by the project team.  
- No external models or commercial textures were used.  
- Open-source libraries included:  
  - GLAD (MIT License)  
  - GLFW (Zlib License)  
  - ImGui (MIT License)

# Team Contributions

**Yunus Emre Erten:**  
- Designed and implemented the 3D museum room using OpenGL  
- Developed global and per-object spotlight systems  
- Assisted with integration and system testing  

**Veysel Genç:**  
- Implemented robot navigation logic  
- Developed the scanning and popup system  
- Tested and debugged robot-artifact interactions  

**Zeliha İnan:**  
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


