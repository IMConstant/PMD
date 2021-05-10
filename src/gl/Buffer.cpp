#include "Buffer.h"


namespace gl {

    uint Buffer::create(void const *data, uint size, uint offset) {
        uint desc{0};

        glGenBuffers(1, &desc);
        glBindBuffer(GL_ARRAY_BUFFER, desc);
        glBufferData(GL_ARRAY_BUFFER, size, data + offset, GL_STATIC_DRAW);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);

        return desc;
    }

    void Buffer::free(uint desc) {
        glDeleteBuffers(1, &desc);
    }
} // namespace gl