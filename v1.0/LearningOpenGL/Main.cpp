#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_master/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_master/stb_image_write.h>

#include <iostream>
#include <string>

#include "Camera.h"
#include "Shader.h"

#include "FluidSystem.h"

// Properties
int screenWidth = 800, screenHeight = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// The MAIN function, from here we start our application and run our Game loop
int main()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", nullptr, nullptr); // Windowed
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Define the viewport dimensions
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);

    // Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);

    // Setup and compile our shaders
    Shader ourShader("../Shader/transform_sample.vs", "../Shader/transform_sample.frag");

    // Set up our vertex data (and buffer(s)) and attribute pointers
    GLfloat vertices[] = {
        -1,-1,-1,
        -1,-1,1,
        1,-1,1,
        1,-1,-1,
        -1,-1,-1,
        -1,1,-1,
        -1,1,1,
        -1,-1,1,
        -1,1,1,
        1,1,1,
        1,-1,1,
        1,1,1,
        1,1,-1,
        1,-1,-1,
        1,1,-1,
        -1,1,-1,
        -1,-1,-1
    };
    GLuint VAO1,VAO2,VBO1,VBO2;
    glGenVertexArrays(1, &VAO1);
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO1);
    glGenBuffers(1, &VBO2);

    glBindVertexArray(VAO1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    


    FluidSystem fluid;

    fluid.init( 15000, 
                glm::vec3(-20, -20, -20), glm::vec3(20, 20, 20), 
                glm::vec3(-15, -10, -15), glm::vec3(5, 20, 5), 
                glm::vec3(0, -10, 0));
    //fluid.init( 15000,
    //            glm::vec3(-20, -20, -20), glm::vec3(20, 20, 20),
    //            glm::vec3(0, 0, 0), glm::vec3(0, 0, 0),
    //            glm::vec3(0, -10, 0));
    fluid.setTick(8);
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        //fluid.addPoint(glm::vec3(-20, -4, -4), glm::vec3(-18, 0, 0), glm::vec3(1.0, 0, 0));
        fluid.tick();

        glBindVertexArray(VAO2);
        glBindBuffer(GL_ARRAY_BUFFER, VBO2);
        GLfloat *save = fluid.getPosData();
        unsigned int number = 3 * fluid.getPointCounts();
        for(int i=0;i<number;i++)
            save[i] /= 20;
        //for (int i = 0; i < number; i += 3)
        //    std::cout << save[i] << " " << save[i + 1] << " " << save[i + 2] << std::endl;
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* number, save, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check and call events
        glfwPollEvents();
        Do_Movement();

        // Clear the colorbuffer
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw our first triangle
        ourShader.Use();



        // Create camera transformation
        glm::mat4 view;
        view = camera.GetViewMatrix();
        glm::mat4 projection;
        projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
        // Get the uniform locations
        GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
        // Pass the matrices to the shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 model1,model;
        model1 = glm::scale(model, glm::vec3(1.05, 1.05, 1.05));
        //GLfloat angle = 2.0f * i*glfwGetTime();
        //model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));

        glBindVertexArray(VAO1);
        glDrawArrays(GL_LINE_LOOP, 0, 17);
        glBindVertexArray(0);
        
        model = glm::translate(model, glm::vec3(0, 0, 0));
        //GLfloat angle = 2.0f * i*glfwGetTime();
        //model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO2);
        glDrawArrays(GL_POINTS, 0, fluid.getPointCounts());
        glBindVertexArray(0);
        // Swap the buffers
        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO1);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO1);
    glDeleteBuffers(1, &VBO2);
    glfwTerminate();
    return 0;
}

// Moves/alters the camera positions based on user input
void Do_Movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    //cout << key << endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}