#include <iostream>

#include "gui/qwallet.hpp"

// ------------------------------------------------------------------------------------------------
int main()
{
    // Initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    auto* window = glfwCreateWindow(1280, 720, "qwallet", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    // Initialize glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return 1;
    }

    // Create and initialize node
    QWallet qwallet{"QWallet"};
    if (!qwallet.Initialize(window))
    {
        std::cerr << "Failed to create the qwallet" << std::endl;
        glfwTerminate();
        return 1;
    }

    // Program loop
    auto lastFrameTime = glfwGetTime();
    while (!glfwWindowShouldClose(window) && !qwallet.Quit())
    {
        // Calculate delta time for current frame
        const auto currentFrameTime = glfwGetTime();
        const auto deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        glClearColor(40.0f / 256.0f, 42.0f / 256.0f, 54.0f / 256.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        qwallet.Update(window, deltaTime);
        qwallet.Render();

        // Finish glfw
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Destroy node and glfw
    qwallet.Destroy();
    glfwTerminate();
}
