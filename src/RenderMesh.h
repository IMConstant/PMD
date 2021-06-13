#ifndef MESHSIMPLIFICATION_RENDERMESH_H
#define MESHSIMPLIFICATION_RENDERMESH_H

#include <chrono>
#include "Mesh.h"
#include "./gl/Buffer.h"
#include "./gl/Texture.h"
#include "Shader.h"


template <typename TVertexComponents>
class RenderMesh : public Mesh<TVertexComponents> {
    struct VertexBuffer {
        uint position_desc;
        uint normal_desc;
        uint color_desc;
        uint uv_desc;
    } vertexBuffer;

    struct RenderGroup {
        uint face_id0;
        uint face_id1;
        uint material_id;
    };

    struct Material {
        uint texture_id;
    };

    struct FaceNativeData {
        uint face_id;
        uchar vertex_num;
        uint vertex_id;
    };

    uint m_vertex_count;

    std::vector<uint> m_textures;

    std::vector<RenderGroup> m_render_groups;
    std::vector<Material> m_materials;

    std::vector<uint> m_indices;
    std::vector<FaceNativeData> m_face_native_data;

public:
    void load_from_file(std::string const &fileName) override {
        Mesh<TVertexComponents>::load_from_file(fileName);
        Mesh<TVertexComponents>::calculate_normals();

        m_vertex_count = static_cast<uint>(Mesh<TVertexComponents>::m_vertices.size());

        reset();

        initMaterials();
    }

    void simplify(float p = 0.5f) override {
        simplify(static_cast<uint>(p * Mesh<TVertexComponents>::m_faces.size()));
    }

    void simplify(uint verticesFinalCount) override {
        restore_faces();
        Mesh<TVertexComponents>::m_vertices.resize(m_vertex_count);

        auto start = std::chrono::high_resolution_clock::now();
        Mesh<TVertexComponents>::simplify(verticesFinalCount);
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> duration = end - start;

        std::cout << "algorithm duration - " << duration.count() << std::endl;

        reset();
        initMaterials(true);
    }

    RenderMesh() = default;
    ~RenderMesh() {
        reset();
    }

    void draw(Shader &shader) const {
        GLuint position_attribute = shader.attributeLocation("position");
        GLuint normal_attribute   = shader.attributeLocation("normal");
        GLuint color_attribute    = shader.attributeLocation("color");
        GLuint uv_attribute       = shader.attributeLocation("uv");

        glEnableVertexAttribArray(position_attribute);
        glEnableVertexAttribArray(normal_attribute);
        glEnableVertexAttribArray(color_attribute);
        glEnableVertexAttribArray(uv_attribute);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.position_desc);
        glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, sizeof(typename Mesh<TVertexComponents>::VertexType), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.normal_desc);
        glVertexAttribPointer(normal_attribute,   3, GL_FLOAT, GL_FALSE, sizeof(typename Mesh<TVertexComponents>::VertexType), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.color_desc);
        glVertexAttribPointer(color_attribute,    4, GL_FLOAT, GL_FALSE, sizeof(typename Mesh<TVertexComponents>::VertexType), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.uv_desc);
        glVertexAttribPointer(uv_attribute,       2, GL_FLOAT, GL_FALSE, sizeof(typename Mesh<TVertexComponents>::VertexType), nullptr);

        glProgramUniform1i(shader.getId(), shader.uniformLocation("tex"), 0);

        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D, m_textures.at(0));
        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, m_indices.data());

//        for (auto &render_group : m_render_groups) {
//            glBindTexture(GL_TEXTURE_2D, m_textures.at(m_materials.at(render_group.material_id).texture_id));
//            glDrawElements(GL_TRIANGLES, (render_group.face_id1 - render_group.face_id0 + 1) * 3, GL_UNSIGNED_INT, m_indices.data() + render_group.face_id0 * 3);
//        }

        glDisableVertexAttribArray(position_attribute);
        glDisableVertexAttribArray(normal_attribute);
        glDisableVertexAttribArray(color_attribute);
        glDisableVertexAttribArray(uv_attribute);

    }

private:
    void restore_faces() {
        for (auto &face_data : m_face_native_data) {
            memcpy(reinterpret_cast<uchar *>(Mesh<TVertexComponents>::m_faces.data() + face_data.face_id) + face_data.vertex_num * sizeof(uint),
                   &face_data.vertex_id,
                   sizeof(face_data.vertex_id));
        }
    }

    void reset() {
        gl::Buffer::free(vertexBuffer.position_desc);
        gl::Buffer::free(vertexBuffer.normal_desc);
        gl::Buffer::free(vertexBuffer.color_desc);
        gl::Buffer::free(vertexBuffer.uv_desc);

        for (auto desc : m_textures) {
            gl::Texture::free(desc);
        }
    }

    void initMaterials(bool copy_vertices = true);
};


template <typename T>
void RenderMesh<T>::initMaterials(bool copy_vertices) {
    auto const &materials_info = OBJReader::prevParseMaterialInfo;

    m_textures.clear();
    m_materials.clear();
    m_render_groups.clear();
    m_indices.clear();
    m_face_native_data.clear();

    for (auto const &texture : materials_info.data.textures) {
        m_textures.push_back(gl::Texture::createFromFile(texture));
    }

    for (auto const &material : materials_info.data.materials) {
        m_materials.push_back(Material{material.texture});
    }

    for (auto const &info : materials_info.info) {
        m_render_groups.push_back(RenderGroup{info.begin_idx, info.end_idx, info.material});
    }

    if (copy_vertices) {
        for (uint i = 0; i < Mesh<T>::m_faces.size(); i++) {
            auto &face = Mesh<T>::m_faces.at(i);

            if (face.uv0 != Mesh<T>::m_vertices.at(face.v0).components.uv) {
                Mesh<T>::m_vertices.push_back(Mesh<T>::m_vertices.at(face.v0));
                Mesh<T>::m_vertices.back().components.uv = face.uv0;

                m_face_native_data.push_back(FaceNativeData{i, 0, face.v0});

                face.v0 = static_cast<uint>(Mesh<T>::m_vertices.size()) - 1;
            }

            if (face.uv1 != Mesh<T>::m_vertices.at(face.v1).components.uv) {
                Mesh<T>::m_vertices.push_back(Mesh<T>::m_vertices.at(face.v1));
                Mesh<T>::m_vertices.back().components.uv = face.uv1;

                m_face_native_data.push_back(FaceNativeData{i, 1, face.v1});

                face.v1 = static_cast<uint>(Mesh<T>::m_vertices.size()) - 1;
            }

            if (face.uv2 != Mesh<T>::m_vertices.at(face.v2).components.uv) {
                Mesh<T>::m_vertices.push_back(Mesh<T>::m_vertices.at(face.v2));
                Mesh<T>::m_vertices.back().components.uv = face.uv2;

                m_face_native_data.push_back(FaceNativeData{i, 2, face.v2});

                face.v2 = static_cast<uint>(Mesh<T>::m_vertices.size()) - 1;
            }
        }
    }

    for (auto &face : Mesh<T>::m_faces) {
        m_indices.push_back(face.v0);
        m_indices.push_back(face.v1);
        m_indices.push_back(face.v2);
    }

    vertexBuffer.position_desc = gl::Buffer::create(Mesh<T>::m_vertices.data(), Mesh<T>::m_vertices.size() * sizeof(typename Mesh<T>::VertexType), VTABLE_OFFSET);
    vertexBuffer.normal_desc   = gl::Buffer::create(Mesh<T>::m_vertices.data(), Mesh<T>::m_vertices.size() * sizeof(typename Mesh<T>::VertexType), VTABLE_OFFSET + sizeof(glm::vec3));
    vertexBuffer.color_desc    = gl::Buffer::create(Mesh<T>::m_vertices.data(), Mesh<T>::m_vertices.size() * sizeof(typename Mesh<T>::VertexType), VTABLE_OFFSET + 2 * sizeof(glm::vec3));
    vertexBuffer.uv_desc       = gl::Buffer::create(Mesh<T>::m_vertices.data(), Mesh<T>::m_vertices.size() * sizeof(typename Mesh<T>::VertexType), VTABLE_OFFSET + 2 * sizeof(glm::vec3) + sizeof(glm::vec4));
}


#endif //MESHSIMPLIFICATION_RENDERMESH_H
