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
#include <Windows.h>         
#include <filesystem>        
#include <string>            

std::string getExecutableDir() {
    char buffer[MAX_PATH];    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::filesystem::path exePath(buffer);
    return exePath.parent_path().string();
}

const unsigned int INIT_WIDTH = 800;
const unsigned int INIT_HEIGHT = 600;

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};


Camera camera(glm::vec3(0.0f, 2.0f, 15.0f));
float lastX = INIT_WIDTH / 2.0f;
float lastY = INIT_HEIGHT / 2.0f;
bool firstMouse = true;
bool leftMousePressed = false;
bool isPanning = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
glm::mat4 projection;

bool autoMode = false; 

float armAngle = 0.0f; 
int scannedModelIndex = -1;


//Shader kaynakları
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

uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D texture_diffuse1;
uniform bool useTexture;

uniform vec3 spotLights[5];
uniform vec3 spotDirs[5];
uniform float intensities[5];

uniform vec3 pointLights[2];
uniform float pointIntensities[2];
uniform vec3 pointColors[2];

uniform vec3 ceilingPos;
uniform vec3 ceilingColor;
uniform float ceilingIntensity;


const float cutOff = cos(radians(20.0)); // Spotlight açısı

void main()
{
    vec3 norm = normalize(Normal);
    vec3 result = vec3(0.0);


    for (int i = 0; i < 5; ++i)
    {
        vec3 lightDir = normalize(spotLights[i] - FragPos);
        float theta = dot(lightDir, normalize(-spotDirs[i]));

        if (theta > cutOff) {
            float diff = max(dot(norm, lightDir), 0.0);
            result += diff * intensities[i] * lightColor;
        }

    vec3 ceilingDir = normalize(ceilingPos - FragPos);
    float ceilingDiff = max(dot(norm, ceilingDir), 0.0);
    vec3 ceilingLight = ceilingDiff * ceilingIntensity * ceilingColor;
    result += ceilingLight;

    }

    for (int i = 0; i < 2; ++i)
    {
        vec3 lightDir = normalize(pointLights[i] - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        result += diff * pointIntensities[i] * pointColors[i];
    }

    result += 0.15 * lightColor;

    vec4 baseColor = useTexture
        ? texture(texture_diffuse1, TexCoord)
        : vec4(objectColor, 1.0);

    FragColor = vec4(result, 1.0) * baseColor;
}
)";


//Callback fonksiyonları
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

void moveIfValid(Robot& robot, glm::vec3 newPos, const std::vector<glm::vec3>& obstacles)
{
    float robotRadius = 0.6f;

    if (newPos.x < -10.0f + robotRadius || newPos.x > 10.0f - robotRadius ||
        newPos.z < -5.0f + robotRadius || newPos.z > 5.0f - robotRadius)
        return;

    for (const auto& obj : obstacles)
    {
        float collisionRadius = 1.2f;
        if (glm::distance(newPos, obj) < collisionRadius)
            return;
    }

    robot.position = newPos;
}

void processInput(GLFWwindow* window, Robot& robot, float deltaTime, const std::vector<glm::vec3>& obstacles)
{
    float speed = deltaTime * 5.0f;
    glm::vec3 nextPos = robot.position;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        nextPos.z -= speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        nextPos.z += speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        nextPos.x -= speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        nextPos.x += speed;

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        robot.rotationY += deltaTime * 100.0f;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        robot.rotationY -= deltaTime * 100.0f;

    moveIfValid(robot, nextPos, obstacles);
}

bool RayIntersectsAABB(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 boxMin, glm::vec3 boxMax) {
    float tMin = (boxMin.x - rayOrigin.x) / rayDir.x;
    float tMax = (boxMax.x - rayOrigin.x) / rayDir.x;
    if (tMin > tMax) std::swap(tMin, tMax);

    float tyMin = (boxMin.y - rayOrigin.y) / rayDir.y;
    float tyMax = (boxMax.y - rayOrigin.y) / rayDir.y;
    if (tyMin > tyMax) std::swap(tyMin, tyMax);

    if ((tMin > tyMax) || (tyMin > tMax))
        return false;

    if (tyMin > tMin) tMin = tyMin;
    if (tyMax < tMax) tMax = tyMax;

    float tzMin = (boxMin.z - rayOrigin.z) / rayDir.z;
    float tzMax = (boxMax.z - rayOrigin.z) / rayDir.z;
    if (tzMin > tzMax) std::swap(tzMin, tzMax);

    if ((tMin > tzMax) || (tzMin > tMax))
        return false;

    return true;
}


std::vector<std::string> modelInfoTexts = {
    u8"Erkek Heykeli  Roma Dönemi, M.S. 2. yüzyıl Bu tunç heykel, Roma İmparatorluğu'nun yüksek sanat anlayışını temsil eden, Toga giymiş bir erkek yurttaşı betimler. M.S. 2. yüzyıla tarihlenen bu eser, özellikle portre detaylarındaki gerçekçilikle dikkat çeker. Togası, dönemin sosyal statüsünü simgelerken, sağ eliyle tuttuğu belge ya da rulo figürü onun entelektüel ya da siyasal bir kimliği olabileceğini düşündürür. Bu tip heykeller, Roma kentlerinin kamu alanlarında imparatorluk değerlerini ve vatandaşlık bilincini yansıtmak için sergilenmiştir.",
    u8"Akhilleus Lahdi  M.S. 2. yüzyıl, Roma Dönemi Akhilleus Lahdi, Troya Savaşı’nın kahramanı Akhilleus’un yaşamından sahnelerle süslenmiş yüksek kabartmalı bir mezar anıtıdır.M.S. 2. yüzyılda yapılmış olan bu lahit, ölen kişinin kahramanlıkla özdeşleştirilmek istendiğini gösterir.Lahdin bir yüzünde Akhilleus’un Briseis’i Agamemnon’a teslim edişi, diğer yüzlerde ise cenaze töreni ve savaş sahneleri yer alır.Mitolojik betimlemeleriyle yalnızca sanatsal değil, aynı zamanda simgesel anlatımıyla da dikkat çeker."
    u8"Arabalı Tanrı Tarhunda (Fırtına Tanrısı)  Geç Hitit Dönemi, M.Ö. 9. yüzyıl Tarhunda, Anadolu'nun Geç Hitit dönemine ait en önemli tanrılarından biridir. Bu anıtsal taş heykel, Tarhunda’yı bir savaş arabası üzerinde iki boğa tarafından çekilirken tasvir eder. Tanrı’nın sağ elindeki balta ve sol elindeki yıldırım demeti, onun gök gürültüsü, savaş ve bereketle ilişkilendirilen doğasını simgeler. M.Ö. 9. yüzyıla tarihlenen bu eser, özellikle Arslantaş (eski Kummuh Krallığı) bölgesinden çıkarılmıştır ve Geç Hitit sanatının görkemli bir örneğidir.",
    u8"Filozoflar Lahdi  M.S. 3. yüzyıl, Roma Dönemi Bu mermer lahit, stoacı düşünce ve felsefi yaşamı simgeleyen figürlerle bezelidir.Lahdin uzun yüzeylerinde toga giymiş filozoflar ellerinde kitap, papirüs veya düşünür pozlarında gösterilmiştir.Bu tasvirler, ölen kişinin entelektüel bir yaşamı benimsediğini ve ölümden sonra da bilgelik içinde anılmak istendiğini ifade eder.Yazıtlarında Yunanca kitabeler yer alır.Bu tip lahitler, özellikle Roma İmparatorluğu’nun doğu eyaletlerinde seçkin sınıflar arasında yaygındı.",
    u8"Kadın Portre Steli  Roma Dönemi, M.S. 2. yüzyıl Bu küçük boyutlu mezar steli, muhtemelen yerel bir Roma vatandaşına ait olup M.S. 2. yüzyıla tarihlenmektedir.Üzerinde kadın figürü yarım kabartma şeklinde yer alır; başı örtülü, göğsünde fibula(giysi tokası) olan sade ama anlam yüklü bir betimleme sunar.Üstteki üçgen alınlık kısmında sembolik bezemeler bulunur.Bu tür steller, Roma dönemi Anadolusunda kadınların sosyal kimliğini, ailevi bağlarını ve inanç sistemini yansıtan önemli belgelerdir.",

};

enum CameraMode { Free, Follow, Scanner };
CameraMode camMode = Free;
CameraMode prevCamMode = Free;


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
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    projection = glm::perspective(glm::radians(camera.Zoom), (float)w / (float)h, 0.1f, 100.0f);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

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

    glm::vec3 smoothCameraPos = glm::vec3(0.0f, 1.5f, 10.0f);

    PointLight ceilingLight = {
        glm::vec3(0.0f, 5.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        0.1f
    };

    static const ImWchar turkish_range[] = {
        0x0020, 0x00FF, 
        0x011E, 0x011F, 
        0x0130, 0x0131, 
        0x015E, 0x015F, 
        0
    };

    ImFontConfig font_cfg;
    font_cfg.OversampleH = 3;
    font_cfg.OversampleV = 1;
    font_cfg.PixelSnapH = true;

    std::string fontPath = getExecutableDir() + std::string("/../../assets/font/OpenSans-Regular.ttf");
    io.FontDefault = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f, &font_cfg, turkish_range);


    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 3.0f;
    style.ScrollbarRounding = 6.0f;

    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(10, 8);
    style.ItemInnerSpacing = ImVec2(8, 4);

    style.FrameBorderSize = 1.0f;
    style.WindowBorderSize = 1.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.105f, 0.12f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.205f, 0.25f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.305f, 0.35f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.35f, 0.4f, 1.0f);

    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.205f, 0.25f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.305f, 0.35f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.1505f, 0.2f, 1.0f);

    colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.105f, 0.12f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.205f, 0.25f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.3f, 1.0f);

    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.2f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.38f, 0.38f, 0.5f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.28f, 0.4f, 1.0f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.2f, 1.0f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);

    colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.105f, 0.12f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.1f, 0.105f, 0.12f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1f, 0.105f, 0.12f, 1.0f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    static bool showInfoPopup = false;
    static float popupTimer = 0.0f;
    static int lastScannedIndex = -1;

    std::vector<glm::vec3> spotlightPositions = {
        glm::vec3(-6.0f, 4.0f, 0.0f),
        glm::vec3(-3.0f, 4.0f, 0.0f),
        glm::vec3(0.0f, 4.0f, 0.0f),
        glm::vec3(3.0f, 4.0f, 0.0f),
        glm::vec3(6.0f, 4.0f, 0.0f)
    };


    std::vector<glm::vec3> pointLights = {
    glm::vec3(-9.0f, 1.0f, 4.0f),   
    glm::vec3(9.0f, 1.0f, 4.0f)    
    };

    std::vector<glm::vec3> pointColors = {
    glm::vec3(1.0f, 1.0f, 1.0f),  
    glm::vec3(1.0f, 1.0f, 1.0f)   
    };

    std::vector<float> pointIntensities = { 0.7f, 0.7f }; 

    glm::vec3 spotlightDirection = glm::vec3(0.0f, -1.0f, 0.0f);

    Shader shader(vertexShaderSource, fragmentShaderSource);

    std::string baseDir = getExecutableDir();
    std::string modelDir = baseDir + "/../../assets/models/";

    Model model1(modelDir + "model1.obj");
    Model model2(modelDir + "model2.obj");
    Model model3(modelDir + "model3.obj");
    Model model4(modelDir + "model4.obj");
    Model model5(modelDir + "model5.obj");

    Robot robot(
        modelDir + "robot_body.obj",
        modelDir + "robot_arm.obj",
        glm::vec3(-5.0f, 0.0f, 2.5f)
    );
    

    std::vector<glm::vec3> objectPositions = {
        glm::vec3(-6.0f, 0.0f, 0.0f),
        glm::vec3(-3.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(3.0f, 0.0f, 0.0f),
        glm::vec3(6.0f, 0.0f, 0.0f)
    };

    static int currentTarget = 0;

    std::vector<glm::vec3> fullPath = {
        glm::vec3(-5.0f, 0.0f, 2.5f),  
        glm::vec3(-6.0f, 0.0f, 1.2f),  
        glm::vec3(-3.0f, 0.0f, 1.2f),  
        glm::vec3(0.0f, 0.0f, 1.2f),   
        glm::vec3(3.0f, 0.0f, 1.2f),  
        glm::vec3(6.0f, 0.0f, 1.2f),   
        glm::vec3(-5.0f, 0.0f, 2.5f)   
    };

    static int pathIndex = 0;
    static bool isWaiting = false;
    static float waitTimer = 0.0f;


    glfwGetFramebufferSize(window, &w, &h);
    projection = glm::perspective(glm::radians(camera.Zoom), (float)w / (float)h, 0.1f, 100.0f);

    float groundVertices[] = {
        // pozisyon            // normal vektörleri 
        -10.0f, 0.0f, -5.0f,      0.0f, 1.0f, 0.0f,
         10.0f, 0.0f, -5.0f,      0.0f, 1.0f, 0.0f,
         10.0f, 0.0f,  5.0f,      0.0f, 1.0f, 0.0f,
        -10.0f, 0.0f,  5.0f,      0.0f, 1.0f, 0.0f
    };


    unsigned int groundIndices[] = { 0, 1, 2, 0, 2, 3 };

    float wallVertices[] = {
        -10,0,-5, -1.0f, 0.0f, 0.0f,
        -10,5,-5, -1.0f, 0.0f, 0.0f,
        -10,5, 5, -1.0f, 0.0f, 0.0f,
        -10,0, 5, -1.0f, 0.0f, 0.0f,


         10,0,-5, 1.0f, 0.0f, 0.0f,
         10,5,-5, 1.0f, 0.0f, 0.0f,
         10,5, 5, 1.0f, 0.0f, 0.0f,
         10,0, 5, 1.0f, 0.0f, 0.0f,


         -10,0,-5, 0.0f, 0.0f,-1.0f,
          10,0,-5, 0.0f, 0.0f,-1.0f,
          10,5,-5, 0.0f, 0.0f,-1.0f,
         -10,5,-5, 0.0f, 0.0f,-1.0f,


         -10,0, 5, 0.0f, 0.0f, 1.0f,
          10,0, 5, 0.0f, 0.0f, 1.0f,
          10,5, 5, 0.0f, 0.0f, 1.0f,
         -10,5, 5, 0.0f, 0.0f, 1.0f,


         -10,5,-5, 0.0f, 1.0f, 0.0f,
          10,5,-5, 0.0f, 1.0f, 0.0f,
          10,5, 5, 0.0f, 1.0f, 0.0f,
         -10,5, 5, 0.0f, 1.0f, 0.0f
    };


    unsigned int wallIndices[] = {
        0,1,2, 0,2,3,
        4,5,6, 4,6,7,
        8,9,10, 8,10,11
    };

    unsigned int rayVAO, rayVBO;
    glGenVertexArrays(1, &rayVAO);
    glGenBuffers(1, &rayVBO);


    unsigned int VAO, VBO, EBO, wallVAO, wallVBO, wallEBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    ;

    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);
    glGenBuffers(1, &wallEBO);

    glBindVertexArray(wallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), wallVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(wallIndices), wallIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


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


        static bool initialized = false;
        if (!initialized) {
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 260, 10), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(250, 400), ImGuiCond_Once);
            initialized = true;
        }

        //Kontrol Paneli
        ImGui::Begin("Control Panel");
        if (ImGui::CollapsingHeader("Robot Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Auto Mode", &autoMode);

            if (!autoMode) {
                glm::vec3 next = robot.position;
                float speed = deltaTime * 100.0f;
                const float rotationSpeed = 10.0f;

                ImGui::Text("Manual Movement");

                if (ImGui::Button("Left")) { next.x -= speed; moveIfValid(robot, next, objectPositions); }
                ImGui::SameLine();
                if (ImGui::Button("Right")) { next.x += speed; moveIfValid(robot, next, objectPositions); }
                ImGui::SameLine();
                if (ImGui::Button("Forward")) { next.z -= speed; moveIfValid(robot, next, objectPositions); }
                ImGui::SameLine();
                if (ImGui::Button("Back")) { next.z += speed; moveIfValid(robot, next, objectPositions); }

                if (ImGui::Button("Rotate Left (Q)")) { robot.rotationY += rotationSpeed; }
                ImGui::SameLine();
                if (ImGui::Button("Rotate Right (E)")) { robot.rotationY -= rotationSpeed; }

                ImGui::SliderFloat("Arm Angle", &armAngle, 0.0f, 90.0f);
            }
        }

        if (ImGui::CollapsingHeader("Light Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Columns(2, nullptr, false);

            ImGui::Text("Light 1 Intensity"); ImGui::NextColumn();
            ImGui::SliderFloat("##L1Int", &pointIntensities[0], 0.0f, 3.0f); ImGui::NextColumn();

            ImGui::Text("Light 2 Intensity"); ImGui::NextColumn();
            ImGui::SliderFloat("##L2Int", &pointIntensities[1], 0.0f, 3.0f); ImGui::NextColumn();

            ImGui::Text("Main Light Intensity"); ImGui::NextColumn();
            ImGui::SliderFloat("##CeilingInt", &ceilingLight.intensity, 0.0f, 0.5f); ImGui::NextColumn();

            ImGui::Text("Light 1 Color"); ImGui::NextColumn();
            ImGui::ColorEdit3("##L1Col", glm::value_ptr(pointColors[0]), ImGuiColorEditFlags_NoInputs); ImGui::NextColumn();

            ImGui::Text("Light 2 Color"); ImGui::NextColumn();
            ImGui::ColorEdit3("##L2Col", glm::value_ptr(pointColors[1]), ImGuiColorEditFlags_NoInputs); ImGui::NextColumn();

            ImGui::Text("Main Light Color"); ImGui::NextColumn();
            ImGui::ColorEdit3("##CeilingCol", glm::value_ptr(ceilingLight.color), ImGuiColorEditFlags_NoInputs); ImGui::NextColumn();

            ImGui::Columns(1);
        }

        if (ImGui::CollapsingHeader("Camera Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Combo("Camera Mode", (int*)&camMode, "Free\0Follow\0Scanner\0");
        }

        ImGui::End();

        if (camMode != prevCamMode) {
            if (camMode == Follow) {
                camera.SetBehindRobot(robot.position, robot.rotationY, deltaTime);

            }


            else if (camMode == Free) {
                camera.Position = glm::vec3(0.0f, 2.0f, 15.0f);
                camera.Front = glm::vec3(0.0f, 0.0f, -1.0f);
            }

            prevCamMode = camMode;
        }


        bool isScanningNow =
            (!autoMode && armAngle >= 60.0f) ||
            (autoMode && waitTimer >= 1.5f && waitTimer < 8.5f);

        if (scannedModelIndex != -1 &&
            scannedModelIndex < modelInfoTexts.size() &&
            isScanningNow)
        {
            ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Always);
            ImGui::Begin("Model Info", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            ImGui::TextWrapped("%s", modelInfoTexts[scannedModelIndex].c_str());
            ImGui::End();
        }


        if (!autoMode) {
            processInput(window, robot, deltaTime, objectPositions);

            //TARAMA KONTROLÜ
            scannedModelIndex = -1;

            if (armAngle >= 60.0f && popupTimer < 3.0f) {
                glm::vec3 rayStart = robot.position + glm::vec3(0.2f, 0.5f, 0.0f);
                glm::vec3 rayDir = glm::normalize(
                    glm::rotate(glm::mat4(1.0f), glm::radians(robot.rotationY), glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 1, 0)
                );

                float minDist = 100.0f;
                scannedModelIndex = -1;

                for (int i = 0; i < objectPositions.size(); ++i) {
                    glm::vec3 toObj = objectPositions[i] - rayStart;
                    float projLength = glm::dot(toObj, rayDir);

                    if (projLength > 0.0f) {
                        glm::vec3 closestPoint = rayStart + rayDir * projLength;
                        float distanceToObj = glm::distance(closestPoint, objectPositions[i]);

                        if (distanceToObj < 1.0f && projLength < minDist) {
                            minDist = projLength;
                            scannedModelIndex = i;
                        }
                    }
                }
            }

        }
        else {
            //AUTO MODE
            glm::vec3 target = fullPath[pathIndex];
            float dist = glm::distance(robot.position, target);

            if (!isWaiting) {

                if (dist < 0.2f) {
                    if (pathIndex > 0 && pathIndex <= 5) {
                        isWaiting = true;
                        waitTimer = 0.0f;

                        scannedModelIndex = pathIndex - 1;
                        armAngle = 60.0f;

                        glm::vec3 toModel = objectPositions[scannedModelIndex] - robot.position;
                        robot.rotationY = glm::degrees(atan2(toModel.x, toModel.z));
                    }
                    else {
                        isWaiting = false;
                        scannedModelIndex = -1;
                        armAngle = 0.0f;
                        popupTimer = 0.0f;
                        pathIndex++;
                        if (pathIndex >= fullPath.size()) pathIndex = 0;
                    }
                }

                else {
                    glm::vec3 direction = glm::normalize(target - robot.position);
                    glm::vec3 nextPos = robot.position + direction * deltaTime * 2.0f;
                    moveIfValid(robot, nextPos, objectPositions);
                }
            }


            else {
                waitTimer += deltaTime;
                popupTimer += deltaTime;

                if (waitTimer < 1.5f) {
                    armAngle = 60.0f * (waitTimer / 1.5f);

                }
                else if (waitTimer >= 1.5f && waitTimer < 8.5f) {
                    float t = waitTimer - 1.5f;  // 0 → 7.0
                    armAngle = 75.0f + sin(t * 2.0f) * 15.0f;
                    if (armAngle > 90.0f) armAngle = 90.0f;
                    if (armAngle < 60.0f) armAngle = 60.0f;

                    if (popupTimer == 0.0f) popupTimer = 0.001f;
                    scannedModelIndex = pathIndex - 1;
                }
                else if (waitTimer >= 8.5f && waitTimer < 10.0f) {
                    float t = (waitTimer - 8.5f) / 1.5f;
                    armAngle = 90.0f * (1.0f - t);
                    scannedModelIndex = -1;
                }
                if (waitTimer >= 10.0f) {
                    isWaiting = false;
                    scannedModelIndex = -1;
                    popupTimer = 0.0f;
                    pathIndex++;
                    if (pathIndex >= fullPath.size()) pathIndex = 0;
                    armAngle = 0.0f;
                }

            }
        }

        shader.use();

        shader.setVec3("ceilingPos", ceilingLight.position);
        shader.setVec3("ceilingColor", ceilingLight.color);
        shader.setFloat("ceilingIntensity", ceilingLight.intensity);

        static float smoothArmAngle = 0.0f;
        float dampingSpeed = 8.0f;
        smoothArmAngle = glm::mix(smoothArmAngle, armAngle, deltaTime * dampingSpeed);
        float armHeightOffset = sin(glm::radians(smoothArmAngle)) * 0.3f;

        glm::vec4 offset = glm::rotate(
            glm::mat4(1.0f),
            glm::radians(robot.rotationY),
            glm::vec3(0, 1, 0)
        ) * glm::vec4(-0.20f, 0.20f + armHeightOffset, 0.25f, 0.0f);

        glm::vec3 rayStart = robot.position + glm::vec3(offset);


        glm::vec3 rayDir = glm::normalize(
            glm::rotate(glm::mat4(1.0f), glm::radians(robot.rotationY), glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 1, 0)
        );

        glm::vec3 rayEnd = rayStart + rayDir * 5.5f;

        float rayVertices[] = {
            rayStart.x, rayStart.y, rayStart.z,
            rayEnd.x, rayEnd.y, rayEnd.z
        };

        glBindVertexArray(rayVAO);
        glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(rayVertices), rayVertices, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        //Spotlight sistemi
        std::vector<float> intensities(5, 0.2f);

        bool lightActive =
            (!autoMode && armAngle >= 60.0f && scannedModelIndex != -1) ||
            (autoMode && waitTimer >= 1.5f && waitTimer < 8.5f);

        for (int i = 0; i < objectPositions.size(); ++i) {
            if (i == scannedModelIndex && lightActive) {
                if (i == 1 || i == 4)  
                    intensities[i] = 0.5f; 
                else
                    intensities[i] = 2.5f;
            }
            else {
                intensities[i] = 0.2f;
            }
        }

        for (int i = 0; i < 5; ++i) {
            std::string posName = "spotLights[" + std::to_string(i) + "]";
            std::string dirName = "spotDirs[" + std::to_string(i) + "]";
            std::string intensityName = "intensities[" + std::to_string(i) + "]";

            glUniform3fv(glGetUniformLocation(shader.ID, posName.c_str()), 1, glm::value_ptr(spotlightPositions[i]));
            glUniform3fv(glGetUniformLocation(shader.ID, dirName.c_str()), 1, glm::value_ptr(spotlightDirection));
            glUniform1f(glGetUniformLocation(shader.ID, intensityName.c_str()), intensities[i]);
        }

        glUniform3fv(glGetUniformLocation(shader.ID, "viewPos"), 1, glm::value_ptr(camera.Position));
        glUniform3fv(glGetUniformLocation(shader.ID, "lightColor"), 1, glm::value_ptr(glm::vec3(1.0f)));

        if (camMode == Follow) {
            camera.SetBehindRobot(robot.position, robot.rotationY, deltaTime);

        }


        else if (camMode == Scanner) {
            camera.Zoom = 90.0f;
            camera.SetScannerView(robot.position, robot.rotationY);

        }

        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        std::vector<std::pair<Model*, glm::vec3>> models = {
        { &model1, glm::vec3(-6.0f, 1.4f, 0.0f) },   
        { &model2, glm::vec3(-3.0f,  0.4f, -0.8f) },   
        { &model3, glm::vec3(-0.19f,  1.0f, 0.0f) },   
        { &model4, glm::vec3(3.0f,  0.4f, 0.0f) },  
        { &model5, glm::vec3(6.0f,  1.15f, 0.3f) }    
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

        for (int i = 0; i < models.size(); ++i) {
            Model* model = models[i].first;
            glm::vec3 pos = models[i].second;
            glm::mat4 modelMat = glm::mat4(1.0f);
            modelMat = glm::translate(modelMat, pos);

            if (i == 0) { 
                modelMat = glm::scale(modelMat, glm::vec3(0.7f));
            }
            else if (i == 1) { 
                modelMat = glm::scale(modelMat, glm::vec3(1.0f));
            }
            else if (i == 2) { 
                modelMat = glm::scale(modelMat, glm::vec3(1.0f));
            }
            else if (i == 3) { 
                modelMat = glm::scale(modelMat, glm::vec3(0.6f));
            }
            else if (i == 4) {
                modelMat = glm::scale(modelMat, glm::vec3(0.45f));
            }

            shader.setMat4("model", modelMat);
            glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), true);
            glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 1.0f, 1.0f, 1.0f);
            model->Draw(shader);
        }

        if (armAngle >= 60.0f) {
            glUseProgram(shader.ID);
            glm::mat4 rayModel = glm::mat4(1.0f);
            shader.setMat4("model", rayModel);
            glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), false);
            glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 1.0f, 0.0f, 0.0f);

            glBindVertexArray(rayVAO);
            glDrawArrays(GL_LINES, 0, 2);
        }

        for (int i = 0; i < 2; ++i) {
            std::string lightName = "pointLights[" + std::to_string(i) + "]";
            glUniform3fv(glGetUniformLocation(shader.ID, lightName.c_str()), 1, glm::value_ptr(pointLights[i]));

            std::string intensityName = "pointIntensities[" + std::to_string(i) + "]";
            glUniform1f(glGetUniformLocation(shader.ID, intensityName.c_str()), pointIntensities[i]);
        }
        for (int i = 0; i < 2; ++i) {
            std::string colorName = "pointColors[" + std::to_string(i) + "]";
            glUniform3fv(glGetUniformLocation(shader.ID, colorName.c_str()), 1, glm::value_ptr(pointColors[i]));
        }


        robot.draw(shader, armAngle);


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