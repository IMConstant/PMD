#include "Application.h"


void CameraUpdateInfo::rotate(Camera &camera) {
    glm::vec2 vector = endPosition - beginPosition;
    glm::vec2 windowSize = Application::getInstance()->getWindowSize();
    vector.x /= windowSize.x;
    vector.y /= windowSize.y;

    static float pi = static_cast<float>(std::acos(-1));

    glm::vec3 cameraVector = camera.position - camera.lookPosition;
    float r = glm::length(cameraVector);
    float phi = std::atan2(cameraVector.x, cameraVector.z);
    float theta = std::acos(cameraVector.y / r);
    float delta_phi = -1 * pi * vector.x;
    float delta_theta = -1 * pi * vector.y;
    float new_theta = theta + delta_theta;

    if (theta + delta_theta < 0.00001f) {
        new_theta = 0.00001f;
    }
    else if (theta + delta_theta >= pi) {
        new_theta = pi - 0.00001f;
    }

    glm::vec3 newCameraVector = glm::vec3(r * std::sin(new_theta) * std::sin(phi + delta_phi),
                                          r * std::cos(new_theta),
                                          r * std::sin(new_theta) * std::cos(phi + delta_phi));

    camera.position = camera.lookPosition + newCameraVector;
}

void CameraUpdateInfo::translate(Camera &camera) {
    glm::vec2 mouseVector = endPosition - beginPosition;
    glm::vec2 windowSize = Application::getInstance()->getWindowSize();
    mouseVector /= glm::vec2{ windowSize.x, windowSize.y };

    glm::mat3 inverseViewMatrix = glm::inverse(glm::lookAtLH(camera.position, camera.lookPosition, { 0, 1, 0 }));

    glm::vec3 moveVector(mouseVector * glm::length(camera.position - camera.lookPosition), 0.0f);
    moveVector = inverseViewMatrix * moveVector;

    camera.position += moveVector;
    camera.lookPosition += moveVector;
}


Application *Application::s_instance = nullptr;

Application::Application() {
    if (s_instance) {
        std::cerr << "Application is already created!" << std::endl;
        throw;
    }

    glfwSetErrorCallback([](int error, const char *description) {
        fprintf(stderr, "glfw error %d: %s\n", error, description);
    });

    glfwInit();

    s_instance = this;
    m_window = glfwCreateWindow(800, 600, "Applcation", nullptr, nullptr);

    glfwMakeContextCurrent(m_window);
    setCallBacks();

    glewExperimental = GL_TRUE;
    glewInit();
    glfwSwapInterval(1);
    glfwWindowHint(GLFW_SAMPLES, 4);

    mMainCamera.position = {0.f, 10.0f, -10.0f};
    mMainCamera.lookPosition = {0.f, 0.f, 0.f};

    imguiInit();
}

void Application::setCallBacks() {
    glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int button, int action, int mods) {
        auto *app = Application::getInstance();

        if (action == GLFW_PRESS) {
            app->mousePressEvent(button);
        }
        else if (action == GLFW_RELEASE) {
            app->mouseReleaseEvent(button);
        }
    });

    glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double x, double y) {
        Application::getInstance()->mouseMoveEvent(glm::vec2{x, y});
    });

    glfwSetScrollCallback(m_window, [](GLFWwindow *window, double x, double y) {
        Application::getInstance()->mouseWheelEvent(y);
    });
}

Application::~Application() {

}

Application *Application::getInstance() {
    return s_instance;
}

void Application::imguiInit() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init();

    ImFontConfig font_config;
    font_config.OversampleH = 1;
    font_config.OversampleV = 1;
    font_config.PixelSnapH = true;
}

void Application::run() {
    Shader shader(Shader::fromSourceFiles(
        "./shaders/vertex.glsl",
        "./shaders/fragment.glsl"
    ));

    shader.bind();

    Mesh<VertexComponentsColored> my_mesh;
    my_mesh.load_from_file("./hall.obj");
    my_mesh.calculate_normals();

    my_mesh.simplify(0.2f);

    my_mesh.calculate_normals();

    float mesh_area = my_mesh.area();
    float mesh_volume = my_mesh.volume();

    Surface_mesh mesh;
    mesh.reserve(2000, 2000, 2000);

    std::ifstream fin("./robot.obj");

    std::vector<Point_3> obj_vertices;
    std::vector<std::vector<std::size_t>> obj_faces;

    CGAL::read_OBJ(fin, obj_vertices, obj_faces);

    CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(obj_vertices, obj_faces, mesh);

    CGAL::Polygon_mesh_processing::triangulate_faces(mesh);

    Simplify::vertices.reserve(mesh.num_vertices());
    Simplify::triangles.reserve(mesh.num_faces());

    for (auto vertex : mesh.points()) {
        Simplify::Vertex v;
        v.p = { vertex.x(), vertex.y(), -vertex.z() };
        Simplify::vertices.push_back(v);
    }

    for (auto face_descriptor : mesh.faces()) {
        std::vector<uint32_t> vertex_face_indices;

        for (auto vertex_descriptor : CGAL::vertices_around_face(mesh.halfedge(face_descriptor), mesh)) {
            vertex_face_indices.push_back(static_cast<uint>(vertex_descriptor));
        }

        Simplify::Triangle t;

        t.v[0] = vertex_face_indices.at(0);
        t.v[1] = vertex_face_indices.at(1);
        t.v[2] = vertex_face_indices.at(2);

        Simplify::triangles.push_back(t);
    }

    //Simplify::simplify_mesh<>(nullptr, 3000, 7);


    std::vector<glm::vec3> vertices;

    for (auto vertex : Simplify::vertices) {
        vertices.push_back(vertex.p);
    }

    std::vector<glm::vec3> normals(vertices.size(), glm::vec3(0.0f, 0.0f, 0.0f));

    for (auto face : Simplify::triangles) {
        glm::vec3 A = vertices.at(face.v[1]) - vertices.at(face.v[0]);
        glm::vec3 B = vertices.at(face.v[2]) - vertices.at(face.v[0]);

        glm::vec3 N = glm::normalize(glm::cross(A, B));

        normals.at(face.v[0]) += N;
        normals.at(face.v[1]) += N;
        normals.at(face.v[2]) += N;
    }

    for (auto &normal : normals) {
        normal = glm::normalize(normal);
    }

    std::vector<uint32_t> indices;
    indices.reserve(3 * Simplify::triangles.size());

    for (auto triangle : Simplify::triangles) {
        indices.push_back(triangle.v[0]);
        indices.push_back(triangle.v[1]);
        indices.push_back(triangle.v[2]);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    glm::vec3 clearColor = {0.1f, 0.1f, 0.1f};

    GLuint vertexBufferDesc{0}, normalBufferDesc, indexBufferDesc, colorBufferDesc;

    GLuint position_attribute = shader.attributeLocation("position");
    GLuint normal_attribute = shader.attributeLocation("normal");
    GLuint color_attribute = shader.attributeLocation("color");


    uint VTABLE_OFFSET = sizeof(unsigned long);


    glGenBuffers(1, &vertexBufferDesc);
    glEnableVertexAttribArray(position_attribute);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferDesc);
    glBufferData(GL_ARRAY_BUFFER, my_mesh.vertices().size() * sizeof(decltype(my_mesh)::VertexType),
                 reinterpret_cast<unsigned char const *>(my_mesh.vertices().data()) + VTABLE_OFFSET, GL_STATIC_DRAW);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(my_mesh)::VertexType), nullptr);

    glGenBuffers(1, &normalBufferDesc);
    glEnableVertexAttribArray(normal_attribute);

    glBindBuffer(GL_ARRAY_BUFFER, normalBufferDesc);
    glBufferData(GL_ARRAY_BUFFER, my_mesh.vertices().size() * sizeof(decltype(my_mesh)::VertexType),
                 reinterpret_cast<unsigned char const *>(my_mesh.vertices().data()) + VTABLE_OFFSET + sizeof(glm::vec3), GL_STATIC_DRAW);
    glVertexAttribPointer(normal_attribute, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(my_mesh)::VertexType), nullptr);

    glGenBuffers(1, &colorBufferDesc);
    glEnableVertexAttribArray(color_attribute);

    glBindBuffer(GL_ARRAY_BUFFER, colorBufferDesc);
    glBufferData(GL_ARRAY_BUFFER, my_mesh.vertices().size() * sizeof(decltype(my_mesh)::VertexType),
                 reinterpret_cast<unsigned char const *>(my_mesh.vertices().data()) + VTABLE_OFFSET + 2 * sizeof(glm::vec3), GL_STATIC_DRAW);
    glVertexAttribPointer(color_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(decltype(my_mesh)::VertexType), nullptr);

    float angle = 0.0f;

    while (!glfwWindowShouldClose(m_window)) {
        glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("MainWindow");
            ImGui::ColorEdit3("clear color", reinterpret_cast<float *>(&clearColor));
            ImGui::SliderFloat3("camera position", reinterpret_cast<float *>(&mMainCamera.position), -20.0f, 20.0f);
            ImGui::Text("Vertices: %u", my_mesh.vertices().size());
            ImGui::Text("Faces: %u", my_mesh.faces().size());
            ImGui::Text("Area: %f", mesh_area);
            ImGui::Text("Volume: %f", mesh_volume);

            ImGui::End();

            if (ImGui::BeginMainMenuBar()) {
                //ImGui::BeginMenu("File");
                //ImGui::EndMenu();
                float framerate = ImGui::GetIO().Framerate;
                float max_framerate = 60.0f;
                float delta = glm::min(1.0f, framerate / max_framerate);

                ImGui::Text("fps:");
                ImGui::TextColored(ImVec4(1.0f - delta, delta, 0.0f, 1.0f), "%4.2f", framerate);
                ImGui::EndMainMenuBar();
            }

            ImGui::Render();
        }

        int width{0};
        int height{0};

        glfwGetFramebufferSize(m_window, &width, &height);

        glViewport(0, 0, width, height);

        angle += 0.01f;

        glm::mat4 model = glm::mat4(1.0f);
        //model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 view = glm::lookAt(mMainCamera.position, mMainCamera.lookPosition, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), width / static_cast<float>(height), 0.1f, 100.0f);

        glm::mat4 model_view_projection = projection * view * model;

        glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "u_model_view_projection"), 1, GL_FALSE, glm::value_ptr(model_view_projection));

        glDrawElements(GL_TRIANGLES, my_mesh.faces().size() * 3, GL_UNSIGNED_INT, my_mesh.faces().data());

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }

    glDisableVertexAttribArray(position_attribute);
    glDisableVertexAttribArray(normal_attribute);
    glDisableVertexAttribArray(color_attribute);
    glDeleteBuffers(1, &vertexBufferDesc);
    glDeleteBuffers(1, &normalBufferDesc);
    glDeleteBuffers(1, &colorBufferDesc);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Application::close() {

}
