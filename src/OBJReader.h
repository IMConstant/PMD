#ifndef MESHSIMPLIFICATION_OBJREADER_H
#define MESHSIMPLIFICATION_OBJREADER_H


#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>

#include <glm/gtc/type_ptr.hpp>


class OBJReader {
private:

    enum class FieldType {
        Vertex,
        Normal,
        UV,
        Face,
        Comment,
        Unknown
    };

public:

    class Layout {
        struct Field {
            unsigned int offset;
            unsigned int size;
        };

    public:
        Field position;
        Field uv;
        Field color;
        Field normal;

        Layout() {
            position.offset = -1;
            uv.offset = -1;
            color.offset = -1;
            normal.offset = -1;
        }
    };

public:
    template <typename TVertex, typename TFace>
    static void read(std::string const &fileName, std::vector<TVertex> &vertices, std::vector<TFace> &faces, Layout const &layout) {
        std::ifstream fin(fileName);

        if (fin.is_open()) {
            std::string line;

            while (std::getline(fin, line)) {
                FieldType fieldType = get_field_type(line);

                switch (fieldType) {
                    case FieldType::Vertex: {
                        TVertex vertex;

                        glm::vec4 default_color{1.f, 1.f, 1.f, 1.f};
                        memcpy(reinterpret_cast<unsigned char *>(&vertex) + layout.color.offset + sizeof(unsigned long), glm::value_ptr(default_color), sizeof(default_color));

                        std::vector<std::string> str_list = string_split(line, " \t\r", 2, false);

                        for (unsigned int i = 0; i < str_list.size(); i++) {
                            if (i < 3) { // vertex position
                                float value = std::stof(str_list.at(i));
                                memcpy(reinterpret_cast<unsigned char *>(&vertex) + layout.position.offset + i * sizeof(float) + sizeof(unsigned long), &value,
                                       sizeof(value));
                            } else if (layout.color.offset != -1) {
                                float value = std::stof(str_list.at(i));
                                memcpy(reinterpret_cast<unsigned char *>(&vertex) + layout.color.offset + (i - 3) * sizeof(float) + sizeof(unsigned long), &value,
                                       sizeof(value));
                            }
                        }

                        vertices.push_back(vertex);

                        break;
                    }
                    case FieldType::Face: {
                        TFace face;

                        std::vector<std::string> str_list = string_split(line, " \t\r", 2, false);

                        for (unsigned int i = 0; i < 3; i++) {
                            std::vector<std::string> face_components = string_split(str_list.at(i), "/", 0);

                            auto value = static_cast<unsigned int>(std::stoul(face_components.at(0))) - 1;
                            memcpy(reinterpret_cast<unsigned char *>(&face) + i * sizeof(unsigned int), &value, sizeof(value));
                        }

                        faces.push_back(face);

                        break;
                    }
                }
            }
        }
    }

private:
    static FieldType get_field_type(std::string const &line) {
        bool is_comment = line.find('#') != std::string::npos;

        if (is_comment) {
            return FieldType::Comment;
        }

        std::string type;
        unsigned long space_index = line.find(' ');
        type = line.substr(0, space_index);

        if (type == "v") return FieldType::Vertex;
        else if (type == "vt") return FieldType::UV;
        else if (type == "vn") return FieldType::Normal;
        else if (type == "f")  return FieldType::Face;
        else return FieldType::Unknown;
    }

public:
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
};


#endif //MESHSIMPLIFICATION_OBJREADER_H
