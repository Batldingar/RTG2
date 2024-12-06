#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <util/assets.h>
#include <util/shader.h>
#include <util/camera.h>
#include <util/model.h>
#include <util/window.h>

constexpr int vertexCount = 36;
constexpr int colorComponentsPerVertex = 3;
constexpr int offsetComponentsPerVertex = 3;

using namespace glm;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void prepareCubes();
void renderCubes();

int WIDTH = 800;
int HEIGHT = 600;

vec3 cameraPos = vec3(0.0f, 0.0f, 0.75f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lightX = 8.0f;
float lightY = 1.0f;
float lightZ = 19.0f;

vec3 lightPos = vec3(lightX, lightY, lightZ);

bool mPressed = false;

const char *APP_NAME = "shading";

unsigned int texture;

unsigned char *image;

int width, height;

void loadTexture()
{

    int nrComponents;
    stbi_set_flip_vertically_on_load(true); // this flips the loaded images vertically
    image = stbi_load("../resources/images/klein.jpg", &width, &height, &nrComponents, 0);

    if (!image)
    {
        std::cout << "Failed to load texture" << std::endl;
    }
}

int main()
{
    InitWindowAndGUI(WIDTH, HEIGHT, APP_NAME);
    SetFramebufferSizeCallback(framebuffer_size_callback);

    Shader myShader("../src/06-shading/shading.vert", "../src/06-shading/shading.frag");
    glm::vec4 bgColor = {0.1, 0.1, 0.1, 1.0};
    glm::vec3 objectColor = {0.9, 0.7, 0.1};

    myShader.use();

    prepareCubes();

    glEnable(GL_DEPTH_TEST);

    // Main Loop
    while (!glfwWindowShouldClose(window))
    {

        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // START: UI-Stuff
        if (gui)
        {
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
            {
                ImGui::Begin(APP_NAME);
                ImGui::ColorEdit3("clear color", (float *)&bgColor.x); // Edit 3 floats representing a color
                ImGui::ColorEdit3("object color", value_ptr(objectColor));

                // a Button to reload the shader (so you don't need to recompile the cpp all the time)
                if (ImGui::Button("reload shaders"))
                {
                    auto &shader = myShader;
                    shader.reload();
                    shader.use();
                }

                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate); // show framerate
                ImGui::End();
            }
            ImGui::Render();
        }
        // END: UI-Stuff

        mat4 projection = perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.f);
        mat4 view = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                mat4 model = mat4(1.0f);
                // model = rotate(model, (float)glfwGetTime(), vec3(1.0f, 0.0f, 0.0f));
                model = rotate(model, 0.0f, vec3(1.0f, 0.0f, 0.0f));
                model = translate(model, vec3(0.0f, 0.0f, 0.0f));
                model = scale(model, vec3(0.2f, 0.2f, 0.2f));

                myShader.setMat4("projection", projection);
                myShader.setMat4("view", view);
                myShader.setMat4("model", model);

                myShader.setVec3("cameraPos", cameraPos);
                myShader.setVec3("lightPos", lightPos);

                renderCubes();

        if (gui)
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    stbi_image_free(image);
    DestroyWindow();
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

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
std::vector<unsigned int> cubeVAOS;

void prepareCubes()
{
    loadTexture();
    cubeVAOS.resize(width * height, 0);

        float positions[] = {
            // Back face
            -1.0f, -1.0f, -1.0f, // Vertex 1
            1.0f, -1.0f, -1.0f,  // Vertex 2
            1.0f, 1.0f, -1.0f,   // Vertex 3
            1.0f, 1.0f, -1.0f,   // Vertex 4
            -1.0f, 1.0f, -1.0f,  // Vertex 5
            -1.0f, -1.0f, -1.0f, // Vertex 6

            // Front face
            -1.0f, -1.0f, 1.0f, // Vertex 7
            1.0f, -1.0f, 1.0f,  // Vertex 8
            1.0f, 1.0f, 1.0f,   // Vertex 9
            1.0f, 1.0f, 1.0f,   // Vertex 10
            -1.0f, 1.0f, 1.0f,  // Vertex 11
            -1.0f, -1.0f, 1.0f, // Vertex 12

            // Left face
            -1.0f, 1.0f, 1.0f,   // Vertex 13
            -1.0f, 1.0f, -1.0f,  // Vertex 14
            -1.0f, -1.0f, -1.0f, // Vertex 15
            -1.0f, -1.0f, -1.0f, // Vertex 16
            -1.0f, -1.0f, 1.0f,  // Vertex 17
            -1.0f, 1.0f, 1.0f,   // Vertex 18

            // Right face
            1.0f, 1.0f, 1.0f,   // Vertex 19
            1.0f, -1.0f, -1.0f, // Vertex 20
            1.0f, 1.0f, -1.0f,  // Vertex 21
            1.0f, -1.0f, -1.0f, // Vertex 22
            1.0f, 1.0f, 1.0f,   // Vertex 23
            1.0f, -1.0f, 1.0f,  // Vertex 24

            // Bottom face
            -1.0f, -1.0f, -1.0f, // Vertex 25
            1.0f, -1.0f, -1.0f,  // Vertex 26
            1.0f, -1.0f, 1.0f,   // Vertex 27
            1.0f, -1.0f, 1.0f,   // Vertex 28
            -1.0f, -1.0f, 1.0f,  // Vertex 29
            -1.0f, -1.0f, -1.0f, // Vertex 30

            // Top face
            -1.0f, 1.0f, -1.0f, // Vertex 31
            1.0f, 1.0f, -1.0f,  // Vertex 32
            1.0f, 1.0f, 1.0f,   // Vertex 33
            1.0f, 1.0f, 1.0f,   // Vertex 34
            -1.0f, 1.0f, 1.0f,  // Vertex 35
            -1.0f, 1.0f, -1.0f  // Vertex 36
        };

        float normals[] = {
            // Back face normal points in negative Z direction
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,

            // Front face normal points in positive Z direction
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,

            // Left face normal points in negative X direction
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,

            // Right face normal points in positive X direction
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,

            // Bottom face normal points in negative Y direction
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,

            // Top face normal points in positive Y direction
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f};

        float uvs[] = {
            // Back face
            0.0f, 0.0f, // Vertex 1
            1.0f, 0.0f, // Vertex 2
            1.0f, 1.0f, // Vertex 3
            1.0f, 1.0f, // Vertex 4
            0.0f, 1.0f, // Vertex 5
            0.0f, 0.0f, // Vertex 6

            // Front face
            0.0f, 0.0f, // Vertex 7
            1.0f, 0.0f, // Vertex 8
            1.0f, 1.0f, // Vertex 9
            1.0f, 1.0f, // Vertex 10
            0.0f, 1.0f, // Vertex 11
            0.0f, 0.0f, // Vertex 12

            // Left face
            0.0f, 0.0f, // Vertex 13
            1.0f, 0.0f, // Vertex 14
            1.0f, 1.0f, // Vertex 15
            1.0f, 1.0f, // Vertex 16
            0.0f, 1.0f, // Vertex 17
            0.0f, 0.0f, // Vertex 18

            // Right face
            0.0f, 0.0f, // Vertex 19
            1.0f, 0.0f, // Vertex 20
            1.0f, 1.0f, // Vertex 21
            1.0f, 1.0f, // Vertex 22
            0.0f, 1.0f, // Vertex 23
            0.0f, 0.0f, // Vertex 24

            // Bottom face
            0.0f, 0.0f, // Vertex 25
            1.0f, 0.0f, // Vertex 26
            1.0f, 1.0f, // Vertex 27
            1.0f, 1.0f, // Vertex 28
            0.0f, 1.0f, // Vertex 29
            0.0f, 0.0f, // Vertex 30

            // Top face
            0.0f, 0.0f, // Vertex 31
            1.0f, 0.0f, // Vertex 32
            1.0f, 1.0f, // Vertex 33
            1.0f, 1.0f, // Vertex 34
            0.0f, 1.0f, // Vertex 35
            0.0f, 0.0f  // Vertex 36
        };

        for (int i = 0; i < width * height; ++i) {

            // Create the color array
            float colors[vertexCount * colorComponentsPerVertex] = {};

            // Calculate the RGB values from the image
            unsigned char r = image[i * 3];     // Red channel
            unsigned char g = image[i * 3 + 1]; // Green channel
            unsigned char b = image[i * 3 + 2]; // Blue channel

            for (int i = 0; i < vertexCount; ++i) {
                colors[i * colorComponentsPerVertex] = r / 255.0f;
                colors[i * colorComponentsPerVertex + 1] = g / 255.0f;
                colors[i * colorComponentsPerVertex + 2] = b / 255.0f;
            }

            float offsets[vertexCount * offsetComponentsPerVertex] = {};

            // Create the offset array
            float xOffset = (float)(i % width) * 3.0f;  // Get the remainder as a float
            float yOffset = (float)(i / width) * 3.0f; // Integer division gives the row number, cast to float
            float zOffset = 0.0f;

            for (int i = 0; i < vertexCount; ++i) {
                offsets[i * offsetComponentsPerVertex] = xOffset;
                offsets[i * offsetComponentsPerVertex + 1] = yOffset;
                offsets[i * offsetComponentsPerVertex + 2] = zOffset;
            }

            // ----- VBO CREATION -----
            GLuint positionVBO, normalVBO, uvVBO, colorVBO, offsetVBO;

            // Position VBO
            glGenBuffers(1, &positionVBO);
            glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

            // Normal VBO
            glGenBuffers(1, &normalVBO);
            glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

            // UV VBO
            glGenBuffers(1, &uvVBO);
            glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

            // Color VBO
            glGenBuffers(1, &colorVBO);
            glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

            // Offset VBO
            glGenBuffers(1, &offsetVBO);
            glBindBuffer(GL_ARRAY_BUFFER, offsetVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(offsets), offsets, GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind

            // ----- VAO CREATION -----
            glGenVertexArrays(1, &cubeVAOS[i]);
            glBindVertexArray(cubeVAOS[i]);

            // Positions (location = 0)
            glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

            // Normals (location = 1)
            glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

            // UVs (location = 2)
            glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

            // Colors (location = 3)
            glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

            // Offsets (location = 4)
            glBindBuffer(GL_ARRAY_BUFFER, offsetVBO);
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

            glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
            glBindVertexArray(0);             // Unbind VAO
        }
}

void renderCubes()
{
    for (int i = 0; i < width * height; ++i) {
        glBindVertexArray(cubeVAOS[i]);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }
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