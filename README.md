
# Virtual Museum
---

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

virtual-museum/  
├── models/               → Blender-exported 3D object files  
├── textures/             → Custom texture images  
├── shaders/              → Vertex and fragment shaders  
├── src/                  → C++ source code  
├── includes/             → Header files  
├── docs/                 → Diagrams and documentation  
└── README.md             → This documentation file

# Functional Features

- A complete 3D museum room built in OpenGL
- Five unique museum artifacts modeled in Blender
- Realistic lighting setup with a global light and five object-specific spotlights
- A mobile robot capable of navigating the room
- Scanning system that displays popup information for each artifact
- User interface allowing:
  - Manual or automatic robot control
  - Lighting toggle for each spotlight
  - Object scanning and information display
  - Return-to-start functionality

# Installation and Execution Instructions

1. Clone the repository:

git clone https://github.com/yourusername/virtual-museum.git
cd virtual-museum

2. Open the `virtual-museum.sln` file using Visual Studio 2022.

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
- Scroll: Zoom in/out  

**ImGui User Interface:**

- Auto Scan: Robot visits and scans all objects sequentially  
- Manual Scan: User moves the robot and scans manually  
- Light Toggles: Turn on/off spotlights for each object  
- Return Home: Return robot to initial position  


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


