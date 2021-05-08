#include "Shader.h"


static std::string default_vertex_shader_source;
static std::string default_fragment_shader_source;



Shader::Shader(const std::string &vertex_source, const std::string &fragment_source) {
    int vrez{GL_FALSE}, frez{GL_FALSE};

    int vid = glCreateShader(GL_VERTEX_SHADER);
    const char *vsource = vertex_source.data();
    glShaderSource(vid, 1, &vsource, nullptr);
    glCompileShader(vid);
    glGetShaderiv(vid, GL_COMPILE_STATUS, &vrez);

    printf("%s\n", glGetString(GL_VERSION));

    if (vrez == GL_FALSE)
        std::cerr << "Vertex shader compilation error" << std::endl;

    int fid = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fsource = fragment_source.data();
    glShaderSource(fid, 1, &fsource, nullptr);
    glCompileShader(fid);
    glGetShaderiv(fid, GL_COMPILE_STATUS, &frez);

    if (frez == GL_FALSE)
        std::cerr << "Fragment shader compilation error" << std::endl;

    if (vrez == GL_TRUE && frez == GL_TRUE) {
        m_shader_program = glCreateProgram();

        glAttachShader(m_shader_program, fid);
        glAttachShader(m_shader_program, vid);

        glLinkProgram(m_shader_program);

        int link_rez{GL_FALSE};
        glGetProgramiv(m_shader_program, GL_LINK_STATUS, &link_rez);

        if (link_rez == GL_FALSE)
            std::cerr << "Shader linking error" << std::endl;
    }
}

Shader::Shader(std::pair<std::string, std::string> sources) : Shader(sources.first, sources.second) {}

std::pair<std::string, std::string> Shader::fromSourceFiles(const std::string &vertex_source_file, const std::string &fragment_source_file) {
    std::ifstream vfin(vertex_source_file);
    std::ifstream ffin(fragment_source_file);

    bool filesOpened = vfin.is_open() && ffin.is_open();

    if (!vfin.is_open())
        std::cerr << "Can not open vertex shader source file" << std::endl;
    if (!ffin.is_open())
        std::cerr << "Can not open fragment shader source file" << std::endl;

    if (filesOpened) {
        std::pair<std::string, std::string> sources;

        vfin.seekg(0, std::ios::end);
        ffin.seekg(0, std::ios::end);

        sources.first.reserve(vfin.tellg());
        sources.second.reserve(ffin.tellg());

        vfin.seekg(0, std::ios::beg);
        ffin.seekg(0, std::ios::beg);

        sources.first.assign(std::istreambuf_iterator<char>(vfin), std::istreambuf_iterator<char>());
        sources.second.assign(std::istreambuf_iterator<char>(ffin), std::istreambuf_iterator<char>());

        return sources;
    }
    else {
        std::cerr << "Default shaders will be used instead" << std::endl;

        return std::pair<std::string, std::string>(default_vertex_shader_source, default_fragment_shader_source);
    }
}

int Shader::getId() const {
    return m_shader_program;
}

void Shader::bind() {
    glUseProgram(m_shader_program);
}
