#pragma once

#include <GL/glew.h>

#include <iostream>
#include <fstream>


class Shader {
    int m_shader_program;

public:
    Shader(const std::string &vertex_source, const std::string &fragment_source);
    Shader(std::pair<std::string, std::string> sources);


    GLuint attributeLocation(const std::string &attribute) {
        return static_cast<GLuint>(glGetAttribLocation(m_shader_program, attribute.c_str()));
    }

    GLuint uniformLocation(const std::string &uniform) {
        return static_cast<GLuint>(glGetUniformLocation(m_shader_program, uniform.c_str()));
    }

    int getId() const;
    void bind();

    static std::pair<std::string, std::string> fromSourceFiles(const std::string &vertex_source_file, const std::string &fragment_source_file);
};
