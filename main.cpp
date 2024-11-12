#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include "shader.h"
#include "camera.h"
#include "model.h"
#include <iostream>
#include <random>

#include "animator.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(vector<std::string> faces);

// window settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 30.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting position
// Define light properties for both lights
struct DirLight {
    glm::vec3 direction;
    glm::vec3 color;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;

    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

DirLight MoonLight;       // Also the moon is shown in the scene but I want it more like an ambient light, to create a night atmosphere
SpotLight LHLight;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PostApocalyse", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader seaShader("vertexShaders/SeaWave_vs.txt", "fragmentShaders/SeaWave_fs.txt");
    Shader skyboxShader("vertexShaders/Sky_vs.txt", "fragmentShaders/Sky_fs.txt");
    Shader moonShader("vertexShaders/Moon_vs.txt", "fragmentShaders/Moon_fs.txt");
    Shader modelShader("vertexShaders/Model_vs.txt", "fragmentShaders/Model_fs.txt");
    Shader fishmanShader("vertexShaders/ModelAnim_vs.txt", "fragmentShaders/ModelAnim_fs.txt");
    //Shader fishShader("vertexShaders/fish_vs.txt", "fragmentShaders/fish_fs.txt");
    //Shader crawlingShader("vertexShaders/ModelAnim_vs.txt", "fragmentShaders/ModelAnim_fs.txt");
    //Shader normalTextureSahder("vertexShaders/Moon_vs.txt", "fragmentShaders/Moon_fs .txt");

    // load models
    // -----------
    Model theMoon("models/NASA CGI Moon Kit/NASA CGI Moon Kit.obj");
    Model farIsland("models/Kauai Hawaii/Kauai Hawaii.obj");
    Model closeIsland("models/Kauai Hawaii/Kauai Hawaii.obj");
    Model cthulhu("models/Cthulhu/Horror_low_subd.obj");
    Model lighthouse("models/lighthouse/Phare.obj");
    Model lighthouseLamp("models/LighthouseLamp/LighthouseLamp.obj");
    

    // load animation models
    Model fishman("models/Praying/prayFishman.fbx");
    Animation prayingFishman("models/Praying/prayFishman.fbx", &fishman);
    Animator praying(&prayingFishman);

    Model zombie("models/fishman/Zombie Crawl.dae");
    Animation crawlingFishman("models/fishman/Zombie Crawl.dae", &zombie);
    Animator crawling(&crawlingFishman);

    Model crouchZombie("models/fishman/Male Crouch Pose.dae");
    Animation crouchFishman("models/fishman/Male Crouch Pose.dae", &crouchZombie);
    Animator crouch(&crouchFishman);

    Model fishCrowd("models/rainbow_trout/scene.gltf");
    Animation swimFish("models/rainbow_trout/scene.gltf", &fishCrowd);
    Animator swimming(&swimFish);

    /*Model tentacle("models/kraken/tentacle.gltf");
    Animation twistTentacle("models/kraken/tentacle.gltf", &tentacle);
    Animator twisting(&twistTentacle);*/

    // create the skybox
    // -----------------
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // load textures
    // -------------
    vector<std::string> faces
    {
        "models/lightblue/right.png",
        "models/lightblue/left.png",
        "models/lightblue/top.png",
        "models/lightblue/bot.png",
        "models/lightblue/front.png",
        "models/lightblue/back.png",
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
    // -----------------------
    // skybox end

    // create the sea plane
    // ---------------------
    struct Vertex {
        glm::vec3 Position;
        glm::vec2 TexCoords;
    };

    std::vector<Vertex> seaVertices;
    for (int z = -100; z <= 100; ++z) {
        for (int x = -100; x <= 100; ++x) {
            Vertex vertex;
            vertex.Position = glm::vec3(x, 0, z);

            // texture coordinates
            vertex.TexCoords = glm::vec2((x + 100.0f) / 200.0f, (z + 100.0f) / 200.0f);

            seaVertices.push_back(vertex);
        }
    }

    // create indices of VBO
    std::vector<unsigned int> seaIndices;
    int pointsPerLine = 201;
    for (int z = 0; z < pointsPerLine - 1; ++z) {
        for (int x = 0; x < pointsPerLine - 1; ++x) {
            unsigned int topLeft = z * pointsPerLine + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = topLeft + pointsPerLine;
            unsigned int bottomRight = bottomLeft + 1;

            // First triangle
            seaIndices.push_back(topLeft);
            seaIndices.push_back(bottomLeft);
            seaIndices.push_back(topRight);

            // Second triangle
            seaIndices.push_back(topRight);
            seaIndices.push_back(bottomLeft);
            seaIndices.push_back(bottomRight);
        }
    }

    unsigned int seaVBO, seaVAO, seaEBO;
    glGenVertexArrays(1, &seaVAO);
    glGenBuffers(1, &seaVBO);
    glGenBuffers(1, &seaEBO);

    glBindVertexArray(seaVAO);

    glBindBuffer(GL_ARRAY_BUFFER, seaVBO);
    glBufferData(GL_ARRAY_BUFFER, seaVertices.size() * sizeof(Vertex), seaVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, seaEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, seaIndices.size() * sizeof(unsigned int), seaIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // load the sea texture
    // --------------------
    unsigned int seaTexture;
    glGenTextures(1, &seaTexture);
    glBindTexture(GL_TEXTURE_2D, seaTexture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load("models/BodyOfWater_square.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    seaShader.use();
    seaShader.setInt("seaTexture", 1);
    // sea end
    // -------------------------------

    // variables for moving crowd
    double animationStartTime = glfwGetTime();

    // create random numbers to make the crowd randomly
    std::random_device rd;  // a random seed
    std::mt19937 gen(rd()); // mersenne_twister_engine
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    std::vector<glm::vec3> randomOffsets;
    for (int i = 0; i < 50; ++i) {
        randomOffsets.push_back(glm::vec3(dis(gen), dis(gen), dis(gen)));
    }

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);
        praying.UpdateAnimation(deltaTime);
        crawling.UpdateAnimation(deltaTime);
        crouch.UpdateAnimation(deltaTime);
        swimming.UpdateAnimation(deltaTime);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw the scene without characters
        // ---------------------------------
        // view/projection transformations
        // parameters: (field of view(angle), aspect of width and height, near plane position, far plane position)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        MoonLight.direction = glm::vec3(-1.0f, -1.0f, 1.0f);
        MoonLight.color = glm::vec3(1.0f);                     // white
        MoonLight.ambient = glm::vec3(0.3f);
        MoonLight.diffuse = glm::vec3(0.4f);
        MoonLight.specular = glm::vec3(0.5f);

        LHLight.color = glm::vec3(35.0f, 35.0f, 10.0f);        // yellow, strong spot light
        LHLight.position = glm::vec3(-25.0f, 21.0f, -2.0f);
        LHLight.direction = glm::vec3(0.0f, -2.0f, 1.0f);
        LHLight.linear = 0.09f;
        LHLight.quadratic = 0.032f;
        LHLight.ambient = glm::vec3(0.1f);
        LHLight.diffuse = glm::vec3(0.8f);
        LHLight.specular = glm::vec3(1.0f);
        LHLight.cutOff = glm::cos(glm::radians(12.5f));
        LHLight.outerCutOff = glm::cos(glm::radians(15.0f));

        // draw the skybox
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);


        // draw the sea
        view = camera.GetViewMatrix();
        glEnable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, seaTexture);
        seaShader.use();
        seaShader.setInt("seaTexture", 1);
        seaShader.setVec3("objectColor", 0.0f, 0.3f, 0.4f);         // dark blue sea
        seaShader.setVec3("dirLight.color", MoonLight.color);
        seaShader.setVec3("dirLight.direction", MoonLight.direction);
        seaShader.setVec3("dirLight.ambient", MoonLight.ambient);
        seaShader.setVec3("dirLight.diffuse", MoonLight.diffuse);
        seaShader.setVec3("dirLight.specular", MoonLight.specular);
        seaShader.setVec3("spotLight.color", LHLight.color);
        seaShader.setVec3("spotLight.position", LHLight.position);
        seaShader.setVec3("spotLight.direction", LHLight.direction);
        seaShader.setFloat("spotLight.linear", LHLight.linear);
        seaShader.setFloat("spotLight.quadratic", LHLight.quadratic);
        seaShader.setVec3("spotLight.ambient", LHLight.ambient);
        seaShader.setVec3("spotLight.diffuse", LHLight.diffuse);
        seaShader.setVec3("spotLight.specular", LHLight.specular);
        seaShader.setFloat("spotLight.cutOff", LHLight.cutOff);
        seaShader.setVec3("viewPos", camera.Position);
        seaShader.setFloat("time", glfwGetTime());
        seaShader.setFloat("frequency", 0.5f);
        seaShader.setFloat("amplitude", 0.2f);
        seaShader.setMat4("projection", projection);
        seaShader.setMat4("view", view);
        seaShader.setMat4("model", model);

        glBindVertexArray(seaVAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(seaIndices.size()), GL_UNSIGNED_INT, 0);


        // draw the moon
        moonShader.use();
        moonShader.setVec3("moonGlowColor", MoonLight.color);
        moonShader.setVec3("viewPos", camera.Position);
        moonShader.setMat4("projection", projection);
        moonShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-15.0f, 5.0f, -80.0f));
        //model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        //model = glm::scale(model, glm::vec3(0.7f)); // a smaller moon
        moonShader.setMat4("model", model);
        theMoon.Draw(moonShader);

        // draw the lighthouse
        modelShader.use();
        modelShader.setVec3("dirLight.color", MoonLight.color);
        modelShader.setVec3("dirLight.direction", MoonLight.direction);
        modelShader.setVec3("dirLight.ambient", MoonLight.ambient);
        modelShader.setVec3("dirLight.diffuse", MoonLight.diffuse);
        modelShader.setVec3("dirLight.specular", MoonLight.specular);
        modelShader.setVec3("spotLight.color", LHLight.color);
        modelShader.setVec3("spotLight.position", LHLight.position);
        modelShader.setFloat("spotLight.linear", LHLight.linear);
        modelShader.setFloat("spotLight.quadratic", LHLight.quadratic);
        modelShader.setVec3("spotLight.ambient", LHLight.ambient);
        modelShader.setVec3("spotLight.diffuse", LHLight.diffuse);
        modelShader.setVec3("spotLight.specular", LHLight.specular);
        modelShader.setFloat("spotLight.cutOff", LHLight.cutOff);
        modelShader.setFloat("spotLight.outerCutOff", LHLight.outerCutOff);
        modelShader.setVec3("viewPos", camera.Position);
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-25.0f, 7.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        modelShader.setMat4("model", model);
        lighthouse.Draw(modelShader);

        // draw the lighthouse lamp
        // based on the lighthouse position
        modelShader.use();
        model = glm::translate(model, glm::vec3(0.0f, 27.0f, 0.0f));
        model = glm::scale(model, glm::vec3(100.0f));
        modelShader.setMat4("model", model);
        lighthouseLamp.Draw(modelShader);

        // draw the far away island
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(30.0f, -0.3f, -70.0f));
        model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.0013f));
        modelShader.setMat4("model", model);
        farIsland.Draw(modelShader);

        // draw the Cthulhu statues
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-55.0f, 0.1f, -55.0f));
        model = glm::rotate(model, glm::radians(5.0f) * (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(8.0f));
        modelShader.setMat4("model", model);
        cthulhu.Draw(modelShader);

        // draw the close island
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-55.0f, -0.5f, 40.0f));
        model = glm::rotate(model, glm::radians(-120.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.003f));
        modelShader.setMat4("model", model);
        closeIsland.Draw(modelShader);


        // draw the praying fishman
        fishmanShader.use();
        fishmanShader.setVec3("dirLight.color", MoonLight.color);
        fishmanShader.setVec3("dirLight.direction", MoonLight.direction);
        fishmanShader.setVec3("dirLight.ambient", MoonLight.ambient);
        fishmanShader.setVec3("dirLight.diffuse", MoonLight.diffuse);
        fishmanShader.setVec3("dirLight.specular", MoonLight.specular);
        fishmanShader.setVec3("spotLight.color", LHLight.color);
        fishmanShader.setVec3("spotLight.position", LHLight.position);
        fishmanShader.setFloat("spotLight.linear", LHLight.linear);
        fishmanShader.setFloat("spotLight.quadratic", LHLight.quadratic);
        fishmanShader.setVec3("spotLight.ambient", LHLight.ambient);
        fishmanShader.setVec3("spotLight.diffuse", LHLight.diffuse);
        fishmanShader.setVec3("spotLight.specular", LHLight.specular);
        fishmanShader.setFloat("spotLight.cutOff", LHLight.cutOff);
        fishmanShader.setFloat("spotLight.outerCutOff", LHLight.outerCutOff);
        fishmanShader.setVec3("viewPos", camera.Position);
        fishmanShader.setMat4("projection", projection);
        fishmanShader.setMat4("view", view);

        auto transform = praying.GetFinalBoneMatrices();
        for (int i = 0; i < transform.size(); ++i) {
            fishmanShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transform[i]);
        }

        // render the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-20.0f, 3.5f, -7.0f));
        model = glm::rotate(model, glm::radians(-130.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.03f));
        fishmanShader.setMat4("model", model);
        fishman.Draw(fishmanShader);

        // draw the crawling crowd
        transform = crawling.GetFinalBoneMatrices();
        for (int i = 0; i < transform.size(); ++i) {
            fishmanShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transform[i]);
        }

        /*float duration = 20.0f;
        float moveFraction = fmin(glfwGetTime() / duration, 1.0f);
        glm::vec3 startPosition = glm::vec3(-800.0f, 0.0f, -1600.0f);
        glm::vec3 endPosition = glm::vec3(-200.0f, 0.0f, -400.0f);
        glm::vec3 currentPosition = startPosition + (endPosition - startPosition) * moveFraction;*/

        glm::mat4 model_1 = glm::mat4(1.0f);
        model_1 = glm::translate(model_1, glm::vec3(-200.0f, 0.0f, -400.0f));
        model_1 = glm::rotate(model_1, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model_1 = glm::scale(model_1, glm::vec3(700.0f));
        model_1 = model * model_1;
        fishmanShader.setMat4("model", model_1);
        zombie.Draw(fishmanShader);
        for (int i = 0; i < 2; ++i) {
            model_1 = glm::translate(model_1, glm::vec3(0.0f, -0.15f, -0.3f));
            fishmanShader.setMat4("model", model_1);
            zombie.Draw(fishmanShader);
        }

        glm::mat4 model_2 = glm::mat4(1.0f);
        model_2 = glm::translate(model_2, glm::vec3(0.0f, 0.0f, -400.0f));
        model_2 = glm::scale(model_2, glm::vec3(700.0f));
        model_2 = model * model_2;
        fishmanShader.setMat4("model", model_2);
        zombie.Draw(fishmanShader);
        for (int i = 0; i < 3; ++i) {
            model_2 = glm::translate(model_2, glm::vec3(0.0f, 0.0f, -0.3f));
            model_2 = glm::rotate(model_2, glm::radians(-35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            fishmanShader.setMat4("model", model_2);
            zombie.Draw(fishmanShader);
        }

        glm::mat4 model_3 = glm::mat4(1.0f);
        model_3 = glm::translate(model_3, glm::vec3(200.0f, 0.1f, -400.0f));
        model_3 = glm::rotate(model_3, glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model_3 = glm::scale(model_3, glm::vec3(700.0f));
        model_3 = model * model_3;
        fishmanShader.setMat4("model", model_3);
        zombie.Draw(fishmanShader);
        for (int i = 0; i < 3; ++i) {
            model_3 = glm::translate(model_3, glm::vec3(0.0f, 0.1f, -0.3f));
            model_3 = glm::rotate(model_3, glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            fishmanShader.setMat4("model", model_3);
            zombie.Draw(fishmanShader);
        }

        glm::mat4 model_4 = glm::mat4(1.0f);
        model_4 = glm::translate(model_4, glm::vec3(-400.0f, 0.0f, -200.0f));
        model_4 = glm::rotate(model_4, glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model_4 = glm::scale(model_4, glm::vec3(700.0f));
        model_4 = model * model_4;
        fishmanShader.setMat4("model", model_4);
        zombie.Draw(fishmanShader);
        for (int i = 0; i < 4; ++i) {
            model_4 = glm::translate(model_4, glm::vec3(0.0f, -0.1f, -0.3f));
            fishmanShader.setMat4("model", model_4);
            zombie.Draw(fishmanShader);
        }


        // draw the schooling fish
        // --------------------------
        transform = swimming.GetFinalBoneMatrices();
        for (int i = 0; i < transform.size(); ++i) {
            fishmanShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transform[i]);
        }
        

        const double animationDuration = 10.0f;
        double timeSinceStart = glfwGetTime() - animationStartTime;

        // a cycletime includes forwards and backwards time
        double cycleTime = fmod(timeSinceStart, animationDuration * 2);
        // the direction of the animation
        bool goingTowardsB = cycleTime <= animationDuration;

        // calculate the period of time
        float t = (float)(cycleTime / animationDuration);
        t = goingTowardsB ? t : 2.0f - t;

        glm::vec3 startPos = glm::vec3(10.0f, -3.0f, -40.0f);
        glm::vec3 endPos = glm::vec3(-35.0f, -3.0f, -40.0f);

        glm::vec3 direction;
        if (goingTowardsB) {
            direction = glm::normalize(endPos - startPos);
        }
        else {
            direction = glm::normalize(startPos - endPos);
        }

        glm::vec3 currentPos = glm::mix(startPos, endPos, t);       // calculate the interpolation
        currentPos.y += 10.0f * sin(glm::pi<float>() * t);          // sin wave track

        // change moving direction when backwards
        float rotationAngle = goingTowardsB ? 0.0f : glm::pi<float>();
        glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

        model = glm::mat4(1.0f);
        model = glm::translate(model, currentPos);
        model *= rotationMat;
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.2f));
        fishmanShader.setMat4("model", model);
        fishCrowd.Draw(fishmanShader);

        // create the crowds
        glm::mat4 model_school = glm::mat4(1.0f);
        model_school = model * model_school;
        model_school = glm::translate(model_school, glm::vec3(1.0f, -0.5f, 1.0f));
        //model_school = glm::rotate(model_school, glm::radians(-10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        for (int i = 0; i < 30; ++i) {
            model_school = glm::translate(model_school, 1.5f * randomOffsets[i]);
            //model_school = glm::rotate(model_school, glm::radians(-10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            fishmanShader.setMat4("model", model_school);
            fishCrowd.Draw(fishmanShader);
        }

        // end of the scene
        // --------------------

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    // check if the left button of the mouse is pressed
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset);
    }
    else
    {
        firstMouse = true; // Reset the initial state if the mouse button is not pressed
    }
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}