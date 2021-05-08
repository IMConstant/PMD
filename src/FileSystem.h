#ifndef MESHSIMPLIFICATION_FILESYSTEM_H
#define MESHSIMPLIFICATION_FILESYSTEM_H

#include <unistd.h>
#include <dirent.h>

#include <string>
#include <vector>
#include <algorithm>

#include "../common/string_func.h"


namespace fs {

    enum class FileType {
        Regular = DT_REG,
        Directory = DT_DIR
    };

    using FilesList = std::vector<std::pair<FileType, std::string>>;
    using File = std::pair<FileType, std::string>;


    std::string getCurrentDirectory();
    FilesList   parseDirectory(std::string const &path, int maxFiles = -1);


} // filesystem



#endif //MESHSIMPLIFICATION_FILESYSTEM_H
