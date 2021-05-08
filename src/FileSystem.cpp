#include "FileSystem.h"


namespace fs {

    std::string getCurrentDirectory() {
        char cwd[128] = {0};
        getcwd(cwd, 128);

        return std::string(cwd);
    }


    FilesList parseDirectory(std::string const &path, int maxFiles) {
        FilesList files;

        dirent *dp;
        DIR *dir = opendir(path.c_str());

        dp = readdir(dir);

        while (dp) {
            std::pair<FileType, std::string> file(static_cast<FileType>(dp->d_type), std::string(dp->d_name));

            if (file.first == FileType::Directory ||
                file.second.substr(file.second.length() - 4, file.second.length() - 1) == ".obj") {
                files.push_back(file);
            }

            dp = readdir(dir);
        }

        closedir(dir);

        std::sort(files.begin(), files.end(), [](File const &a, File const &b) -> bool {
            if (a.second == "." || a.second == "..") return true;
            if (b.second == "." || b.second == "..") return false;

            if (a.first == FileType::Directory && b.first == FileType::Regular) {
                return true;
            } else if (b.first == FileType::Directory && a.first == FileType::Regular) {
                return false;
            } else {
                return a.second < b.second;
            }
        });

        return files;
    }

} // namespace fs