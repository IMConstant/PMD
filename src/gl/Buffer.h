#ifndef MESHSIMPLIFICATION_BUFFER_H
#define MESHSIMPLIFICATION_BUFFER_H

#include <GL/glew.h>


namespace gl {

    using uint = unsigned int;

    class Buffer {
    public:
        static uint create(void const *data, uint size, uint offset);
        static void free(uint desc);
    };

} // namespace gl


#endif //MESHSIMPLIFICATION_BUFFER_H
