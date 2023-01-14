#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_m.h"
#include "camera.h"
#include "CyclicBuffer2D.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <random>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void push_point(std::vector<float>& vec, float x, float y, float z)
{
    vec.push_back(x);
    vec.push_back(y);
    vec.push_back(z);
}

std::vector<float> generateChunk(std::vector<float> &ys)
{
    int size = 51;
    std::vector<float> result;
    for (int i = 0; i < size - 1; ++i)
    {
        for (int j = 0; j < size - 1; ++j)
        {
            push_point(result, i, ys[i * size + j], j);
            push_point(result, i + 1, ys[(i + 1) * size + j], j);
            push_point(result, i, ys[i * size + j + 1], j + 1);

            push_point(result, i + 1, ys[(i + 1) * size + j + 1], j + 1);
            push_point(result, i + 1, ys[(i + 1) * size + j], j);
            push_point(result, i, ys[i * size + j + 1], j + 1);
        }
    }
    return result;
}


float interpolate(float a0, float a1, float w)
{
    return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
}

float random_param = 1.0;

glm::vec2 randomGradient(int ix, int iy) {
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;
    unsigned a = ix, b = iy;
    a *= 3284157443; b ^= a << s | a >> w - s;
    b *= 1911520717; a ^= b << s | b >> w - s;
    a *= 2048419325;
    float random = a * random_param;
    glm::vec2 v;
    v.x = cos(random); v.y = sin(random);
    return v;
}

float dotGridGradient(int ix, int iy, float x, float y) {
    glm::vec2 gradient = randomGradient(ix, iy);

    float dx = x - (float)ix;
    float dy = y - (float)iy;

    return (dx * gradient.x + dy * gradient.y);
}

float perlin(float x, float y) {
    int x0 = (int)floor(x);
    int x1 = x0 + 1;
    int y0 = (int)floor(y);
    int y1 = y0 + 1;

    float sx = x - (float)x0;
    float sy = y - (float)y0;

    float n0, n1, ix0, ix1, value;

    n0 = dotGridGradient(x0, y0, x, y);
    n1 = dotGridGradient(x1, y0, x, y);
    ix0 = interpolate(n0, n1, sx);

    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    ix1 = interpolate(n0, n1, sx);

    value = interpolate(ix0, ix1, sy);
    return value;
}

std::vector<float> generateHeights(float x0, float y0)
{
    std::vector<float> result;
    for (int i = 0; i < 51; ++i)
    {
        for (int j = 0; j < 51; ++j)
        {
            float x = i;
            float y = j;
            float x_fin = (x + x0 * 50) / 51;
            float y_fin = (y + y0 * 50) / 51;
            result.push_back(perlin(x_fin, y_fin) * 130);
        }
    }
    return result;
}

glm::vec2 getIntCameraPos()
{
    glm::vec2 result;
    result.x = (int)camera.Position.x;
    result.y = (int)camera.Position.z;
    return result;
}

void cyclicTest()
{
    
}

int main()
{
    


    std::random_device rd;
    std::mt19937 e2(rd());
    std::uniform_real_distribution<> dist(0, 3);
    random_param = dist(e2);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    glEnable(GL_DEPTH_TEST);


    Shader ourShader("camera.vs", "camera.fs");


    constexpr int RENDER_DISTANCE = 5;
    constexpr int N_TILES = RENDER_DISTANCE * RENDER_DISTANCE;

    unsigned int VBO[N_TILES], VAO[N_TILES];
    glGenVertexArrays(N_TILES, VAO);
    glGenBuffers(N_TILES, VBO);
    for (int i = 0; i < RENDER_DISTANCE; ++i)
    {
        for (int j = 0; j < RENDER_DISTANCE; ++j)
        {
            auto ys = generateHeights(i - (RENDER_DISTANCE / 2), j - (RENDER_DISTANCE / 2));

            auto vertices = generateChunk(ys);

            glBindVertexArray(VAO[i * RENDER_DISTANCE + j]);

            glBindBuffer(GL_ARRAY_BUFFER, VBO[i * RENDER_DISTANCE + j]);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
        }
    }

    


    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        processInput(window);


        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        glm::vec2 intCameraPos = getIntCameraPos();

        std::cout << "Pos: x=" << intCameraPos.x << ", y =" << intCameraPos.y << std::endl;


        for (int i = 0; i < RENDER_DISTANCE; ++i)
        {
            for (int j = 0; j < RENDER_DISTANCE; ++j)
            {
                glBindVertexArray(VAO[i * RENDER_DISTANCE + j]);

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::scale(model, glm::vec3(0.025, 0.025, 0.025));
                model = glm::translate(model, glm::vec3(i - (RENDER_DISTANCE / 2) - 0.5, 0, j - (RENDER_DISTANCE / 2) - 0.5) * 50.0f);

                ourShader.setMat4("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 3 * 2 * 50 * 50);
            }
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
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
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}