//
// Created by koko on 09.05.21.
#ifndef MESHSIMPLIFICATION_TEXTURE_H
#define MESHSIMPLIFICATION_TEXTURE_H

#include <string>
#include <stb_image.h>

#include <GL/glew.h>

namespace gl {

    using uint = unsigned int;

    class Texture {
    public:
        static uint createFromFile(std::string const &fileName) {
            stbi_set_flip_vertically_on_load(1);

            int width;
            int height;
            int BPP;

            unsigned char *buffer = stbi_load(fileName.c_str(), &width, &height, &BPP, 4);

            uint desc{0};

            glGenTextures(1, &desc);
            glBindTexture(GL_TEXTURE_2D, desc);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
            glBindTexture(GL_TEXTURE_2D, 0);

            stbi_image_free(buffer);

            return desc;
        }
    };

}// namespace gl


#endif //MESHSIMPLIFICATION_TEXTURE_H
