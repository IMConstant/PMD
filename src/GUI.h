#ifndef MESHSIMPLIFICATION_GUI_H
#define MESHSIMPLIFICATION_GUI_H

#include "FileSystem.h"


namespace GUI {

    extern bool showFileSystemWindow;

    struct ParseData {
        friend void createFileSystemWindow();
    private:
        static std::string lastDirectory;
    };


    void createFileSystemWindow();

} // namespace GUI


#endif //MESHSIMPLIFICATION_GUI_H
