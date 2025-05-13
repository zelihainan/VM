#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "shaderClass.h"
#include "model.h"
#include "camera.h"
#include "robot.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

const unsigned int INIT_WIDTH = 800;
const unsigned int INIT_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 1.5f, 10.0f));
float lastX = INIT_WIDTH / 2.0f;
float lastY = INIT_HEIGHT / 2.0f;
bool firstMouse = true;
bool leftMousePressed = false;
bool isPanning = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
glm::mat4 projection;

// === Shader kaynaklarý ===
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D texture_diffuse1;
uniform bool useTexture;

void main()
{
    float ambientStrength = 0.7;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 lighting = (ambient + diffuse);

    vec4 baseColor;
    if (useTexture)
        baseColor = texture(texture_diffuse1, TexCoord);
    else
        baseColor = vec4(objectColor, 1.0);

    FragColor = vec4(lighting, 1.0) * baseColor;
}
)";


// === Callback fonksiyonlarý ===
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
            leftMousePressed = true;
        else if (action == GLFW_RELEASE)
            leftMousePressed = false;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

    if (ImGui::GetIO().WantCaptureMouse)
        return;

    if (!leftMousePressed) {
        firstMouse = true;
        return;
    }

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        isPanning = true;
        camera.Position -= camera.Right * xoffset * 0.01f;
        camera.Position += camera.Up * yoffset * 0.01f;
    }
    else
    {
        isPanning = false;
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
}

void processInput(GLFWwindow* window, Robot& robot, float deltaTime, const std::vector<glm::vec3>& obstacles)
{
    glm::vec3 nextPos = robot.position;
    float speed = deltaTime * 3.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        nextPos.z -= speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        nextPos.z += speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        nextPos.x -= speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        nextPos.x += speed;

    // Robot sýnýrlarý hesaba katýlarak dýþa çýkma engeli
    float robotRadius = 1.2f;
    if (nextPos.x < -10.0f + robotRadius || nextPos.x > 10.0f - robotRadius ||
        nextPos.z < -5.0f + robotRadius || nextPos.z > 5.0f - robotRadius)
        return;

    // Obje çarpýþma kontrolü
    for (const auto& obj : obstacles)
    {
        float dynamicThreshold = 1.5f;
        if (obj.x < -5.0f || obj.x > 5.0f)
            dynamicThreshold = 2.0f;

        if (glm::distance(nextPos, obj) < dynamicThreshold)
            return;
    }

    robot.position = nextPos;
}



int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(INIT_WIDTH, INIT_HEIGHT, "Virtual Museum", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Shader shader(vertexShaderSource, fragmentShaderSource);

    Model model1("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model1.obj");
    Model model2("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model2.obj");
    Model model3("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model3.obj");
    Model model4("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model4.obj");
    Model model5("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model5.obj");

    Robot robot("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/robot.obj", glm::vec3(0.0f, 0.0f, -3.0f));

    std::vector<glm::vec3> objectPositions = {
    glm::vec3(-6.0f, 0.0f, 0.0f),
    glm::vec3(-3.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(3.0f, 0.0f, 0.0f),
    glm::vec3(6.0f, 0.0f, 0.0f)
    };
    static int currentTarget = 0;
    static bool autoMode = false;

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    projection = glm::perspective(glm::radians(camera.Zoom), (float)w / (float)h, 0.1f, 100.0f);

    float groundVertices[] = {
        -10.0f, 0.0f, -5.0f,  10.0f, 0.0f, -5.0f,  10.0f, 0.0f, 5.0f,  -10.0f, 0.0f, 5.0f
    };
    unsigned int groundIndices[] = { 0, 1, 2, 0, 2, 3 };

    float wallVertices[] = {
        -10,0,-5, -10,5,-5, -10,5,5, -10,0,5,
        10,0,-5, 10,5,-5, 10,5,5, 10,0,5,
        -10,0,-5, 10,0,-5, 10,5,-5, -10,5,-5,
        -10,0,5, 10,0,5, 10,5,5, -10,5,5,
        -10,5,-5, 10,5,-5, 10,5,5, -10,5,5
    };
    unsigned int wallIndices[] = {
        0,1,2, 0,2,3,
        4,5,6, 4,6,7,
        8,9,10, 8,10,11
    };

    unsigned int VAO, VBO, EBO, wallVAO, wallVBO, wallEBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);
    glGenBuffers(1, &wallEBO);
    glBindVertexArray(wallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), wallVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(wallIndices), wallIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.7f, 0.7f, 0.75f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Robot Kontrol Paneli");
        static bool autoMode = true;
        ImGui::Checkbox("Auto Mode", &autoMode);
        if (!autoMode) {
            if (ImGui::Button("Left"))  robot.position.x -= deltaTime * 5.0f;
            if (ImGui::Button("Right")) robot.position.x += deltaTime * 5.0f;
            if (ImGui::Button("Forward")) robot.position.z -= deltaTime * 5.0f;
            if (ImGui::Button("Back")) robot.position.z += deltaTime * 5.0f;
        }
        ImGui::End();

        if (!autoMode)
            processInput(window, robot, deltaTime, objectPositions);
        else {
            if (glm::distance(robot.position, objectPositions[currentTarget]) < 0.2f) {
                currentTarget = (currentTarget + 1) % objectPositions.size();
            }
            robot.moveTo(objectPositions[currentTarget], deltaTime * 2.0f);
        }

        shader.use();

        glm::vec3 lightPos = glm::vec3(0.0f, 4.9f, 0.0f);
        glUniform3fv(glGetUniformLocation(shader.ID, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shader.ID, "viewPos"), 1, glm::value_ptr(camera.Position));
        glUniform3fv(glGetUniformLocation(shader.ID, "lightColor"), 1, glm::value_ptr(glm::vec3(1.0f)));

        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        std::vector<std::pair<Model*, glm::vec3>> models = {
            { &model1, glm::vec3(-6.0f, 1.1f, 0.0f) },
            { &model2, glm::vec3(-3.0f, 1.0f, 0.0f) },
            { &model3, glm::vec3(0.0f, 0.52f, 0.0f) },
            { &model4, glm::vec3(3.0f, 1.1f, 0.0f) },
            { &model5, glm::vec3(6.0f, 0.4f, 0.0f) }
        };

        shader.setMat4("model", glm::mat4(1.0f));
        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), false);
        glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 0.6f, 0.6f, 0.6f);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        shader.setMat4("model", glm::mat4(1.0f));
        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), false);
        glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 0.95f, 0.9f, 0.85f);
        glBindVertexArray(wallVAO);
        glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);

        for (auto& [model, pos] : models) {
            glm::mat4 modelMat = glm::mat4(1.0f);
            modelMat = glm::translate(modelMat, pos);
            modelMat = glm::scale(modelMat, glm::vec3(1.0f));
            shader.setMat4("model", modelMat);
            glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), true);
            glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 1.0f, 1.0f, 1.0f);
            model->Draw(shader);
        }

        robot.draw(shader);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}