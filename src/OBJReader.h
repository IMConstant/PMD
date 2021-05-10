#ifndef MESHSIMPLIFICATION_OBJREADER_H
#define MESHSIMPLIFICATION_OBJREADER_H


#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <cstring>
#include <stdlib.h>

#include <glm/gtc/type_ptr.hpp>
#include <libgen.h>
#include <unordered_set>

#include "../common/string_func.h"



namespace mtl {

    struct Material {
        std::string name;
        uint texture;
    };


    struct MaterialInfo {
        uint begin_idx;
        uint end_idx;

        uint material;
    };

    struct Data {
        std::unordered_map<std::string, uint> materials_map;
        std::vector<std::string> textures;
        std::vector<Material> materials;
    };


    class MTLReader {
        enum class FieldType {
            NewMaterial,
            Texture,
            Comment,
            Unknown
        };

    public:
        static Data read(std::string const &fileName) {
            std::string file_name_copy = fileName;
            std::string file_dir_name = dirname(const_cast<char *>(file_name_copy.c_str()));

            Data materials_data;
            std::unordered_map<std::string, uint> textures_map;

            std::ifstream fin(fileName);

            if (fin.is_open()) {
                std::string line;

                std::string current_material;

                while (std::getline(fin, line)) {
                    FieldType fieldType = get_field_type(line);

                    switch (fieldType) {
                        case FieldType::NewMaterial: {
                            current_material = string_split(line, " \t\r", 7, false).at(0);
                            materials_data.materials.push_back(Material{current_material, 0});
                            materials_data.materials_map[current_material] = static_cast<unsigned int>(materials_data.materials.size() - 1);

                            break;
                        }
                        case FieldType::Texture: {
                            std::string texture = string_split(line, " \t\r", 7, false).at(0);

                            if (textures_map.find(texture) == textures_map.end()) {
                                materials_data.textures.push_back(file_dir_name + "/" + texture);
                                textures_map[texture] = static_cast<uint>(materials_data.textures.size() - 1);
                            }

                            materials_data.materials.at(materials_data.materials_map.at(current_material)).texture = textures_map.at(texture);

                            break;
                        }
                    }
                }

                fin.close();
            }

            return materials_data;
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

            if (type == "newmtl") return FieldType::NewMaterial;
            if (type == "map_Kd") return FieldType::Texture;
            else return FieldType::Unknown;
        }
    };


} // namespace mtl



class OBJReader {
private:

    enum class FieldType {
        Vertex,
        Normal,
        UV,
        Face,
        Comment,
        Material,
        UseMaterial,
        Unknown
    };

public:
    static struct ParseMaterialInfo {
        mtl::Data data;

        std::vector<mtl::MaterialInfo> info;
    } prevParseMaterialInfo;

public:

    class Layout {
        struct Field {
            int offset;
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
        std::string file_name_copy = fileName;
        std::string file_dir_name = dirname(const_cast<char *>(file_name_copy.c_str()));

        bool has_material = false;

        std::ifstream fin(fileName);

        if (fin.is_open()) {
            std::string line;

            std::vector<glm::vec2> tv;

            uint first_meterial_face_id = 0;
            uint next_face_id = 0;

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
                    case FieldType::UV: {
                        std::vector<std::string> str_list = string_split(line, " \t\r", 3, false);

                        float v1 = std::stof(str_list.at(0));
                        float v2 = std::stof(str_list.at(1));

                        tv.emplace_back(glm::vec2{v1, v2});

                        break;
                    }
                    case FieldType::Face: {
                        TFace face;

                        std::vector<std::string> str_list = string_split(line, " \t\r", 2, false);

                        for (unsigned int i = 0; i < 3; i++) {
                            std::vector<std::string> face_components = string_split(str_list.at(i), "/", 0);

                            auto value = static_cast<unsigned int>(std::stoul(face_components.at(0))) - 1;
                            memcpy(reinterpret_cast<unsigned char *>(&face) + i * sizeof(unsigned int), &value, sizeof(value));

                            if (face_components.at(1).length() != 0) {
                                uint uv_id = std::stoul(face_components.at(1)) - 1;
                                glm::vec2 uv = tv.at(uv_id);

                                memcpy(reinterpret_cast<unsigned char *>(vertices.data() + value) + layout.uv.offset + sizeof(unsigned long), &uv,
                                       sizeof(uv));

                                memcpy(reinterpret_cast<unsigned char *>(&face) + 3 * sizeof(uint) + i * sizeof(glm::vec2), &uv, sizeof(uv));
                            }
                        }

                        faces.push_back(face);
                        next_face_id++;

                        break;
                    }
                    case FieldType::Material: {
                        std::vector<std::string> str_list = string_split(line, " \t\r", 7, false);

                        OBJReader::prevParseMaterialInfo.data = mtl::MTLReader::read(file_dir_name + "/" + str_list.at(0));
                        OBJReader::prevParseMaterialInfo.info.clear();

                        has_material = true;

                        break;
                    }
                    case FieldType::UseMaterial: {
                        if (has_material) {
                            if (!OBJReader::prevParseMaterialInfo.info.empty()) {
                                OBJReader::prevParseMaterialInfo.info.back().end_idx = next_face_id - 1;
                            }

                            OBJReader::prevParseMaterialInfo.info.emplace_back();
                            OBJReader::prevParseMaterialInfo.info.back().begin_idx = next_face_id;
                            OBJReader::prevParseMaterialInfo.info.back().material = OBJReader::prevParseMaterialInfo.data.materials_map.at(string_split(line, " \t\r", 7, false).at(0));
                        }

                        break;
                    }
                }
            }

            if (has_material) {
                if (!OBJReader::prevParseMaterialInfo.info.empty()) {
                    OBJReader::prevParseMaterialInfo.info.back().end_idx = next_face_id - 1;
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
        else if (type == "mtllib") return FieldType::Material;
        else if (type == "usemtl") return FieldType::UseMaterial;
        else return FieldType::Unknown;
    }
};


#endif //MESHSIMPLIFICATION_OBJREADER_H
