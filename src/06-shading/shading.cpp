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

using namespace glm;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void prepareCube();
void renderCube();

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
    image = stbi_load("../resources/images/2.jpg", &width, &height, &nrComponents, 0);

    if (image)
    {

        stbi_image_free(image);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
}

int main()
{
    loadTexture();

    InitWindowAndGUI(WIDTH, HEIGHT, APP_NAME);
    SetFramebufferSizeCallback(framebuffer_size_callback);

    Shader myShader("../src/06-shading/shading.vert", "../src/06-shading/shading.frag");
    glm::vec4 bgColor = {0.1, 0.1, 0.1, 1.0};
    glm::vec3 objectColor = {0.9, 0.7, 0.1};

    myShader.use();

    prepareCube();

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

        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                mat4 model = mat4(1.0f);
                // model = rotate(model, (float)glfwGetTime(), vec3(1.0f, 0.0f, 0.0f));
                model = rotate(model, 0.0f, vec3(1.0f, 0.0f, 0.0f));
                model = translate(model, vec3(x, y, 0.0f));
                model = scale(model, vec3(0.2f, 0.2f, 0.2f));

                myShader.setMat4("projection", projection);
                myShader.setMat4("view", view);
                myShader.setMat4("model", model);

                myShader.setVec3("cameraPos", cameraPos);
                myShader.setVec3("lightPos", lightPos);
                myShader.setVec3("objectColor", objectColor);

                renderCube();
            }
        }

        if (gui)
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

void prepareCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // position, normal, uv
            // back face
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // top-left
            // front face
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
            -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
                                                                // right face
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,    // top-right
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,    // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f   // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void renderCube()
{
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
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