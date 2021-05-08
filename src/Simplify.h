#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <memory.h>
#include "Mesh.h"

#define loop(var_l,start_l,end_l) for ( int var_l=start_l;var_l<end_l;++var_l )


template <typename T>
class Mesh;


class SymetricMatrix {

public:

    // Constructor

    SymetricMatrix(double c=0) {
        for (int i = 0; i < 10; i++) {
            m[i] = c;
        }
    }

    SymetricMatrix(	double m11, double m12, double m13, double m14,
                       double m22, double m23, double m24,
                       double m33, double m34,
                       double m44) {
        m[0] = m11;  m[1] = m12;  m[2] = m13;  m[3] = m14;
        m[4] = m22;  m[5] = m23;  m[6] = m24;
        m[7] = m33;  m[8] = m34;
        m[9] = m44;
    }

    // Make plane

    SymetricMatrix(double a,double b,double c,double d)
    {
        m[0] = a*a;  m[1] = a*b;  m[2] = a*c;  m[3] = a*d;
        m[4] = b*b;  m[5] = b*c;  m[6] = b*d;
        m[7 ] =c*c;  m[8 ] = c*d;
        m[9 ] = d*d;
    }

    double operator[](int c) const { return m[c]; }

    // Determinant

    double det(	int a11, int a12, int a13,
                   int a21, int a22, int a23,
                   int a31, int a32, int a33)
    {
        double det =  m[a11]*m[a22]*m[a33] + m[a13]*m[a21]*m[a32] + m[a12]*m[a23]*m[a31]
                      - m[a13]*m[a22]*m[a31] - m[a11]*m[a23]*m[a32]- m[a12]*m[a21]*m[a33];
        return det;
    }

    const SymetricMatrix operator+(const SymetricMatrix& n) const
    {
        return SymetricMatrix( m[0]+n[0],   m[1]+n[1],   m[2]+n[2],   m[3]+n[3],
                               m[4]+n[4],   m[5]+n[5],   m[6]+n[6],
                               m[ 7]+n[ 7], m[ 8]+n[8 ],
                               m[ 9]+n[9 ]);
    }

    SymetricMatrix& operator+=(const SymetricMatrix& n)
    {
        m[0]+=n[0];   m[1]+=n[1];   m[2]+=n[2];   m[3]+=n[3];
        m[4]+=n[4];   m[5]+=n[5];   m[6]+=n[6];   m[7]+=n[7];
        m[8]+=n[8];   m[9]+=n[9];
        return *this;
    }

    double m[10];
};
///////////////////////////////////////////

namespace Simplify
{
    // Global Variables & Strctures

    using vec3f = glm::vec3;

    struct Triangle { int v[3];double err[4];int deleted,dirty; vec3f n; };
    struct Vertex { vec3f p;int tstart,tcount;SymetricMatrix q;int border;};
    struct Ref { int tid,tvertex; };

    extern std::vector<Triangle> triangles;
    extern std::vector<Vertex> vertices;
    extern std::vector<Ref> refs;

    // Helper functions

    double vertex_error(SymetricMatrix q, double x, double y, double z);
    double calculate_error(int id_v1, int id_v2, vec3f &p_result);
    bool flipped(vec3f p,int i0,int i1,Vertex &v0,Vertex &v1,std::vector<int> &deleted);
    void update_triangles(int i0,Vertex &v,std::vector<int> &deleted,int &deleted_triangles);
    void update_mesh(int iteration);

    template <typename T>
    void compact_mesh(Mesh<T> *mesh) {
        uint dst = 0;

        for (uint i = 0; i < vertices.size(); i++) {
            vertices[i].tcount = 0;
        }

        for (int i = 0; i < triangles.size(); i++) {
            if (!triangles[i].deleted) {
                Triangle &t = triangles[i];
                triangles[dst++] = t;

                mesh->m_faces.at(dst - 1) = mesh->m_faces.at(i);

                for (uint j = 0; j < 3; j++) {
                    vertices[t.v[j]].tcount = 1;
                }
            }
        }

        triangles.resize(dst);
        mesh->m_faces.resize(dst);
        dst = 0;

        for (int i = 0; i < vertices.size(); i++) {
            if (vertices[i].tcount) {
                vertices[i].tstart = dst;
                vertices[dst].p = vertices[i].p;

                mesh->m_vertices.at(dst) = mesh->m_vertices.at(i);

                dst++;
            }
        }

        for (uint i = 0; i < triangles.size(); i++) {
            Triangle &t = triangles[i];
            auto &mesh_triangle = mesh->m_faces.at(i);

            for (int j = 0; j < 3; j++) {
                t.v[j] = vertices[t.v[j]].tstart;

                *(&mesh_triangle.v0 + j) = static_cast<uint>(t.v[j]);
            }
        }

        vertices.resize(dst);
        mesh->m_vertices.resize(dst);
    }

    template <typename T>
    void simplify_mesh(Mesh<T> *mesh, int target_count, double agressiveness=7) {
        // init
        printf("%s - start\n",__FUNCTION__);
        //int timeStart=timeGetTime();

        vertices.clear();
        triangles.clear();

        for (auto &vertex : mesh->m_vertices) {
            Vertex v;
            v.p = vertex.components.position;

            vertices.push_back(v);
        }

        for (auto &triangle : mesh->m_faces) {
            Triangle t;
            memcpy(t.v, &triangle.v0, 3 * sizeof(uint));

            triangles.push_back(t);
        }

        for (int i = 0; i < triangles.size(); i++) {
            triangles[i].deleted = 0;
        }

        // main iteration loop

        int deleted_triangles = 0;
        std::vector<int> deleted0,deleted1;
        int triangle_count = triangles.size();

        loop(iteration,0,10000)
        {
            // target number of triangles reached ? Then break
            printf("iteration %d - triangles %d\n",iteration,triangle_count-deleted_triangles);
            if(triangle_count-deleted_triangles<=target_count)break;

            // update mesh once in a while
            if(iteration%1==0)
            {
                update_mesh(iteration);
            }

            // clear dirty flag
            for (int i = 0; i < triangles.size(); i++) {
                triangles[i].dirty = 0;
            }

            //
            // All triangles with edges below the threshold will be removed
            //
            // The following numbers works well for most models.
            // If it does not, try to adjust the 3 parameters
            //
            double threshold = 0.000000001*pow(double(iteration+3),agressiveness);

            // remove vertices & mark deleted triangles
            for (int i = 0; i < triangles.size(); i++)
            {
                Triangle &t=triangles[i];
                if(t.err[3]>threshold) continue;
                if(t.deleted) continue;
                if(t.dirty) continue;

                for (int j = 0; j < 3; j++) if(t.err[j]<threshold)
                    {
                        int i0=t.v[ j     ]; Vertex &v0 = vertices[i0];
                        int i1=t.v[(j+1)%3]; Vertex &v1 = vertices[i1];

                        // Border check
                        if(v0.border != v1.border)  continue;

                        // Compute vertex to collapse to
                        vec3f p;
                        calculate_error(i0,i1,p);

                        deleted0.resize(v0.tcount); // normals temporarily
                        deleted1.resize(v1.tcount); // normals temporarily

                        // don't remove if flipped
                        if( flipped(p,i0,i1,v0,v1,deleted0) ) continue;
                        if( flipped(p,i1,i0,v1,v0,deleted1) ) continue;

                        // not flipped, so remove edge
                        v0.p = p;
                        v0.q = v1.q + v0.q;
                        int tstart=refs.size();

                        update_triangles(i0,v0,deleted0,deleted_triangles);
                        update_triangles(i0,v1,deleted1,deleted_triangles);

                        int tcount = refs.size() - tstart;

                        if(tcount <= v0.tcount) {
                            // save ram
                            if (tcount) {
                                memcpy(&refs[v0.tstart], &refs[tstart], tcount * sizeof(Ref));
                            }
                        }
                        else {
                            // append
                            v0.tstart = tstart;
                        }

                        v0.tcount=tcount;
                        break;
                    }
                // done?
                if(triangle_count-deleted_triangles<=target_count)break;
            }
        }

        // clean up mesh
        compact_mesh(mesh);

        // ready
//        int timeEnd=timeGetTime();
//        printf("%s - %d/%d %d%% removed in %d ms\n",__FUNCTION__,
//               triangle_count-deleted_triangles,
//               triangle_count,deleted_triangles*100/triangle_count,
//               timeEnd-timeStart);

    }
};
///////////////////////////////////////////

