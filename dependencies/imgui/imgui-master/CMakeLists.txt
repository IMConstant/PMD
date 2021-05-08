SET(IMGUI_SOURCES imgui.cpp
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_widgets.cpp
        imgui_impl_glfw.cpp
        imgui_impl_opengl3.cpp
        )

SET(IMGUI_HEADERS imconfig.h
        imgui.h
        imgui_internal.h
        imstb_rectpack.h
        imstb_textedit.h
        imstb_truetype.h
        imgui_impl_glfw.h
        imgui_impl_opengl3.h
        )

add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS})

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/dependencies/glew/include)
target_link_libraries(imgui PUBLIC glfw glew_s)