#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "shaderClass.h"
#include "model.h"
#include "camera.h"

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

glm::mat4 projection; // Global projection matrix

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
uniform sampler2D texture_diffuse1;
uniform vec3 objectColor;

void main()
{
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse);
    vec4 texColor = texture(texture_diffuse1, TexCoord);
    FragColor = vec4(result, 1.0) * texColor;
}
)";

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

    Shader shader(vertexShaderSource, fragmentShaderSource);

    Model model1("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model1.obj");
    Model model2("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model2.obj");
    Model model3("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model3.obj");
    Model model4("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model4.obj");
    Model model5("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model5.obj");

    // Ýlk projection hesaplamasý
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    projection = glm::perspective(glm::radians(camera.Zoom), (float)w / (float)h, 0.1f, 100.0f);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::vec3 lightPos = glm::vec3(0.0f, 3.0f, 3.0f);
        glm::vec3 viewPos = camera.Position;
        glm::vec3 lightColor = glm::vec3(1.0f);

        glUniform3fv(glGetUniformLocation(shader.ID, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shader.ID, "viewPos"), 1, glm::value_ptr(viewPos));
        glUniform3fv(glGetUniformLocation(shader.ID, "lightColor"), 1, glm::value_ptr(lightColor));

        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        // Zemin
        glm::mat4 groundMat = glm::mat4(1.0f);
        shader.setMat4("model", groundMat);
        glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 0.2f, 0.2f, 0.35f);

        float groundVertices[] = {
            -10.0f, 0.0f, -5.0f,
             10.0f, 0.0f, -5.0f,
             10.0f, 0.0f,  5.0f,
            -10.0f, 0.0f,  5.0f
        };
        unsigned int groundIndices[] = { 0, 1, 2, 0, 2, 3 };
        unsigned int VAO, VBO, EBO;
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
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        std::vector<std::pair<Model*, glm::vec3>> models = {
            { &model1, glm::vec3(-6.0f, 1.1f, 0.0f) },
            { &model2, glm::vec3(-3.0f, 1.0f, 0.0f) },
            { &model3, glm::vec3(0.0f, 0.52f, 0.0f) },
            { &model4, glm::vec3(3.0f, 1.1f, 0.0f) },
            { &model5, glm::vec3(6.0f, 0.4f, 0.0f) }
        };

        for (auto& [model, pos] : models) {
            glm::mat4 modelMat = glm::mat4(1.0f);
            modelMat = glm::translate(modelMat, pos);
            modelMat = glm::scale(modelMat, glm::vec3(1.0f));
            shader.setMat4("model", modelMat);
            glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 1.0f, 1.0f, 1.0f);
            model->Draw(shader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
