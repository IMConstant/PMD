#ifndef MESHSIMPLIFICATION_STRING_FUNC_H
#define MESHSIMPLIFICATION_STRING_FUNC_H

#include <string>
#include <vector>


static unsigned long string_find(std::string const &str, std::string const &dlm, int start_pos = 0) {
    for (auto iter = dlm.begin(); iter != dlm.end(); iter++) {
        unsigned long index = str.find(*iter, start_pos);

        if (index != std::string::npos) {
            return index;
        }
    }

    return std::string::npos;
}

static std::vector<std::string> string_split(std::string const &str, std::string const &dlm, int start_pos, bool save_empty = true) {
    std::vector<std::string> v;

    int last_dlm_index = start_pos - 1;
    int next_dlm_index = -1;


    while ((next_dlm_index = string_find(str, dlm, last_dlm_index + 1)) != std::string::npos) {
        std::string item = str.substr(last_dlm_index + 1, next_dlm_index - last_dlm_index - 1);

        if (save_empty || item.length() != 0) {
            v.push_back(item);
        }

        last_dlm_index = next_dlm_index;
    }

    if (last_dlm_index < str.size() - 1) {
        v.push_back(str.substr(last_dlm_index + 1, str.size()));
    }

    return v;
}


#endif //MESHSIMPLIFICATION_STRING_FUNC_H
