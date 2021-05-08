#include "Application.h"
#include "Input.h"


glm::vec2 Input::getMousePosition() {
    double x;
    double y;

    glfwGetCursorPos(Application::getInstance()->getWindow(), &x, &y);

    return glm::vec2(x, y);
}

bool Input::mouseButtonPressed(int button) {
    return glfwGetMouseButton(Application::getInstance()->getWindow(), button) == GLFW_PRESS;
}