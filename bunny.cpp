#include <iostream>
#include <fstream>

#ifdef USE_GLEW
#define GLEW_STATIC
#include <GL/glew.h>
#endif
#ifndef USE_GLEW
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Shader.hpp"

// Function prototypes
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_click_callback(GLFWwindow *window, int button, int action, int mods);
void do_movement();

//Window Size
const GLuint WIDTH = 1280, HEIGHT = 720;

// const
const char *DataFileName = "./myBunny.ply2";
const char *LightFileName = "./myLight.ply2";
const GLuint MAXPOINT = 40000;
const GLuint MAXVERTICES = MAXPOINT * 6;
const GLuint MAXTRIANGLE = 70000;
const GLuint MAXINDICES = MAXTRIANGLE * 3;
const char *SPLIT = "*******************************";

// Rabbit Model
GLuint PointNum, TriNum;
GLfloat vertices[MAXVERTICES];
glm::vec3 vecNormal[MAXPOINT];
GLuint indices[MAXINDICES];
// Light Model
GLuint LPointNum, LTriNum;
GLfloat Lvertices[8 * 3];
GLuint Lindices[12 * 3];

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 30.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
GLfloat yaw = -90.0f;
GLfloat pitch = 0.0f;
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
GLfloat fov = 45.0f;
bool keys[1024];
bool firstMouse = true;
bool cursorDisable;

// Deltatime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

//rabbit move
glm::mat4 rabbitModel(1.f);

//Light
glm::vec3 lightPos(0.0f, 30.0f, 0.0f);
glm::vec3 lightColor(1.f);
bool isAttenuation = true;

//Mouse
GLfloat mouseX = WIDTH / 2.0;
GLfloat mouseY = HEIGHT / 2.0;
GLuint selectedPointIndice = 0 - 1;

GLFWwindow *InitGL()
{
    std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "bunny", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_click_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    cursorDisable = true;
#ifdef USE_GLEW
    glewExperimental = GL_TRUE;
    glewInit();
#endif
#ifndef USE_GLEW
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }
#endif
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    std::cout << "Initial Successful!" << std::endl;
    std::cout << "WASD & up & down: Camera Movement" << std::endl;
    std::cout << "Mouse Movement : Camera Orientation" << std::endl;
    std::cout << "Mouse Scroll : Camera Field of View" << std::endl;
    std::cout << "Left Alt : Change Mouse Mode" << std::endl;
    std::cout << "<- & -> : Move the rabbit" << std::endl;
    std::cout << "IJKL : Rotate the rabbit" << std::endl;
    std::cout << "Mouse Scroll : Camera Field of View" << std::endl;
    std::cout << "Z & X : Scale the rabbit" << std::endl;
    std::cout << "C : Change whether add attenuation or not" << std::endl;
    std::cout << "= : switch light color to white" << std::endl;
    std::cout << "- : switch light color to colorful" << std::endl;
    std::cout << SPLIT << std::endl;
    return window;
}

void ReadDataRabbit()
{
    std::ifstream inFile(DataFileName);
    if (inFile.fail())
    {
        std::cout << "Error opening file" << std::endl;
        exit(1);
    }
    inFile >> PointNum >> TriNum;
    for (int i = 0; i < PointNum; i++)
    {
        inFile >> vertices[6 * i + 0] >> vertices[6 * i + 1] >> vertices[6 * i + 2];
    }
    for (int i = 0; i < TriNum; i++)
    {
        GLuint tmp_num, in1, in2, in3;
        inFile >> tmp_num >> in1 >> in2 >> in3;
        indices[i * 3] = in1;
        indices[i * 3 + 1] = in2;
        indices[i * 3 + 2] = in3;
        if (tmp_num != 3)
        {
            std::cout << "Error file format" << std::endl;
            exit(1);
        }
        glm::vec3 triangleOne = glm::vec3(vertices[in2 * 6] - vertices[in1 * 6], vertices[in2 * 6 + 1] - vertices[in1 * 6 + 1], vertices[in2 * 6 + 2] - vertices[in1 * 6 + 2]);
        glm::vec3 triangleTwo = glm::vec3(vertices[in3 * 6] - vertices[in2 * 6], vertices[in3 * 6 + 1] - vertices[in2 * 6 + 1], vertices[in3 * 6 + 2] - vertices[in2 * 6 + 2]);
        glm::vec3 triangleNormal = glm::normalize(glm::cross(triangleOne, triangleTwo));
        vecNormal[in1] += triangleNormal;
        vecNormal[in2] += triangleNormal;
        vecNormal[in3] += triangleNormal;
    }
    for (int i = 0; i < PointNum; i++)
    {
        vecNormal[i] = glm::normalize(vecNormal[i]);
    }
    for (int i = 0; i < PointNum; i++)
    {
        vertices[6 * i + 3] = vecNormal[i].x;
        vertices[6 * i + 4] = vecNormal[i].y;
        vertices[6 * i + 5] = vecNormal[i].z;
    }
    inFile.close();
}

void ReadDataLight()
{
    std::ifstream inFile(LightFileName);
    if (inFile.fail())
    {
        std::cout << "Error opening file" << std::endl;
        exit(1);
    }
    inFile >> LPointNum >> LTriNum;
    for (int i = 0; i < 3 * LPointNum; i++)
    {
        inFile >> Lvertices[i];
    }
    for (int i = 0; i < LTriNum; i++)
    {
        GLuint tmp_num;
        inFile >> tmp_num >> Lindices[i * 3] >> Lindices[i * 3 + 1] >> Lindices[i * 3 + 2];
        if (tmp_num != 3)
        {
            std::cout << "Error file format" << std::endl;
            exit(1);
        }
    }
    inFile.close();
}

void getVAO(GLuint &VAO, GLuint &LVAO, GLuint &VBO, GLuint &EBO)
{
    // Generate VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * PointNum * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * TriNum * sizeof(GLfloat), indices, GL_STATIC_DRAW);
    //Points
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    //Points Normal Data
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Generate LVAO
    glGenVertexArrays(1, &LVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(LVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Lvertices), Lvertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Lindices), Lindices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

int main()
{
    GLFWwindow *window = InitGL();
    ReadDataRabbit();
    ReadDataLight();
    Shader ourShader("bunny.vs", "bunny.frag");
    Shader lightShader("bunnyL.vs", "bunnyL.frag");
    Shader selectShader("bunnyP.vs", "bunnyP.frag");
    GLuint VAO, LVAO, VBO, EBO;
    getVAO(VAO, LVAO, VBO, EBO);
#ifdef GL_LINE_TRUE
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glfwPollEvents();
        do_movement();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.Use();
        //Light
        GLint lightPosLoc = glGetUniformLocation(ourShader.Program, "light.position");
        GLint viewPosLoc = glGetUniformLocation(ourShader.Program, "viewPos");
        glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
        // Set lights properties
        // recommand : diffuse:0.5 ambient:0.2
        glm::vec3 diffuseColor = lightColor * glm::vec3(1.f);
        glm::vec3 ambientColor = lightColor * glm::vec3(1.f);
        glm::vec3 specularColor = lightColor * glm::vec3(1.f);
        glUniform3f(glGetUniformLocation(ourShader.Program, "light.ambient"), ambientColor.x, ambientColor.y, ambientColor.z);
        glUniform3f(glGetUniformLocation(ourShader.Program, "light.diffuse"), diffuseColor.x, diffuseColor.y, diffuseColor.z);
        glUniform3f(glGetUniformLocation(ourShader.Program, "light.specular"), specularColor.x, specularColor.y, specularColor.z);
        // attenuation
        // radius 600
        glUniform1f(glGetUniformLocation(ourShader.Program, "light.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(ourShader.Program, "light.linear"), (isAttenuation ? 0.007 : 0.f));
        glUniform1f(glGetUniformLocation(ourShader.Program, "light.quadratic"), (isAttenuation ? 0.0002 : 0.f));
        // Set material properties
        // Gold
        // 0.24725 0.1995 0.0745 0.75164 0.60648 0.22648 0.628281 0.555802 0.366065 0.4
        glUniform3f(glGetUniformLocation(ourShader.Program, "material.ambient"), 0.2475f, 0.1995f, 0.0745f);
        glUniform3f(glGetUniformLocation(ourShader.Program, "material.diffuse"), 0.75146f, 0.60648f, 0.22648f);
        glUniform3f(glGetUniformLocation(ourShader.Program, "material.specular"), 0.628281f, 0.555802f, 0.366065f);
        glUniform1f(glGetUniformLocation(ourShader.Program, "material.shininess"), 0.4f * 128);
        /*
        // white plastic
        // 0.0 0.0 0.0 0.55 0.55 0.55 0.7 0.7 0.7 0.25
        glUniform3f(glGetUniformLocation(ourShader.Program, "material.ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(ourShader.Program, "material.diffuse"), 0.55f, 0.55f, 0.55f);
        glUniform3f(glGetUniformLocation(ourShader.Program, "material.specular"), 0.7f, 0.7f, 0.7f);
        glUniform1f(glGetUniformLocation(ourShader.Program, "material.shininess"), 0.25f * 128);
        */
        //Camera
        glm::mat4 view(1.f);
        glm::mat4 projection(1.f);
        glm::mat4 model(1.f);
        model = rabbitModel;
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        projection = glm::perspective(glm::radians(fov), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 1000.0f);
        GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 3 * TriNum, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        //Select Point
        if (!(selectedPointIndice > MAXINDICES))
        {
            selectShader.Use();
            glm::vec4 now(rabbitModel * glm::vec4(vertices[selectedPointIndice * 6], vertices[selectedPointIndice * 6 + 1], vertices[selectedPointIndice * 6 + 2], 1.f));
            model = glm::translate(glm::mat4(1.f), glm::vec3(now.x, now.y, now.z));
            model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
            projection = glm::perspective(glm::radians(fov), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 1000.0f);
            GLint modelLoc = glGetUniformLocation(selectShader.Program, "model");
            GLint viewLoc = glGetUniformLocation(selectShader.Program, "view");
            GLint projLoc = glGetUniformLocation(selectShader.Program, "projection");
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(LVAO);
            glDrawElements(GL_TRIANGLES, 3 * LTriNum, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        lightShader.Use();
        //Light Matrix
        view = glm::mat4(1.f);
        projection = glm::mat4(1.f);
        model = glm::mat4(1.f);
        model = glm::translate(model, lightPos);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        projection = glm::perspective(glm::radians(fov), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 1000.0f);
        modelLoc = glGetUniformLocation(lightShader.Program, "model");
        viewLoc = glGetUniformLocation(lightShader.Program, "view");
        projLoc = glGetUniformLocation(lightShader.Program, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //Light Color
        GLint lightColorLoc = glGetUniformLocation(lightShader.Program, "lightColor");
        glUniform3f(lightColorLoc, lightColor.x, lightColor.y, lightColor.z);
        glBindVertexArray(LVAO);
        glDrawElements(GL_TRIANGLES, 3 * LTriNum, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS)
    {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            cursorDisable = false;
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            cursorDisable = true;
            firstMouse = true;
        }
    }
    switch (key)
    {
    // = Light to white
    case GLFW_KEY_EQUAL:
        if (action != GLFW_PRESS)
            break;
        lightColor = glm::vec3(1.f);
        break;
    // - Change Light Color
    case GLFW_KEY_MINUS:
        if (action != GLFW_PRESS)
            break;
        lightColor.x = sin(glfwGetTime() * 2.0f);
        lightColor.y = sin(glfwGetTime() * 0.7f);
        lightColor.z = sin(glfwGetTime() * 1.3f);
        break;
    // C Change if attenuation
    case GLFW_KEY_C:
        if (action != GLFW_PRESS)
            break;
        isAttenuation = !isAttenuation;
        std::cout << "The attenuation is : ";
        if (isAttenuation)
            std::cout << "on" << std::endl;
        else
            std::cout << "off" << std::endl;
        std::cout << SPLIT << std::endl;
        break;
    default:
        break;
    }
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void do_movement()
{
    // Camera controls
    GLfloat cameraSpeed = 50.0f * deltaTime;
    GLfloat rabbitSpeed = 30.0f * deltaTime;
    if (keys[GLFW_KEY_W])
        cameraPos += cameraSpeed * cameraFront;
    if (keys[GLFW_KEY_S])
        cameraPos -= cameraSpeed * cameraFront;
    if (keys[GLFW_KEY_A])
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (keys[GLFW_KEY_D])
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (keys[GLFW_KEY_UP])
        cameraPos += cameraUp * cameraSpeed;
    if (keys[GLFW_KEY_DOWN])
        cameraPos -= cameraUp * cameraSpeed;
    // <- rabbit move left
    if (keys[GLFW_KEY_LEFT])
        rabbitModel = glm::translate(rabbitModel, glm::vec3(-rabbitSpeed, 0, 0));
    // -> rabbit move right
    if (keys[GLFW_KEY_RIGHT])
        rabbitModel = glm::translate(rabbitModel, glm::vec3(rabbitSpeed, 0, 0));
    // J rabbit rotate left
    if (keys[GLFW_KEY_J])
        rabbitModel = glm::rotate(rabbitModel, glm::radians(rabbitSpeed), glm::vec3(0.f, 0.f, 1.f));
    // L rabbit rotate right
    if (keys[GLFW_KEY_L])
        rabbitModel = glm::rotate(rabbitModel, glm::radians(-rabbitSpeed), glm::vec3(0.f, 0.f, 1.f));
    // I rabbit rotate straight
    if (keys[GLFW_KEY_I])
        rabbitModel = glm::rotate(rabbitModel, glm::radians(-rabbitSpeed), glm::vec3(1.f, 0.f, 0.f));
    // K rabbit rotate behind
    if (keys[GLFW_KEY_K])
        rabbitModel = glm::rotate(rabbitModel, glm::radians(rabbitSpeed), glm::vec3(1.f, 0.f, 0.f));
    // Z rabbit scale small
    if (keys[GLFW_KEY_Z])
        rabbitModel = glm::scale(rabbitModel, glm::vec3(1.0f - 0.001f * rabbitSpeed, 1.0f - 0.001f * rabbitSpeed, 1.0f - 0.001f * rabbitSpeed));
    // X rabbit scale large
    if (keys[GLFW_KEY_X])
        rabbitModel = glm::scale(rabbitModel, glm::vec3(1.0f + 0.001f * rabbitSpeed, 1.0f + 0.001f * rabbitSpeed, 1.0f + 0.001f * rabbitSpeed));
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    mouseX = xpos;
    mouseY = ypos;
    if (cursorDisable)
    {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        GLfloat xoffset = xpos - lastX;
        GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to left
        lastX = xpos;
        lastY = ypos;

        GLfloat sensitivity = 0.05; // Change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
}

void mouse_click_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (!cursorDisable && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_MOUSE_BUTTON_LEFT)
    {
        GLfloat xpos = mouseX;
        GLfloat ypos = mouseY;
        std::cout << "Screen Position : " << xpos << ' ' << ypos << std::endl;
        GLfloat minDistance = glm::pow(10, 20);
        GLuint minIndice = 0 - 1;
        GLfloat minX, minY;
        for (int i = 0; i < PointNum; i++)
        {
            if (glm::dot(glm::mat3(glm::transpose(glm::inverse(rabbitModel))) * vecNormal[i], cameraFront) < 0)
            {
                glm::vec4 iPos;
                glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
                glm::mat4 projection = glm::perspective(glm::radians(fov), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 1000.0f);
                iPos = rabbitModel * glm::vec4(vertices[i * 6 + 0], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
                iPos = projection * view * iPos;
                GLfloat pointPosX = WIDTH / 2 * (iPos.x / iPos.w) + WIDTH / 2;
                // Attention to the negative sign here
                GLfloat pointPosY = HEIGHT / 2 * (-iPos.y / iPos.w) + HEIGHT / 2;
                if ((pointPosX - xpos) * (pointPosX - xpos) + (pointPosY - ypos) * (pointPosY - ypos) < minDistance)
                {
                    minDistance = (pointPosX - xpos) * (pointPosX - xpos) + (pointPosY - ypos) * (pointPosY - ypos);
                    minIndice = i;
                    minX = pointPosX;
                    minY = pointPosY;
                }
            }
        }
        // distance <20
        if (minDistance < 400)
        {
            selectedPointIndice = minIndice;
            std::cout << "The point indice is : " << minIndice << std::endl;
            std::cout << "The point position is : " << vertices[minIndice * 6 + 0] << ' ' << vertices[minIndice * 6 + 1] << ' ' << vertices[minIndice * 6 + 2] << std::endl;
            std::cout << "The point screen position is : " << minX << ' ' << minY << std::endl;
        }
        else
        {
            std::cout << "No point nearby (The distance between the cursor and the nearest point is less than 20 pixels)" << std::endl;
        }
        std::cout << SPLIT << std::endl;
    }
}