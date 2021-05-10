#ifndef MESHSIMPLIFICATION_APPLICATION_H
#define MESHSIMPLIFICATION_APPLICATION_H

#include <GL/glew.h>

#include <chrono>
#include <fstream>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "RenderMesh.h"
#include "Shader.h"
#include "Simplify.h"

#include "Input.h"


int main(int argc, char **argv);


struct Camera {
    glm::vec3 position;
    glm::vec3 lookPosition;
};


struct CameraUpdateInfo {
    glm::vec2 beginPosition;
    glm::vec2 endPosition;

    void rotate(Camera &camera);
    void translate(Camera &camera);
};


class Application {
    friend int ::main(int argc, char **argv);

    static Application *s_instance;

    GLFWwindow *m_window;

    CameraUpdateInfo cameraUpdateInfo;
    Camera mMainCamera;

    RenderMesh<VertexComponentsColored> m_mesh;
    bool m_mesh_need_reload;
    std::string m_next_mesh_load_file;

public:
    Application();
    virtual ~Application();

    static Application *getInstance();

    GLFWwindow *getWindow() {
        return m_window;
    }

    glm::vec2 getWindowSize() const {
        int x;
        int y;

        glfwGetWindowSize(m_window, &x, &y);

        return glm::vec2(x, y);
    }

    void imguiInit();

    void reloadMesh(std::string const &fileName) {
        m_mesh_need_reload = true;
        m_next_mesh_load_file = fileName;
    }

protected:
    void mousePressEvent(int button) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            cameraUpdateInfo.beginPosition = Input::getMousePosition();
        }
    }

    void mouseReleaseEvent(int button) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {

        }
    }

    void mouseMoveEvent(glm::vec2 pos) {
        cameraUpdateInfo.endPosition = pos;

        if (!ImGui::IsAnyWindowFocused()) {
            if (Input::mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                cameraUpdateInfo.rotate(mMainCamera);
            } else if (Input::mouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
                cameraUpdateInfo.translate(mMainCamera);
            }
        }

        cameraUpdateInfo.beginPosition = cameraUpdateInfo.endPosition;
    }

    void mouseWheelEvent(double delta) {
        glm::vec3 cameraVector = mMainCamera.position - mMainCamera.lookPosition;
        float zoomSpeed = 0.1f;// *glm::length(cameraVector);

        glm::vec3 offsetVector = static_cast<float>(1.0f + zoomSpeed * delta) * cameraVector;
        glm::vec3 newCameraPosition = mMainCamera.lookPosition + offsetVector;

        if (glm::length(offsetVector) >= 0.75f) {
            mMainCamera.position = newCameraPosition;
        }
    }

private:
    void setCallBacks();

    void run();
    void close();
};


#endif //MESHSIMPLIFICATION_APPLICATION_H
