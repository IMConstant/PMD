#ifndef MESHSIMPLIFICATION_INPUT_H
#define MESHSIMPLIFICATION_INPUT_H

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>


class Input {
public:
    static glm::vec2 getMousePosition();
    static bool mouseButtonPressed(int button);
};


#endif //MESHSIMPLIFICATION_INPUT_H
