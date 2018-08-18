//
//  IAFmtObj3ds.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#include "IAFmtObj3ds.h"

#include "lib3ds.h"

#if 0

#include "Iota.h"
#include "../geometry/IAMesh.h"
#include "../geometry/IAMath.h"

#include <stdio.h>


static int max_vertices = 0;
static int max_texcos = 0;
static int max_normals = 0;


/**
 Load a single node from a 3ds file.
 */
void load3ds(Lib3dsFile *f, Lib3dsMeshInstanceNode *node) {
    float (*orig_vertices)[3];
    int export_texcos;
    int export_normals;
    int i;
    Lib3dsMesh *mesh;

    IAMesh *msh = new IAMesh();
    Iota.gMeshList->push_back(msh);

    mesh = lib3ds_file_mesh_for_node(f, (Lib3dsNode*)node);
    if (!mesh || !mesh->vertices) return;

    orig_vertices = (float(*)[3])malloc(sizeof(float) * 3 * mesh->nvertices);
    memcpy(orig_vertices, mesh->vertices, sizeof(float) * 3 * mesh->nvertices);
    {
        float inv_matrix[4][4], M[4][4];
        float tmp[3];
        int i;

        lib3ds_matrix_copy(M, node->base.matrix);
        lib3ds_matrix_translate(M, -node->pivot[0], -node->pivot[1], -node->pivot[2]);
        lib3ds_matrix_copy(inv_matrix, mesh->matrix);
        lib3ds_matrix_inv(inv_matrix);
        lib3ds_matrix_mult(M, M, inv_matrix);

        //lib3ds_matrix_rotate(M, 90, 1, 0, 0);

        for (i = 0; i < mesh->nvertices; ++i) {
            lib3ds_vector_transform(tmp, M, mesh->vertices[i]);
            lib3ds_vector_copy(mesh->vertices[i], tmp);
        }
    }
    {
        int i;
        for (i = 0; i < mesh->nvertices; ++i) {
            IAVertex *isPoint = new IAVertex();
            isPoint->pPosition.read(mesh->vertices[i]);
            //isPoint->pPosition *= 10;
            //isPoint->pPosition *= 40; // mokey full size
            //isPoint->pPosition *= 5; // mokey tiny (z=-5...+5)
#if 0
            isPoint->pTex.set(
                              isPoint->pPosition.x() * 0.8 + 0.5,
                              -isPoint->pPosition.y() * 0.8 + 0.5,
                              0.0
                              );
            //isPoint->pPosition *= 10; // mokey tiny (z=-5...+5)
            isPoint->pPosition *= 30; // mokey tiny (z=-5...+5)
#endif
            msh->vertexList.push_back(isPoint);
            msh->updateBoundingBox(isPoint->pPosition);
        }
    }

    /*
     Monkey Model bounding box is:
     x: -1.36719, 1.36719
     y: -0.984375, 0.984375
     z: -0.851562, 0.851562
     Iota wants mm, so scale by 40, resulting in a 112mm wide head.
     */

    export_texcos = (mesh->texcos != 0);
    export_normals = (mesh->faces != 0);

    //  for (i = 0; i < mesh->nvertices; ++i) {
    //    fprintf(o, "v %f %f %f\n", mesh->vertices[i][0],
    //            mesh->vertices[i][1],
    //            mesh->vertices[i][2]);
    //  }
    //  fprintf(o, "# %d vertices\n", mesh->nvertices);

    //  if (export_texcos) {
    //    for (i = 0; i < mesh->nvertices; ++i) {
    //      fprintf(o, "vt %f %f\n", mesh->texcos[i][0],
    //              mesh->texcos[i][1]);
    //    }
    //    fprintf(o, "# %d texture vertices\n", mesh->nvertices);
    //  }

    //  if (export_normals) {
    //    float (*normals)[3] = (float(*)[3])malloc(sizeof(float) * 9 * mesh->nfaces);
    //    lib3ds_mesh_calculate_vertex_normals(mesh, normals);
    //    for (i = 0; i < 3 * mesh->nfaces; ++i) {
    //      fprintf(o, "vn %f %f %f\n", normals[i][0],
    //              normals[i][1],
    //              normals[i][2]);
    //    }
    //    free(normals);
    //    fprintf(o, "# %d normals\n", 3 * mesh->nfaces);
    //  }

    {
        //    int mat_index = -1;
        for (i = 0; i < mesh->nfaces; ++i) {

            //      if (mat_index != mesh->faces[i].material) {
            //        mat_index = mesh->faces[i].material;
            //        if (mat_index != -1) {
            //          fprintf(o, "usemtl %s\n", f->materials[mat_index]->name);
            //        }
            //      }
            //      fprintf(o, "f ");
            //      for (j = 0; j < 3; ++j) {
            IATriangle *t = new IATriangle();
            t->pVertex[0] = msh->vertexList[mesh->faces[i].index[0]];
            t->pVertex[1] = msh->vertexList[mesh->faces[i].index[1]];
            t->pVertex[2] = msh->vertexList[mesh->faces[i].index[2]];
            //      IATriangle->print();
            msh->addFace(t);
            //        fprintf(o, "%d", mesh->faces[i].index[j] + max_vertices + 1);
            //        int vi;
            //        float *fv;
            //        vi = mesh->faces[i].index[j];
            //        fv = mesh->vertices[vi];
            //        glVertex3fv(fv);
            //        if (export_texcos) {
            //          fprintf(o, "/%d", mesh->faces[i].index[j] + max_texcos + 1);
            //        } else if (export_normals) {
            //          fprintf(o, "/");
            //        }
            //        if (export_normals) {
            //          fprintf(o, "/%d", 3 * i + j + max_normals + 1);
            //        }
            //        if (j < 3) {
            //          fprintf(o, " ");
            //        }
            //      }
            //      fprintf(o, "\n");
        }
    }

    max_vertices += mesh->nvertices;
    if (export_texcos)
        max_texcos += mesh->nvertices;
    if (export_normals)
        max_normals += 3 * mesh->nfaces;

    memcpy(mesh->vertices, orig_vertices, sizeof(float) * 3 * mesh->nvertices);
    free(orig_vertices);

    msh->validate();
    // TODO: fix seams
    // TODO: fix zero size holes
    // TODO: fix degenrate triangles
    msh->fixHoles();
    msh->validate();

    msh->clearNormals();
    msh->calculateNormals();
}



/**
 Load a model from a 3ds file.
 */
void load3ds(const char *filename)
{
    Lib3dsFile *f = lib3ds_file_open(filename);
    if (!f->nodes)
        lib3ds_file_create_nodes_for_meshes(f);
    lib3ds_file_eval(f, 0);
    Lib3dsNode *p;
    for (p = f->nodes; p; p = p->next) {
        if (p->type == LIB3DS_NODE_MESH_INSTANCE) {
            load3ds(f, (Lib3dsMeshInstanceNode*)p);
        }
    }
    lib3ds_file_free(f);
}

#else

void load3ds(Lib3dsFile *f, Lib3dsMeshInstanceNode *node) { }

#endif
