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

bool autoMode = false; // global tanım


// === Shader kaynakları ===
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


// === Callback fonksiyonları ===
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
    float speed = deltaTime * 5.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        nextPos.z -= speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        nextPos.z += speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        nextPos.x -= speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        nextPos.x += speed;

    // Sınırlardan dışarı çıkmasın
    float robotRadius = 0.6f;
    if (nextPos.x < -10.0f + robotRadius || nextPos.x > 10.0f - robotRadius ||
        nextPos.z < -5.0f + robotRadius || nextPos.z > 5.0f - robotRadius)
        return;

    // Obje çarpışma kontrolü (manuel için)
    for (const auto& obj : obstacles)
    {
        float collisionRadius = 1.2f; // modelin kapladığı alan
        if (glm::distance(nextPos, obj) < collisionRadius)
            return; // çok yaklaştı, çarpışma oldu
    }

    robot.position = nextPos;
}

std::vector<std::string> modelInfoTexts = {
    u8"Kadın Portre Steli  Roma Dönemi, M.S. 2. yüzyıl Bu küçük boyutlu mezar steli, muhtemelen yerel bir Roma vatandaşına ait olup M.S. 2. yüzyıla tarihlenmektedir.Üzerinde kadın figürü yarım kabartma şeklinde yer alır; başı örtülü, göğsünde fibula(giysi tokası) olan sade ama anlam yüklü bir betimleme sunar.Üstteki üçgen alınlık kısmında sembolik bezemeler bulunur.Bu tür steller, Roma dönemi Anadolu’sunda kadınların sosyal kimliğini, ailevi bağlarını ve inanç sistemini yansıtan önemli belgelerdir.",
    u8"Erkek Heykeli  Roma Dönemi, M.S. 2. yüzyıl Bu tunç heykel, Roma İmparatorluğu'nun yüksek sanat anlayışını temsil eden, Toga giymiş bir erkek yurttaşı betimler. M.S. 2. yüzyıla tarihlenen bu eser, özellikle portre detaylarındaki gerçekçilikle dikkat çeker. Togası, dönemin sosyal statüsünü simgelerken, sağ eliyle tuttuğu belge ya da rulo figürü onun entelektüel ya da siyasal bir kimliği olabileceğini düşündürür. Bu tip heykeller, Roma kentlerinin kamu alanlarında imparatorluk değerlerini ve vatandaşlık bilincini yansıtmak için sergilenmiştir.",
    u8"Filozoflar Lahdi  M.S. 3. yüzyıl, Roma Dönemi Bu mermer lahit, stoacı düşünce ve felsefi yaşamı simgeleyen figürlerle bezelidir.Lahdin uzun yüzeylerinde toga giymiş filozoflar ellerinde kitap, papirüs veya düşünür pozlarında gösterilmiştir.Bu tasvirler, ölen kişinin entelektüel bir yaşamı benimsediğini ve ölümden sonra da bilgelik içinde anılmak istendiğini ifade eder.Yazıtlarında Yunanca kitabeler yer alır.Bu tip lahitler, özellikle Roma İmparatorluğu’nun doğu eyaletlerinde seçkin sınıflar arasında yaygındı.",
    u8"Arabalı Tanrı Tarhunda (Fırtına Tanrısı)  Geç Hitit Dönemi, M.Ö. 9. yüzyıl Tarhunda, Anadolu'nun Geç Hitit dönemine ait en önemli tanrılarından biridir. Bu anıtsal taş heykel, Tarhunda’yı bir savaş arabası üzerinde iki boğa tarafından çekilirken tasvir eder. Tanrı’nın sağ elindeki balta ve sol elindeki yıldırım demeti, onun gök gürültüsü, savaş ve bereketle ilişkilendirilen doğasını simgeler. M.Ö. 9. yüzyıla tarihlenen bu eser, özellikle Arslantaş (eski Kummuh Krallığı) bölgesinden çıkarılmıştır ve Geç Hitit sanatının görkemli bir örneğidir.",
    u8"Akhilleus Lahdi  M.S. 2. yüzyıl, Roma Dönemi Akhilleus Lahdi, Troya Savaşı’nın kahramanı Akhilleus’un yaşamından sahnelerle süslenmiş yüksek kabartmalı bir mezar anıtıdır.M.S. 2. yüzyılda yapılmış olan bu lahit, ölen kişinin kahramanlıkla özdeşleştirilmek istendiğini gösterir.Lahdin bir yüzünde Akhilleus’un Briseis’i Agamemnon’a teslim edişi, diğer yüzlerde ise cenaze töreni ve savaş sahneleri yer alır.Mitolojik betimlemeleriyle yalnızca sanatsal değil, aynı zamanda simgesel anlatımıyla da dikkat çeker."
};



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

    static const ImWchar turkish_range[] = {
        0x0020, 0x00FF, // Latin-1
        0x011E, 0x011F, // Ğğ
        0x0130, 0x0131, // İı
        0x015E, 0x015F, // Şş
        0
    };

    ImFontConfig font_cfg;
    font_cfg.OversampleH = 3;
    font_cfg.OversampleV = 1;
    font_cfg.PixelSnapH = true;

    io.FontDefault = io.Fonts->AddFontFromFileTTF(
        "C:\\Users\\zeliha\\source\\repos\\Project1\\x64\\Debug\\fonts\\OpenSans-Regular.ttf", 16.0f, &font_cfg, turkish_range);

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    static bool showInfoPopup = false;
    static float popupTimer = 0.0f;
    static int lastScannedIndex = -1;

    Shader shader(vertexShaderSource, fragmentShaderSource);

    Model model1("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model1.obj");
    Model model2("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model2.obj");
    Model model3("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model3.obj");
    Model model4("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model4.obj");
    Model model5("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/model5.obj");

    Robot robot("C:/Users/zeliha/source/repos/Project1/x64/Debug/models/robot.obj", glm::vec3(-5.0f, 0.0f, 2.5f));


    std::vector<glm::vec3> objectPositions = {
        glm::vec3(-6.0f, 0.0f, 0.0f),
        glm::vec3(-3.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(3.0f, 0.0f, 0.0f),
        glm::vec3(6.0f, 0.0f, 0.0f)
    };

    static int currentTarget = 0;

    std::vector<glm::vec3> fullPath = {
        glm::vec3(-5.0f, 0.0f, 2.5f),  // Başlangıç
        glm::vec3(-6.0f, 0.0f, 1.2f),  // Model 1 önü
        glm::vec3(-3.0f, 0.0f, 1.2f),  // Model 2 önü
        glm::vec3(0.0f, 0.0f, 1.2f),   // Model 3 önü
        glm::vec3(3.0f, 0.0f, 1.2f),   // Model 4 önü
        glm::vec3(6.0f, 0.0f, 1.2f),   // Model 5 önü
        glm::vec3(-5.0f, 0.0f, 2.5f)   // Geri dönüş
    };


    static int pathIndex = 0;
    static bool isWaiting = false;
    static float waitTimer = 0.0f;


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
        ImGui::Checkbox("Auto Mode", &autoMode);
        if (!autoMode) {
            if (ImGui::Button("Left"))  robot.position.x -= deltaTime * 5.0f;
            if (ImGui::Button("Right")) robot.position.x += deltaTime * 5.0f;
            if (ImGui::Button("Forward")) robot.position.z -= deltaTime * 5.0f;
            if (ImGui::Button("Back")) robot.position.z += deltaTime * 5.0f;
        }
        ImGui::End();

        if (showInfoPopup && lastScannedIndex >= 0 && lastScannedIndex < modelInfoTexts.size()) {
            ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Always);

            ImGui::Begin("Model Bilgisi", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            ImGui::TextWrapped("%s", modelInfoTexts[lastScannedIndex].c_str());
            ImGui::End();
        }

        if (!autoMode) {
            processInput(window, robot, deltaTime, objectPositions);

            bool found = false;
            for (int i = 0; i < objectPositions.size(); ++i) {
                if (glm::distance(robot.position, objectPositions[i]) < 1.5f) {
                    lastScannedIndex = i;
                    showInfoPopup = true;
                    found = true;
                    break;
                }
            }
            if (!found) {
                showInfoPopup = false;
                lastScannedIndex = -1;
            }
        }
        else {
            glm::vec3 target = fullPath[pathIndex];

            if (!isWaiting) {
                float dist = glm::distance(robot.position, target);

                if (dist < 0.2f) {
                    isWaiting = true;
                    waitTimer = 0.0f;

                    if (pathIndex > 0 && pathIndex <= 5) {
                        lastScannedIndex = pathIndex - 1;
                        showInfoPopup = true;
                        popupTimer = 0.0f;
                    }
                }
                else {
                    glm::vec3 direction = glm::normalize(target - robot.position);
                    glm::vec3 nextPos = robot.position + direction * deltaTime * 2.0f;

                    float robotRadius = 0.5f;
                    bool inBounds = nextPos.x >= -10.0f + robotRadius && nextPos.x <= 10.0f - robotRadius &&
                        nextPos.z >= -5.0f + robotRadius && nextPos.z <= 5.0f - robotRadius;

                    if (inBounds) {
                        robot.moveTo(target, deltaTime * 2.0f);
                    }
                }
            }
            else {
                waitTimer += deltaTime;

                if (waitTimer >= 3.0f) {
                    isWaiting = false;
                    pathIndex++;

                    if (pathIndex >= fullPath.size()) {
                        pathIndex = 0;
                    }
                }

                popupTimer += deltaTime;
                if (popupTimer >= 3.0f) {
                    showInfoPopup = false;
                }
            }
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