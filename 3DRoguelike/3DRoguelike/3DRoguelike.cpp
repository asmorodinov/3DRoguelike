#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Game/Assert.h"
#include "Game/Shader.h"
#include "Game/Camera.h"
#include "Game/Assets.h"
#include "Game/Renderer.h"
#include "Game/UI/RenderText.h"

#include "Game/Dungeon/Dungeon.h"
#include "Game/Physics/Entity.h"

#include <iostream>
#include <string>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// player
Entity player;

// timing
float deltaTime = 0.0f;  // time between current frame and last frame
float lastFrame = 0.0f;

float fps = 0.0f;
float fpsLastFrame = 0.0f;

bool generate = false;
auto rightPressed = false;
auto leftPressed = false;

auto F1Pressed = false;
auto spacePressed = false;

SeedType seed = 0;

auto disableCollision = false;

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    auto fullscreen = Assets::GetConfigParameter<bool>("full-screen");
    auto monitor = fullscreen ? glfwGetPrimaryMonitor() : NULL;

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", monitor, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Assets::Get().orthogonalProjection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));

    auto dimensions = Dimensions{60, 30, 60};
    auto dungeon = Dungeon(dimensions);

    if (Assets::HasConfigParameter("seed")) {
        seed = Assets::GetConfigParameter<SeedType>("seed");
    } else {
        seed = RNG(SeedType()).RandomSeed();
    }

    dungeon.SetSeed(seed);
    dungeon.Generate();
    player.SetFlying(true);
    player.SetPosition(dungeon.GetSpawnPoint().AsVec3());
    camera.Position = player.GetPosition() + glm::vec3(0.0f, 0.15f, 0.0f);

    size_t frame = 0;

    auto textRenderer = TextRenderer();
    textRenderer.LoadFont("font.ttf");

    while (!glfwWindowShouldClose(window)) {
        ++frame;
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (frame % 120 == 0) {
            auto delta = currentFrame - fpsLastFrame;
            fps = 120.0f / delta;
            fpsLastFrame = currentFrame;
        }

        processInput(window);

        player.Update(dungeon.GetTiles(), deltaTime, disableCollision);
        camera.Position = player.GetPosition() + glm::vec3(0.0f, 0.15f, 0.0f);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        Assets::Get().projection = projection;
        Assets::Get().view = view;

        if (generate) {
            generate = false;
            dungeon.SetSeed(seed);
            dungeon.Generate();
            player.SetPosition(dungeon.GetSpawnPoint().AsVec3());
            camera.Position = player.GetPosition();
        }

        dungeon.Render();

        // render text
        auto fpsstr = std::to_string(static_cast<int>(glm::round(fps)));

        auto pos = player.GetPosition();
        auto ipos = glm::ivec3(glm::round(pos));
        auto posx = std::to_string(ipos.x);
        auto posy = std::to_string(ipos.y);
        auto posz = std::to_string(ipos.z);

        auto vel = camera.Velocity;
        auto ivel = glm::ivec3(glm::round(vel));
        auto velx = std::to_string(ivel.x);
        auto vely = std::to_string(ivel.y);
        auto velz = std::to_string(ivel.z);

        auto dc = std::to_string(disableCollision);
        auto fl = std::to_string(player.IsFlying());
        auto gr = std::to_string(player.IsGrounded());
        auto room = std::to_string(dungeon.WhichRoomPointIsInside(Coordinates::FromVec3(pos)));

        glDisable(GL_DEPTH_TEST);
        textRenderer.RenderText("fps: " + fpsstr + " pos: " + posx + " " + posy + " " + posz + " vel: " + velx + " " + vely + " " + velz +
                                    " dc: " + dc + " fl: " + fl + " gr: " + gr + " room: " + room,
                                glm::vec2(25.0f, 25.0f), 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    if (!rightPressed && glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        ++seed;
        generate = true;
        rightPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE) {
        rightPressed = false;
    }

    if (!leftPressed && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        --seed;
        generate = true;
        leftPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE) {
        leftPressed = false;
    }

    disableCollision = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;

    auto forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    auto backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    auto left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    auto right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    camera.ProcessKeyboard(forward, backward, left, right);

    player.GetAcceleration().x = camera.Velocity.x;
    player.GetAcceleration().z = camera.Velocity.z;

    auto jump = false;
    auto up = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    if (up && !spacePressed) {
        jump = true;
    }
    spacePressed = up;

    auto down = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    if (!F1Pressed && glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
        F1Pressed = true;
        player.SetFlying(!player.IsFlying());
    }
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_RELEASE) {
        F1Pressed = false;
    }

    if (!player.IsFlying() && jump) {
        player.Jump();
    }
    if (player.IsFlying()) {
        if (up && !down) {
            player.GetAcceleration().y = SPEED;
        }
        if (down && !up) {
            player.GetAcceleration().y = -SPEED;
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;  // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}