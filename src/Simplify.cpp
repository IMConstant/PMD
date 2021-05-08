#include "Simplify.h"
#include "Mesh.h"


namespace Simplify {
    std::vector<Triangle> triangles;
    std::vector<Vertex> vertices;
    std::vector<Ref> refs;

    // Check if a triangle flips when this edge is removed

    bool flipped(vec3f p,int i0,int i1,Vertex &v0,Vertex &v1,std::vector<int> &deleted)
    {
        int bordercount=0;
        for (int k = 0; k < v0.tcount; k++)
        {
            Triangle &t=triangles[refs[v0.tstart+k].tid];
            if(t.deleted)continue;

            int s=refs[v0.tstart+k].tvertex;
            int id1 = t.v[(s + 1) % 3];
            int id2 = t.v[(s + 2) % 3];

            if(id1==i1 || id2==i1) // delete ?
            {
                bordercount++;
                deleted[k]=1;
                continue;
            }
            vec3f d1 = vertices[id1].p-p; d1 = glm::normalize(d1);
            vec3f d2 = vertices[id2].p-p; d2 = glm::normalize(d2);
            if(fabs(glm::dot<3, float>(d1, d2))>0.999) return true;
            vec3f n;
            n = glm::cross(d1, d2);
            n = glm::normalize(n);
            deleted[k]=0;

            if(glm::dot<3, float>(n, t.n) < 0.2) return true;
        }
        return false;
    }

    // Update triangle connections and edge error after a edge is collapsed

    void update_triangles(int i0,Vertex &v,std::vector<int> &deleted,int &deleted_triangles) {
        vec3f p;

        for (int k = 0; k < v.tcount; k++) {
            Ref &r = refs[v.tstart + k];
            Triangle &t = triangles[r.tid];

            if(t.deleted) continue;

            if(deleted[k]) {
                t.deleted = 1;
                deleted_triangles++;
                continue;
            }

            t.v[r.tvertex] = i0;
            t.dirty = 1;
            t.err[0] = calculate_error(t.v[0], t.v[1], p);
            t.err[1] = calculate_error(t.v[1], t.v[2], p);
            t.err[2] = calculate_error(t.v[2], t.v[0], p);
            t.err[3] = glm::min(t.err[0], glm::min(t.err[1], t.err[2]));
            refs.push_back(r);
        }
    }

    // compact triangles, compute edge error and build reference list

    void update_mesh(int iteration) {
        if(iteration > 0) {
            int dst = 0;

            for (int i = 0; i < triangles.size(); i++)
                if(!triangles[i].deleted) {
                    triangles[dst++]=triangles[i];
                }

            triangles.resize(dst);
        }
        //
        // Init Quadrics by Plane & Edge Errors
        //
        // required at the beginning ( iteration == 0 )
        // recomputing during the simplification is not required,
        // but mostly improves the result for closed meshes
        //
        if( iteration == 0 ) {
            for (int i = 0; i < vertices.size(); i++) {
                vertices[i].q = SymetricMatrix(0.0);
            }

            for (int i = 0; i < triangles.size(); i++) {
                Triangle &t=triangles[i];
                vec3f n,p[3];

                for (int j = 0; j < 3; j++) {
                    p[j]=vertices[t.v[j]].p;
                }

                n = glm::cross(p[1] - p[0], p[2] - p[0]);
                n = glm::normalize(n);
                t.n = n;

                for (int j = 0; j < 3; j++) {
                    vertices[t.v[j]].q = vertices[t.v[j]].q + SymetricMatrix(n.x, n.y, n.z, -glm::dot<3, float>(n, p[0]));
                }
            }

            for (int i = 0; i < triangles.size(); i++) {
                // Calc Edge Error
                Triangle &t = triangles[i];
                vec3f p;

                for (int j = 0; j < 3; j++) {
                    t.err[j] = calculate_error(t.v[j], t.v[(j + 1) % 3], p);
                }

                t.err[3]=glm::min(t.err[0],glm::min(t.err[1],t.err[2]));
            }
        }

        // Init Reference ID list
        for (int i = 0; i < vertices.size(); i++) {
            vertices[i].tstart=0;
            vertices[i].tcount=0;
        }
        for (int i = 0; i < triangles.size(); i++) {
            Triangle &t = triangles[i];

            for (int j = 0; j < 3; j++) {
                vertices[t.v[j]].tcount++;
            }
        }

        int tstart = 0;

        for (int i = 0; i < vertices.size(); i++) {
            Vertex &v = vertices[i];
            v.tstart = tstart;
            tstart += v.tcount;
            v.tcount = 0;
        }

        // Write References
        refs.resize(triangles.size() * 3);

        for (int i = 0; i < triangles.size(); i++) {
            Triangle &t=triangles[i];

            for (int j = 0; j < 3; j++) {
                Vertex &v = vertices[t.v[j]];
                refs[v.tstart + v.tcount].tid = i;
                refs[v.tstart + v.tcount].tvertex = j;
                v.tcount++;
            }
        }

        // Identify boundary : vertices[].border=0,1
        if( iteration == 0 ) {
            std::vector<int> vcount,vids;

            for (int i = 0; i < vertices.size(); i++) {
                vertices[i].border = 0;
            }

            for (int i = 0; i < vertices.size(); i++) {
                Vertex &v = vertices[i];
                vcount.clear();
                vids.clear();

                for (int j = 0; j < v.tcount; j++) {
                    int k = refs[v.tstart+j].tid;
                    Triangle &t=triangles[k];

                    for (int k = 0; k < 3; k++) {
                        int ofs=0,id=t.v[k];

                        while(ofs < vcount.size()) {
                            if(vids[ofs] == id) break;
                            ofs++;
                        }

                        if(ofs==vcount.size()) {
                            vcount.push_back(1);
                            vids.push_back(id);
                        }
                        else {
                            vcount[ofs]++;
                        }
                    }
                }

                for (int j = 0; j < vcount.size(); j++) {
                    if (vcount[j] == 1) {
                        vertices[vids[j]].border = 1;
                    }
                }
            }
        }
    }

    // Error between vertex and Quadric

    double vertex_error(SymetricMatrix q, double x, double y, double z) {
        return   q[0]*x*x + 2*q[1]*x*y + 2*q[2]*x*z + 2*q[3]*x + q[4]*y*y
                 + 2*q[5]*y*z + 2*q[6]*y + q[7]*z*z + 2*q[8]*z + q[9];
    }

    // Error for one edge

    double calculate_error(int id_v1, int id_v2, vec3f &p_result)
    {
        // compute interpolated vertex

        SymetricMatrix q = vertices[id_v1].q + vertices[id_v2].q;

        bool   border = vertices[id_v1].border & vertices[id_v2].border;
        double error=0;
        double det = q.det(0, 1, 2, 1, 4, 5, 2, 5, 7);

        if ( det != 0 && !border ) {
            // q_delta is invertible
            p_result.x = -1 / det * (q.det(1, 2, 3, 4, 5, 6, 5, 7 , 8));	// vx = A41/det(q_delta)
            p_result.y =  1 / det * (q.det(0, 2, 3, 1, 5, 6, 2, 7 , 8));	// vy = A42/det(q_delta)
            p_result.z = -1 / det * (q.det(0, 1, 3, 1, 4, 6, 2, 5,  8));	// vz = A43/det(q_delta)
            error = vertex_error(q, p_result.x, p_result.y, p_result.z);
        }
        else {
            // det = 0 -> try to find best result
            vec3f p1 = vertices[id_v1].p;
            vec3f p2 = vertices[id_v2].p;
            vec3f p3 = (p1 + p2) / 2.0f;

            double error1 = vertex_error(q, p1.x,p1.y,p1.z);
            double error2 = vertex_error(q, p2.x,p2.y,p2.z);
            double error3 = vertex_error(q, p3.x,p3.y,p3.z);
            error = glm::min(error1, glm::min(error2, error3));
            if (error1 == error) p_result=p1;
            if (error2 == error) p_result=p2;
            if (error3 == error) p_result=p3;
        }

        return error;
    }
} // namespace MySimplify