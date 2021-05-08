#ifndef MESHSIMPLIFICATION_MESH_H
#define MESHSIMPLIFICATION_MESH_H

#include <CGAL/Simple_cartesian.h>
// #include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/IO/OBJ_reader.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>

#include <type_traits>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "Simplify.h"
#include "OBJReader.h"

typedef CGAL::Simple_cartesian<float>                Kernel;
typedef Kernel::Point_3                              Point_3;
typedef CGAL::Surface_mesh<Point_3>                  Surface_mesh;


using uchar = unsigned char;
using uint = unsigned int;


float tetrahedronSignedVolume(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2);


template <typename T>
T linearInterpolation(const T &v0, const T &v1, float t) {
    return v0 + t * (v1 - v0);
}


struct VertexComponents {
    virtual void interpolate(VertexComponents *out, VertexComponents const &v0, VertexComponents const &v1, float t) const = 0;
};


struct VertexBaseComponents : public VertexComponents {
    glm::vec3 position;
    glm::vec3 normal;

    void interpolate(VertexComponents *out, VertexComponents const &v0, VertexComponents const &v1, float t) const override {
        auto &_v0 = static_cast<VertexBaseComponents const &>(v0);
        auto &_v1 = static_cast<VertexBaseComponents const &>(v1);
        auto *_out = static_cast<VertexBaseComponents *>(out);

        _out->position = linearInterpolation(_v0.position, _v1.position, t);
        _out->normal   = linearInterpolation(_v0.normal,   _v1.normal,   t);
    }
};


struct UVVertexComponent : public VertexBaseComponents {
    void interpolate(VertexComponents *out, VertexComponents const &v0, VertexComponents const &v1, float t) const override {
        VertexBaseComponents::interpolate(out, v0, v1, t);
    }
};


struct VertexComponentsColored : public VertexBaseComponents {
    glm::vec4 color;

    void interpolate(VertexComponents *out, VertexComponents const &v0, VertexComponents const &v1, float t) const override {
        VertexBaseComponents::interpolate(out, v0, v1, t);
    }
};


template <typename TVertexComponents>
class Vertex {
    static_assert(std::is_base_of<VertexComponents, TVertexComponents>::value,
                       "Class type has to be derived from VertexComponents class");

public:

    TVertexComponents components;
};

using MeshVertex = Vertex<VertexBaseComponents>;


struct Face {
    uint v0;
    uint v1;
    uint v2;
};


template <typename TVertexComponents>
class Mesh;


namespace Simplify {
    template <class T>
    void simplify_mesh(T *mesh, int target_count, double agressiveness);

    template <class T>
    void compact_mesh(T *mesh);
}



template <class TVertexComponents>
class Mesh {
    std::vector<Vertex<TVertexComponents>> m_vertices;
    std::vector<Face> m_faces;

public:
    template <class T>
    friend void Simplify::simplify_mesh(Mesh<T> *mesh, int target_count, double agressiveness);

    template <class T>
    friend void Simplify::compact_mesh(Mesh<T> *mesh);

    using VertexType = Vertex<TVertexComponents>;

    static OBJReader::Layout layoutDefault;

public:
    Mesh() {
        Mesh<TVertexComponents>::layoutDefault.position.offset = static_cast<uint>(offsetof(TVertexComponents, position));
    }

    void load_from_file(std::string const &fileName) {
        OBJReader::Layout layout;
        layout.position.offset = 0;
        layout.color.offset = 24;

        m_vertices.clear();
        m_faces.clear();

        OBJReader::read(fileName, m_vertices, m_faces, layout);
    }

    std::vector<Vertex<TVertexComponents>> const &vertices() const {
        return m_vertices;
    }

    std::vector<Face> const &faces() const {
        return m_faces;
    }

    float area() const {
        float _area = 0.f;

        for (uint i = 0; i < m_faces.size(); i++) {
            _area += triangle_area(i);
        }
    }

    float volume() const {
        float _volume = 0.0f;

        for (auto const &face : m_faces) {
            _volume += tetrahedronSignedVolume(
                    m_vertices.at(face.v0).components.position,
                    m_vertices.at(face.v1).components.position,
                    m_vertices.at(face.v2).components.position
            );
        }

        return _volume;
    }

    void simplify(float p = 0.5f);
    void simplify(uint verticesFinalCount);

    void calculate_normals();

private:
    float triangle_area(uint i) const {
        auto &triangle = m_faces.at(i);

        auto const &_vp0 = m_vertices.at(triangle.v0).components.position;
        auto const &_vp1 = m_vertices.at(triangle.v1).components.position;
        auto const &_vp2 = m_vertices.at(triangle.v2).components.position;

        return 0.5f * glm::length(glm::cross(_vp1 - _vp0, _vp2 - _vp0));
    }
};


template <typename T>
OBJReader::Layout Mesh<T>::layoutDefault;


template <typename T>
void Mesh<T>::calculate_normals() {
    for (auto &vertex : m_vertices) {
        vertex.components.normal = glm::vec3(0.f);
    }

    for (auto &face : m_faces) {
        glm::vec3 A = m_vertices.at(face.v1).components.position - m_vertices.at(face.v0).components.position;
        glm::vec3 B = m_vertices.at(face.v2).components.position - m_vertices.at(face.v0).components.position;

        glm::vec3 N = glm::normalize(glm::cross(A, B));

        m_vertices.at(face.v0).components.normal += N;
        m_vertices.at(face.v1).components.normal += N;
        m_vertices.at(face.v2).components.normal += N;
    }

    for (auto &vertex : m_vertices) {
        vertex.components.normal = glm::normalize(vertex.components.normal);
    }
}


template <typename T>
void Mesh<T>::simplify(float p) {
    simplify(static_cast<uint>(p * m_faces.size()));
}

template <typename T>
void Mesh<T>::simplify(uint verticesFinalCount) {
    Simplify::simplify_mesh<T>(this, verticesFinalCount, 4);
}

#endif //MESHSIMPLIFICATION_MESH_H
