#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

// Cube variables
struct Cube
{
    vec3 position;
    vec3 size;
    bool hovered;
    bool moved;
};

// define functions
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
bool rayIntersectsAABB(const vec3 &origin, const vec3 &dir, const vec3 &center, const vec3 &size);
void renderCube(const Cube &cube);
unsigned int compileShader(unsigned int type, const char *source);
void prepareCubes();

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

vec3 cameraPos = vec3(0.0f, 0.0f, 3.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lightX = 8.0f;
float lightY = 1.0f;
float lightZ = 19.0f;

vec3 lightPos = vec3(lightX, lightY, lightZ);

// Shader source code
const char *vertexShaderSource = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)GLSL";

const char *fragmentShaderSource = R"GLSL(
#version 330 core
out vec4 FragColor;

uniform vec3 color;

void main() {
    FragColor = vec4(color, 1.0);
}
)GLSL";

std::vector<Cube> cubes;
unsigned int shaderProgram;
unsigned int VAO;

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Hover Cubes", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Compile and link shaders
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
        return -1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Initialize projection and view matrices
    mat4 projection = perspective(radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
    mat4 view = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    // Initialize cubes
    cubes.push_back({vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), false, false});
    cubes.push_back({vec3(2.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), false, false});
    cubes.push_back({vec3(-2.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), false, false});

    prepareCubes();

    while (!glfwWindowShouldClose(window))
    {

        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Poll Events
        glfwPollEvents();

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use the shader program
        glUseProgram(shaderProgram);

        // Set a shared View und Projection nur einmal vor dem Rendering
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

        mat4 view = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        mat4 projection = perspective(radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));

        // Render jedes Würfel
        for (auto &cube : cubes)
        {
            renderCube(cube); // Nur Render-Logik aufrufen
        }

        // Swap Buffers
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        lightX += 1.0;
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        lightY += 1.0;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        lightZ += 1.0;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        lightX -= 1.0;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        lightY -= 1.0;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        lightZ -= 1.0;

    lightPos = vec3(lightX, lightY, lightZ);
}

unsigned int compileShader(unsigned int type, const char *source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    if (shader == 0) // Shader nicht korrekt erstellt
    {
        std::cerr << "Shader creation failed!" << std::endl;
    }

    return shader;
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
void prepareCubes()
{
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, // Back face
        0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f, 0.5f, // Front face
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,

        -0.5f, 0.5f, 0.5f, // Left face
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,

        0.5f, 0.5f, 0.5f, // Right face
        0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,

        -0.5f, -0.5f, -0.5f, // Bottom face
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, 0.5f, -0.5f, // Top face
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f};

    glGenVertexArrays(1, &VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderCube(const Cube &cube)
{
    mat4 model = mat4(1.0f);
    model = translate(model, cube.position);
    model = scale(model, cube.size);

    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
    unsigned int colorLoc = glGetUniformLocation(shaderProgram, "color");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(lookAt(cameraPos, cameraPos + cameraFront, cameraUp)));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(perspective(radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f)));
    glUniform3f(colorLoc, cube.hovered ? 1.0f : 0.5f, 0.5f, 0.5f);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

bool rayIntersectsAABB(const vec3 &origin, const vec3 &dir, const vec3 &center, const vec3 &size)
{
    vec3 tMin = (center - size * 0.5f - origin) / dir;
    vec3 tMax = (center + size * 0.5f - origin) / dir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);

    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);

    return tNear <= tFar && tFar >= 0.0f;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    if (width > 0 && height > 0)
    {
        glViewport(0, 0, width, height);
    }
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    float x = (2.0f * xpos) / WIDTH - 1.0f;
    float y = 1.0f - (2.0f * ypos) / HEIGHT;
    vec3 ray_nds = vec3(x, y, -1.0f);

    vec4 ray_clip = vec4(ray_nds.x, ray_nds.y, -1.0f, 1.0f);
    vec4 ray_eye = inverse(perspective(radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f)) * ray_clip;
    ray_eye = vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
    vec3 ray_world = normalize(vec3(inverse(lookAt(cameraPos, cameraPos + cameraFront, cameraUp)) * ray_eye));

    bool hoveredAny = false;
    for (auto &cube : cubes)
    {
        if (rayIntersectsAABB(cameraPos, ray_world, cube.position, cube.size))
        {
            hoveredAny = true;
            vec3 targetPosition = cube.position;
            targetPosition.z -= 0.5f;
            cube.position.z = mix(cube.position.z, targetPosition.z, 0.1f); // smooth interpolation
        }
    }

    if (!hoveredAny)
    {
        for (auto &cube : cubes)
        {
            cube.position.z = mix(cube.position.z, 0.0f, 0.1f);
        }
    }
}

// glfw: whenever any mouse button is pressed, this callback is called
// ----------------------------------------------------------------------
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
}
