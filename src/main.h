//
//  main.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <assert.h>
//#include <math.h>
//#include <ctype.h>
//#include <string.h>
//
//#include "lib3ds.h"
//
#include "IAMesh.h"
#include "IASlice.h"
//
#include "printer/IAPrinter.h"
//
//
//#ifdef _MSC_VER
//#pragma warning ( disable : 4996 )
//#endif
//

extern class Fl_Slider *zSlider1, *zSlider2;
extern class Fl_RGB_Image *texture;


extern IAMeshList gMeshList;

extern IASlice gMeshSlice;

extern IAPrinter gPrinter;

//
//GLUtesselator *gGluTess = 0;
//

extern bool gShowSlice;


extern double min(double a, double b);
extern double max(double a, double b);

extern float min(float a, float b);
extern float max(float a, float b);

extern int gWriteSliceNext;
extern FILE *gOutFile;

extern void writeInt(FILE *f, int32_t x);

//{
//    uint8_t v;
//    // bits 34..28
//    if (x&(0xffffffff<<28)) {
//        v = ((x>>28) & 0x7f) | 0x80;
//        fputc(v, f);
//    }
//    // bits 27..21
//    if (x&(0xffffffff<<21)) {
//        v = ((x>>21) & 0x7f) | 0x80;
//        fputc(v, f);
//    }
//    // bits 20..14
//    if (x&(0xffffffff<<14)) {
//        v = ((x>>14) & 0x7f) | 0x80;
//        fputc(v, f);
//    }
//    // bits 13..7
//    if (x&(0xffffffff<<7)) {
//        v = ((x>>7) & 0x7f) | 0x80;
//        fputc(v, f);
//    }
//    // bits 6..0
//    v = x & 0x7f;
//    fputc(v, f);
//}
//
//// -----------------------------------------------------------------------------
//
//static int max_vertices = 0;
//static int max_texcos = 0;
//static int max_normals = 0;
//
//
//// -----------------------------------------------------------------------------
//
//float minf(float a, float b) { return (a<b)?a:b; }
//float maxf(float a, float b) { return (a>b)?a:b; }
//
//
//void drawModelGouraud()
//{
//    int i, n = (int)gMeshList.size();
//    for (i=0; i<n; i++) {
//        IAMesh *IAMesh = gMeshList[i];
//        glDepthRange (0.1, 1.0);
//        IAMesh->drawGouraud();
//    }
//}
//
//
//
//void drawModelFlat(unsigned int color)
//{
//    gMeshList.drawFlat(color);
//}
//
//
//void drawModelShrunk(unsigned int color, double d)
//{
//    int i, n = (int)gMeshList.size();
//    for (i=0; i<n; i++) {
//        IAMesh *IAMesh = gMeshList[i];
//        IAMesh->drawShrunk(color, d);
//    }
//}
//
//void setShaders() {
//
//    GLuint v, f, p;
//
//    v = glCreateShader(GL_VERTEX_SHADER);
//    f = glCreateShader(GL_FRAGMENT_SHADER);
//
//    /*
//     vec4 v = vec4(gl_Vertex);
//     v.z = 0.0;
//     gl_Position = gl_ModelViewProjectionMatrix * v;
//     */
//    const char * vv =
//    "void main(void) {\n"
//    "    gl_Position = ftransform();\n"
//    "}";
//
//    //  "void main()\n"
//    //  "{\n"
//    //  "  gl_Position = ftransform();\n"
//    //  "}\n"
//    //  ;
//
//    // if (pixelIsSilly) dicard;
//    const char * ff =
//    "void main() {\n"
//    "    gl_FragColor = vec4( 1, 1, 0, 1);\n"
//    "}"
//    ;
//
//    glShaderSource(v, 1, &vv,NULL);
//    glShaderSource(f, 1, &ff,NULL);
//
//    glCompileShader(v);
//    glCompileShader(f);
//
//    p = glCreateProgram();
//
//    glAttachShader(p,v);
//    glAttachShader(p,f);
//
//    glLinkProgram(p);
//    glUseProgram(p);
//}
//
//void clipToSlice(double z1, double z2)
//{
//    glMatrixMode (GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(-66.1,66.1,-66.1,66.1, -z1, -z1-z2); // mm
//    glMatrixMode(GL_MODELVIEW);
//}
//
//void dontClipToSlice()
//{
//    glMatrixMode (GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(-66.1,66.1,-66.1,66.1,-66.1,66.1); // mm
//    glMatrixMode(GL_MODELVIEW);
//}
//
//
//MyGLView *glView = 0L;
//
//double minX = 0.0, maxX = 0.0, minY = 0.0, maxY = 0.0, minZ = 0.0, maxZ = 0.0;
//
//
///**
// Load a single node from a 3ds file.
// */
//void load3ds(Lib3dsFile *f, Lib3dsMeshInstanceNode *node) {
//    float (*orig_vertices)[3];
//    int export_texcos;
//    int export_normals;
//    int i;
//    Lib3dsMesh *mesh;
//
//    IAMesh *msh = new IAMesh();
//    gMeshList.push_back(msh);
//
//    mesh = lib3ds_file_mesh_for_node(f, (Lib3dsNode*)node);
//    if (!mesh || !mesh->vertices) return;
//
//    orig_vertices = (float(*)[3])malloc(sizeof(float) * 3 * mesh->nvertices);
//    memcpy(orig_vertices, mesh->vertices, sizeof(float) * 3 * mesh->nvertices);
//    {
//        float inv_matrix[4][4], M[4][4];
//        float tmp[3];
//        int i;
//
//        lib3ds_matrix_copy(M, node->base.matrix);
//        lib3ds_matrix_translate(M, -node->pivot[0], -node->pivot[1], -node->pivot[2]);
//        lib3ds_matrix_copy(inv_matrix, mesh->matrix);
//        lib3ds_matrix_inv(inv_matrix);
//        lib3ds_matrix_mult(M, M, inv_matrix);
//
//        //lib3ds_matrix_rotate(M, 90, 1, 0, 0);
//
//        for (i = 0; i < mesh->nvertices; ++i) {
//            lib3ds_vector_transform(tmp, M, mesh->vertices[i]);
//            lib3ds_vector_copy(mesh->vertices[i], tmp);
//        }
//    }
//    {
//        int i;
//        for (i = 0; i < mesh->nvertices; ++i) {
//            IAVertex *isPoint = new IAVertex();
//            isPoint->pPosition.read(mesh->vertices[i]);
//            minX = min(minX, isPoint->pPosition.x());
//            maxX = max(maxX, isPoint->pPosition.x());
//            minY = min(minY, isPoint->pPosition.y());
//            maxY = max(maxY, isPoint->pPosition.y());
//            minZ = min(minZ, isPoint->pPosition.z());
//            maxZ = max(maxZ, isPoint->pPosition.z());
//            //isPoint->pPosition *= 10;
//            //isPoint->pPosition *= 40; // mokey full size
//            //isPoint->pPosition *= 5; // mokey tiny (z=-5...+5)
//#ifdef M_MONKEY
//            isPoint->pTex.set(
//                              isPoint->pPosition.x() * 0.8 + 0.5,
//                              -isPoint->pPosition.y() * 0.8 + 0.5,
//                              0.0
//                              );
//            //isPoint->pPosition *= 10; // mokey tiny (z=-5...+5)
//            isPoint->pPosition *= 30; // mokey tiny (z=-5...+5)
//            isPoint->pInitialPosition = isPoint->pPosition;
//#elif defined M_DRAGON
//            isPoint->pPosition *= 1; // dragon (z=-5...+5)
//#elif defined M_XYZ
//            isPoint->pPosition *= 1; // pepsi (z=-5...+5)
//#endif
//            msh->vertexList.push_back(isPoint);
//        }
//    }
//
//    printf("Model bounding box is:\n  x: %g, %g\n  y: %g, %g\n  z: %g, %g\n",
//           minX, maxX, minY, maxY, minZ, maxZ);
//    /*
//     Monkey Model bounding box is:
//     x: -1.36719, 1.36719
//     y: -0.984375, 0.984375
//     z: -0.851562, 0.851562
//     Iota wants mm, so scale by 40, resulting in a 112mm wide head.
//     */
//
//    export_texcos = (mesh->texcos != 0);
//    export_normals = (mesh->faces != 0);
//
//    //  for (i = 0; i < mesh->nvertices; ++i) {
//    //    fprintf(o, "v %f %f %f\n", mesh->vertices[i][0],
//    //            mesh->vertices[i][1],
//    //            mesh->vertices[i][2]);
//    //  }
//    //  fprintf(o, "# %d vertices\n", mesh->nvertices);
//
//    //  if (export_texcos) {
//    //    for (i = 0; i < mesh->nvertices; ++i) {
//    //      fprintf(o, "vt %f %f\n", mesh->texcos[i][0],
//    //              mesh->texcos[i][1]);
//    //    }
//    //    fprintf(o, "# %d texture vertices\n", mesh->nvertices);
//    //  }
//
//    //  if (export_normals) {
//    //    float (*normals)[3] = (float(*)[3])malloc(sizeof(float) * 9 * mesh->nfaces);
//    //    lib3ds_mesh_calculate_vertex_normals(mesh, normals);
//    //    for (i = 0; i < 3 * mesh->nfaces; ++i) {
//    //      fprintf(o, "vn %f %f %f\n", normals[i][0],
//    //              normals[i][1],
//    //              normals[i][2]);
//    //    }
//    //    free(normals);
//    //    fprintf(o, "# %d normals\n", 3 * mesh->nfaces);
//    //  }
//
//    {
//        //    int mat_index = -1;
//        for (i = 0; i < mesh->nfaces; ++i) {
//
//            //      if (mat_index != mesh->faces[i].material) {
//            //        mat_index = mesh->faces[i].material;
//            //        if (mat_index != -1) {
//            //          fprintf(o, "usemtl %s\n", f->materials[mat_index]->name);
//            //        }
//            //      }
//            //      fprintf(o, "f ");
//            //      for (j = 0; j < 3; ++j) {
//            IATriangle *t = new IATriangle();
//            t->pVertex[0] = msh->vertexList[mesh->faces[i].index[0]];
//            t->pVertex[1] = msh->vertexList[mesh->faces[i].index[1]];
//            t->pVertex[2] = msh->vertexList[mesh->faces[i].index[2]];
//            //      IATriangle->print();
//            msh->addFace(t);
//            //        fprintf(o, "%d", mesh->faces[i].index[j] + max_vertices + 1);
//            //        int vi;
//            //        float *fv;
//            //        vi = mesh->faces[i].index[j];
//            //        fv = mesh->vertices[vi];
//            //        glVertex3fv(fv);
//            //        if (export_texcos) {
//            //          fprintf(o, "/%d", mesh->faces[i].index[j] + max_texcos + 1);
//            //        } else if (export_normals) {
//            //          fprintf(o, "/");
//            //        }
//            //        if (export_normals) {
//            //          fprintf(o, "/%d", 3 * i + j + max_normals + 1);
//            //        }
//            //        if (j < 3) {
//            //          fprintf(o, " ");
//            //        }
//            //      }
//            //      fprintf(o, "\n");
//        }
//    }
//
//    max_vertices += mesh->nvertices;
//    if (export_texcos)
//        max_texcos += mesh->nvertices;
//    if (export_normals)
//        max_normals += 3 * mesh->nfaces;
//
//    memcpy(mesh->vertices, orig_vertices, sizeof(float) * 3 * mesh->nvertices);
//    free(orig_vertices);
//
//    msh->validate();
//    // TODO: fix seams
//    // TODO: fix zero size holes
//    // TODO: fix degenrate triangles
//    msh->fixHoles();
//    msh->validate();
//
//    msh->clearNormals();
//    msh->calculateNormals();
//}
//
//
//int getShort(FILE *f) {
//    int ret = 0;
//    ret |= fgetc(f);
//    ret |= fgetc(f)<<8;
//    return ret;
//}
//
//int getShort(const unsigned char *&d) {
//    int ret = 0;
//    ret |= *d++;
//    ret |= (*d++)<<8;
//    return ret;
//}
//
//int getInt(FILE *f) {
//    int ret = 0;
//    ret |= fgetc(f);
//    ret |= fgetc(f)<<8;
//    ret |= fgetc(f)<<16;
//    ret |= fgetc(f)<<24;
//    return ret;
//}
//
//int getInt(const unsigned char *&d) {
//    int ret = 0;
//    ret |= *d++;
//    ret |= (*d++)<<8;
//    ret |= (*d++)<<16;
//    ret |= (*d++)<<24;
//    return ret;
//}
//
//float getFloat(FILE *f) {
//    float ret;
//    fread(&ret, 4, 1, f);
//    return ret;
//}
//
//float getFloat(const unsigned char *&d) {
//    float ret = *(const float*)d;
//    d+=4;
//    return ret;
//}
//
//int addPoint(IAMesh *IAMesh, float x, float y, float z)
//{
//    int i, n = (int)IAMesh->vertexList.size();
//    for (i = 0; i < n; ++i) {
//        IAVertex *v = IAMesh->vertexList[i];
//        if (   v->pPosition.x()==x
//            && v->pPosition.y()==y
//            && v->pPosition.z()==z)
//        {
//            return i;
//        }
//    }
//    IAVertex *v = new IAVertex();
//    v->pPosition.set(x, y, z);
//    IAMesh->vertexList.push_back(v);
//    return n;
//}
//
//
//// STL triangles ar CCW, normals are pointing outward
//void loadStl(const unsigned char *d) {
//    const float SCL = 30;
//    d+=0x50;
//    IAMesh *msh = new IAMesh();
//    gMeshList.push_back(msh);
//
//    int nFaces = getInt(d);
//    for (int i=0; i<nFaces; i++) {
//        float x, y, z;
//        int p1, p2, p3;
//        // face normal
//        getFloat(d);
//        getFloat(d);
//        getFloat(d);
//        // point 1
//        x = getFloat(d);
//        y = getFloat(d);
//        z = getFloat(d);
//        p1 = addPoint(msh, x*SCL, z*SCL, -y*SCL);
//        msh->vertexList[p1]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
//        // point 2
//        x = getFloat(d);
//        y = getFloat(d);
//        z = getFloat(d);
//        p2 = addPoint(msh, x*SCL, z*SCL, -y*SCL);
//        msh->vertexList[p2]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
//        // point 3
//        x = getFloat(d);
//        y = getFloat(d);
//        z = getFloat(d);
//        p3 = addPoint(msh, x*SCL, z*SCL, -y*SCL);
//        msh->vertexList[p3]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
//        // add face
//        IATriangle *t = new IATriangle();
//        t->pVertex[0] = msh->vertexList[p1];
//        t->pVertex[1] = msh->vertexList[p2];
//        t->pVertex[2] = msh->vertexList[p3];
//        msh->addFace(t);
//        // color
//
//        minX = min(minX, msh->vertexList[p1]->pPosition.x());
//        maxX = max(maxX, msh->vertexList[p1]->pPosition.x());
//        minY = min(minY, msh->vertexList[p1]->pPosition.y());
//        maxY = max(maxY, msh->vertexList[p1]->pPosition.y());
//        minZ = min(minZ, msh->vertexList[p1]->pPosition.z());
//        maxZ = max(maxZ, msh->vertexList[p1]->pPosition.z());
//
//        getShort(d);
//    }
//
//    msh->validate();
//    // TODO: fix seams
//    // TODO: fix zero size holes
//    // TODO: fix degenrate triangles
//    msh->fixHoles();
//    msh->validate();
//
//    msh->clearNormals();
//    msh->calculateNormals();
//}
//
//
///**
// Load a single node from a binary stl file.
// */
//void loadStl(const char *filename) {
//    int i;
//
//    FILE *f = fopen(filename, "rb");
//    if (!f) {
//        fprintf(stderr, "ERROR openening file!\n");
//        return;
//    }
//    fseek(f, 0x50, SEEK_SET);
//    IAMesh *msh = new IAMesh();
//    gMeshList.push_back(msh);
//
//    int nFaces = getInt(f);
//    for (i=0; i<nFaces; i++) {
//        float x, y, z;
//        int p1, p2, p3;
//        // face normal
//        getFloat(f);
//        getFloat(f);
//        getFloat(f);
//        // point 1
//        x = getFloat(f);
//        y = getFloat(f);
//        z = getFloat(f);
//        p1 = addPoint(msh, x, y, z);
//        // point 2
//        x = getFloat(f);
//        y = getFloat(f);
//        z = getFloat(f);
//        p2 = addPoint(msh, x, y, z);
//        // point 3
//        x = getFloat(f);
//        y = getFloat(f);
//        z = getFloat(f);
//        p3 = addPoint(msh, x, y, z);
//        // add face
//        IATriangle *t = new IATriangle();
//        t->pVertex[0] = msh->vertexList[p1];
//        t->pVertex[1] = msh->vertexList[p2];
//        t->pVertex[2] = msh->vertexList[p3];
//        msh->addFace(t);
//        // color
//        getShort(f);
//    }
//
//    msh->validate();
//    // TODO: fix seams
//    // TODO: fix zero size holes
//    // TODO: fix degenrate triangles
//#ifndef M_XYZ
//    msh->fixHoles();
//#endif
//    msh->validate();
//
//    msh->clearNormals();
//    msh->calculateNormals();
//
//    fclose(f);
//}
//
//
///**
// Load a model from a 3ds file.
// */
//void load3ds(const char *filename)
//{
//    Lib3dsFile *f = lib3ds_file_open(filename);
//    if (!f->nodes)
//        lib3ds_file_create_nodes_for_meshes(f);
//    lib3ds_file_eval(f, 0);
//    Lib3dsNode *p;
//    for (p = f->nodes; p; p = p->next) {
//        if (p->type == LIB3DS_NODE_MESH_INSTANCE) {
//            load3ds(f, (Lib3dsMeshInstanceNode*)p);
//        }
//    }
//    lib3ds_file_free(f);
//}
//
//
///**
// Load any image file as a texture.
// */
//void loadTexture(const char *filename)
//{
//    Fl_JPEG_Image *img = new Fl_JPEG_Image(filename);
//    if (img->d()) {
//        texture = img;
//    } else {
//        delete img;
//    }
//}
//
//void loadTexture(const char *name, unsigned char *imageDate)
//{
//    Fl_JPEG_Image *img = new Fl_JPEG_Image(name, imageDate);
//    if (img->d()) {
//        texture = img;
//    } else {
//        delete img;
//    }
//}
//
//
//static void xButtonCB(Fl_Widget*, void*)
//{
//    gShowSlice = false;
//    glView->redraw();
//}
//
//static void sliceCB(Fl_Widget*, void*)
//{
//    gMeshSlice.generateFrom(gMeshList, zSlider1->value());
//    // start a new slice. A slice holds the information from all meshes.
//    gMeshSlice.clear();
//    // get the number of meshes in this model
//    int i, n = (int)gMeshList.size();
//    // loop through all meshes
//    for (i=0; i<n; i++) {
//        IAMesh *IAMesh = gMeshList[i];
//        // add all faces in a mesh that intersect with zMin. They will form the lower lid.
//        gMeshSlice.addZSlice(*IAMesh, zSlider1->value());
//        // use OpenGL to convert the sorted list of edges into a list of simple polygons
//        gMeshSlice.tesselate();
//    }
//    glView->redraw();
//}
//
//static void z1ChangedCB(Fl_Widget*, void*)
//{
//    sliceCB(0, 0);
//    gShowSlice = true;
//    glView->redraw();
//}
//
//static void z2ChangedCB(Fl_Widget*, void*)
//{
//    gShowSlice = true;
//    glView->redraw();
//}
//
//static void writeSliceCB(Fl_Widget*, void*)
//{
//    double z;
//#ifdef M_MONKEY
//    double firstLayer  = -8.8;
//    double lastLayer   =  9.0;
//    double layerHeight =  0.1;
//    gOutFile = fopen("/Users/matt/monkey.3dp", "wb");
//#elif defined M_DRAGON
//    double firstLayer  = -20.0;
//    double lastLayer   =  14.0;
//    double layerHeight =   0.1;
//    gOutFile = fopen("/Users/matt/dragon.3dp", "wb");
//#elif defined M_XYZ
//    double firstLayer  =   0.0;
//    double lastLayer   =  14.0;
//    double layerHeight =   0.1;
//    gOutFile = fopen("/Users/matt/pepsi.3dp", "wb");
//#endif
//    // header
//    writeInt(gOutFile, 23);  // Magic
//    writeInt(gOutFile, 3);
//    writeInt(gOutFile, 2013);
//    writeInt(gOutFile, 1);   // File Version
//    writeInt(gOutFile, 159); // total number of layers
//    writeInt(gOutFile,  (lastLayer-firstLayer)/layerHeight );
//
//    for (z=firstLayer; z<=lastLayer; z+=layerHeight) {
//        // spread powder
//        writeInt(gOutFile, 158);
//        writeInt(gOutFile,  10); // spread 0.1mm layers
//        // render the layer
//        zSlider1->value(z);
//        zSlider1->do_callback();
//        gWriteSliceNext = 1;
//        glView->redraw();
//        glView->flush();
//        Fl::flush();
//    }
//    //  writeInt(gOutFile, 158);
//    //  writeInt(gOutFile,  25); // spread 0.25mm layers
//    //  writeInt(gOutFile, 158);
//    //  writeInt(gOutFile,  25); // spread 0.25mm layers
//    fclose(gOutFile);
//    fprintf(stderr, "/Users/matt/monkey.3dp");
//}
//
//
//static void writePrnSliceCB(Fl_Widget*, void*)
//{
//    double z;
//#ifdef M_MONKEY
//    double firstLayer  = -8.8;
//    double lastLayer   =  9.0;
//    double layerHeight =  0.1;
//#elif defined M_DRAGON
//    double firstLayer  = -20.0;
//    double lastLayer   =  14.0;
//    double layerHeight =   0.1;
//#elif defined M_XYZ
//    double firstLayer  =   0.0;
//    double lastLayer   =  14.0;
//    double layerHeight =   0.1;
//#endif
//
//    for (z=firstLayer; z<=lastLayer; z+=layerHeight) {
//        zSlider1->value(z);
//        zSlider1->do_callback();
//        gWriteSliceNext = 2;
//        glView->redraw();
//        glView->flush();
//        Fl::flush();
//    }
//    //  writeInt(gOutFile, 158);
//    //  writeInt(gOutFile,  25); // spread 0.25mm layers
//    //  writeInt(gOutFile, 158);
//    //  writeInt(gOutFile,  25); // spread 0.25mm layers
//    fclose(gOutFile);
//    fprintf(stderr, "/Users/matt/monkey.3dp");
//}
//
//
//int main (int argc, char **argv)
//{
//    /*
//     500 pixles at 96dpi = 5in = 13cm
//     at 96dpi, 1 dot is 0.26mm in diameter
//     */
//    loadTexture("testcard1024.jpg", defaultTexture);
//
//    Fl_Window win(840, 800, "Iota Slice");
//    win.begin();
//    Fl_Group *g = new Fl_Group(0, 0, 800, 800);
//    g->begin();
//    glView = new MyGLView(150, 150, 500, 500);
//    g->end();
//    zSlider1 = new Fl_Slider(800, 0, 20, 680);
//    zSlider1->tooltip("Position of the slice in Z\n-66 to +66 millimeters");
//    zSlider1->range(30, -30);
//    zSlider1->step(0.25);
//    zSlider1->callback(z1ChangedCB);
//    zSlider2 = new Fl_Slider(820, 0, 20, 680);
//    zSlider2->tooltip("Slice thickness\n-10 to +10 millimeters");
//    zSlider2->range(10, -10);
//    zSlider2->value(0.25);
//    zSlider2->callback(z2ChangedCB);
//    Fl_Button *b = new Fl_Button(800, 680, 40, 40, "X");
//    b->callback(xButtonCB);
//    b = new Fl_Button(800, 720, 40, 40, "Write");
//    b->callback(writeSliceCB);
//    b = new Fl_Button(800, 760, 40, 40, ".prn");
//    b->callback(writePrnSliceCB);
//    win.end();
//    win.resizable(g);
//    win.show(argc, argv);
//    glView->show();
//    Fl::flush();
//#ifdef M_MONKEY
//    load3ds("/Users/matt/Desktop/Machine Shop/Machine Pwdr/lib3ds-20080909/monkey.3ds");
//    //loadStl(defaultModel);
//#elif defined M_DRAGON
//    loadStl("/Users/matt/Desktop/Machine Shop/Machine Pwdr/0.02_dragon_2.stl");
//#elif defined M_XYZ
//    loadStl("/Users/matt/dev/IotaSlicer/data/xyz.stl");
//#endif
//    //load3ds("/Users/matt/squirrel/NewSquirrel.3ds");
//    //load3ds("/Users/matt/Desktop/Machine Shop/Machine Pwdr/0.02_dragon_2.3ds");
//    glView->redraw();
//    return Fl::run();
//}
//
////
//// End of "$Id: hello.cxx 11782 2016-06-14 11:15:22Z AlbrechtS $".
////
//

#endif /* MAIN_H */
