#include <glad/glad.h>
#include <GLFW/glfw3.h>

//memory leaks
#define CRTDBG_ON 0
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#undef CRTDBG_ON
#define CRTDBG_ON 1
#endif

#include "z_stages/game/game.h"
#include "z_stages/main_menu/main_menu.h"
#include "z_stages/stages.h"
#include "window/window.h"

//prototypes
GLFWwindow* init_window(const char* name, int width, int height);

int main()
{
    //_CrtSetBreakAlloc(1710975);

    GLFWwindow* window=NULL;
    int currentStage = STAGE_INITIALIZE;
    int shouldQuit = 0;
    while (shouldQuit==0)
    {
        switch (currentStage)
        {
        case STAGE_INITIALIZE:
            window_setWidth(1300);
            window_setHeight(800);
            window = init_window("Mesteri MC", window_getWidth(), window_getHeight());
            currentStage = STAGE_MAIN_MENU;
            break;

        case STAGE_MAIN_MENU:
            mainMenu(window, &currentStage);
            break;

        case STAGE_IN_GAME:
            game(window, &currentStage);
            break;

        default:
            shouldQuit = 69;
            break;
        }
    }

    glfwTerminate();

    if (CRTDBG_ON)
        _CrtDumpMemoryLeaks();
}

GLFWwindow* init_window(const char* name, int width, int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(width, height, name, NULL, NULL);
    if (window == NULL)//check if glfw is kaputt
    {
        printf("Failed to create GLFW window");
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    //check if glad is kaputt
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD");
        return NULL;
    }

    glViewport(0, 0, width, height);

    return window;
}