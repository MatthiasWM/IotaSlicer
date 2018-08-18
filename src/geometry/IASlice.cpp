//
//  IASlice.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IASlice.h"

#include "../Iota.h"
#include "IAMesh.h"
#include "../userinterface/IAGUIMain.h"

#include <FL/gl.h>
#include <FL/glu.h>

GLUtesselator *gGluTess = nullptr;


/**
 Create an emoty slice.
 */
IASlice::IASlice()
{
}


/**
 Free all resources.
 */
IASlice::~IASlice()
{
    clear();
}


/**
 Free all resources.
 */
void IASlice::clear()
{
    for (auto e: pFlange) { // pLid is an edge list
        delete e;
    }
    pFlange.clear();
    IAMesh::clear();
}


/**
 Create the outline of a lid by slicing all meshes at Z.
 */
void IASlice::generateFlange(IAMesh *mesh)
{
    clear();
    addFlange(mesh);
}


/**
 Create an edge list where the slice intersects with the mesh.
 The egde list runs clockwise for a connected outline, and counterclockwise for
 holes. Every outline loop can followed by a null ptr and more outlines.
 */
void IASlice::addFlange(IAMesh *m)
{
    // setup
    m->updateGlobalSpace();
    double zMin = pCurrentZ;

    // run through all faces and mark them as unused
    for (auto t: m->faceList) {
        t->pUsed = false;
    }

    // run through all faces again and add all faces to the first lid that intersect with zMin
    for (auto t: m->faceList) {
        if (t->pUsed) continue;
        t->pUsed = true;
        int nBelow = t->pointsBelowZGlobal(zMin);
        if (nBelow==0) {
            // do nothing, all vertices are above zMin
        } else if (nBelow==1) {
            // starting from here, find all faces that intersect zMin and generate an outline for this slice
            addFirstFlangeVertex(t);
        } else if (nBelow==2) {
            // starting from here, find all faces that intersect zMin and generate an outline for this slice
            addFirstFlangeVertex(t);
        } else if (nBelow==3) {
            // do nothing, all vertices are below zMin
        }
    }
}


/*
 Create the edge that cuts this triangle in half.

 The first point is know to be on the z slice. The second edge that crosses
 z is found and the point of intersection is calculated. Then an edge is
 created that splits the face on the z plane.

 \param IATriangle the face that is split in two; the face must cross zMin
 \param vCutA the first point on zMin along the first edge
 \param edgeIndex the index of the first edge that crosses zMin
 \param zMin slice on this z plane
 */
void IASlice::addFirstFlangeVertex(IATriangle *tri)
{
    // setup
    double zMin = pCurrentZ;
    IATriangle *firstFace = tri;

    // find first edge that crosses Z
    int edgeIndex = -1;
    if (tri->pVertex[0]->pGlobalPosition.z()<zMin && tri->pVertex[1]->pGlobalPosition.z()>=zMin) edgeIndex = 0;
    if (tri->pVertex[1]->pGlobalPosition.z()<zMin && tri->pVertex[2]->pGlobalPosition.z()>=zMin) edgeIndex = 1;
    if (tri->pVertex[2]->pGlobalPosition.z()<zMin && tri->pVertex[0]->pGlobalPosition.z()>=zMin) edgeIndex = 2;
    if (edgeIndex==-1) {
        puts("ERROR: addFirstLidVertex failed, not crossing zMin!");
    }

    // create a vertex where the edge crosses Z.
    IAVertex *vCutA = tri->pEdge[edgeIndex]->findZGlobal(zMin);
    if (!vCutA) {
        puts("ERROR: addFirstLidVertex failed, no Z point found!");
    }

    // add this edge to list
    vertexList.push_back(vCutA);

    // find more connected edges
    int cc = 0;
    for (;;) {
        addNextFlangeVertex(tri, vCutA, edgeIndex);
        cc++;
        if (tri->pUsed)
            break;
        tri->pUsed = true;
    }

    // some statistics (should always be a loop if the model is watertight
    //    printf("%d edges linked\n", cc);
    if (firstFace==tri) {
        //        puts("It's a loop!");
    } else {
        //        puts("It's NOT a loop!");
    }

    // mark the end of an edge list, start with a hole or separate mesh
    pFlange.push_back(0L);
}


/*
 Create the edge that cuts this triangle in half.

 The first point is know to be on the z slice. The second edge that crosses
 z is found and the point of intersection is calculated. Then an edge is
 created that splits the face on the z plane.

 \param IATriangle the face that is split in two; the face must cross zMin
 \param vCutA the first point on zMin along the first edge
 \param edgeIndex the index of the first edge that crosses zMin
 \param zMin slice on this z plane
 */
void IASlice::addNextFlangeVertex(IATrianglePtr &IATriangle, ISVertexPtr &vCutA, int &edgeIndex)
{
    // setup
    double zMin = pCurrentZ;

    // find the other edge in the triangle that crosses Z. Faces are always clockwise
    // what happens if the triangle has one point exactly on Z?
    IAVertex *vOpp = IATriangle->pVertex[(edgeIndex+2)%3];
    int newIndex;
    if (vOpp->pGlobalPosition.z()<zMin) {
        newIndex = (edgeIndex+1)%3;
    } else {
        newIndex = (edgeIndex+2)%3;
    }

    // Cut the new edge at Z
    IAEdge *eCutB = IATriangle->pEdge[newIndex];
    IAVertex *vCutB = eCutB->findZGlobal(zMin);
    if (!vCutB) {
        puts("ERROR: addNextLidVertex failed, no Z point found!");
    }
    vertexList.push_back(vCutB);
    IAEdge *lidEdge = new IAEdge();
    lidEdge->pVertex[0] = vCutA;
    lidEdge->pVertex[1] = vCutB;
    pFlange.push_back(lidEdge);

    vCutA = vCutB;
    IATriangle = eCutB->otherFace(IATriangle);
    edgeIndex = eCutB->indexIn(IATriangle);
}


/**
 Draw the edge where the slice intersects the model.
 */
void IASlice::drawFlange()
{
    glColor3f(0.8f, 1.0f, 1.0f);
    glLineWidth(12.0);
    glBegin(GL_LINES);
    for (auto e: pFlange) {
        if (e) {
            for (int j = 0; j < 2; ++j) {
                IAVertex *v = e->pVertex[j];
                if (v) {
                    glTexCoord2dv(v->pTex.dataPointer());
                    glVertex3dv(v->pLocalPosition.dataPointer());
                }
            }
        }
    }
    glEnd();
    glLineWidth(1.0);
}


/*
 Tesselation magic, better leave untouched.
 */
int tessVertexCount = 0;
IAVertex *tessV0, *tessV1, *tessV2;

#ifdef __APPLE__
#define __stdcall
#endif

void __stdcall tessBeginCallback(GLenum which)
{
    tessVertexCount = 0;
}

void __stdcall tessEndCallback()
{
    // tessVertexCount must be 0!
}

void __stdcall tessVertexCallback(GLvoid *vertex)
{
    if (tessVertexCount==0) {
        tessV0 = (IAVertex*)vertex;
        tessVertexCount = 1;
    } else if (tessVertexCount==1) {
        tessV1 = (IAVertex*)vertex;
        tessVertexCount = 2;
    } else {
        tessV2 = (IAVertex*)vertex;
        IATriangle *f = new IATriangle();
        f->pVertex[0] = tessV0;
        f->pVertex[1] = tessV1;
        f->pVertex[2] = tessV2;
        Iota.gMeshSlice.addFace(f);
        tessVertexCount = 0;
    }
}

void __stdcall tessCombineCallback(GLdouble coords[3],
                         IAVertex *vertex_data[4],
                         GLfloat weight[4], IAVertex **dataOut )
{
    IAVertex *v = new IAVertex();
    v->pLocalPosition.read(coords);
    // TODO: or used mesh.addVertex()? It would save space, but be a bit slower.
    Iota.gMeshSlice.vertexList.push_back(v);
    *dataOut = v;
}

void __stdcall tessEdgeFlagCallback(GLboolean flag)
{
}

void __stdcall tessErrorCallback(GLenum errorCode)
{
    const GLubyte *estring;
    estring = gluErrorString(errorCode);
    fprintf (stderr, "Tessellation Error: %s\n", estring);
}


/**
 Fill the sliced outline with triangles, considering complex polygons and holes.

 This call requires a flange, so you must call generateFlange() first.

 \todo Glu's tesselation calls are deprecated. Please find a library:
 \todo http://www.cs.man.ac.uk/~toby/alan/software/
 \todo http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml
 \todo https://github.com/greenm01/poly2tri
 */
void IASlice::tesselateLidFromFlange()
{
    if (!gGluTess)
        gGluTess = gluNewTess();
    
#ifdef __APPLE__
	gluTessCallback(gGluTess, GLU_TESS_VERTEX, (GLvoid(*) ()) &tessVertexCallback);
	gluTessCallback(gGluTess, GLU_TESS_BEGIN, (GLvoid (*) ()) &tessBeginCallback);
    gluTessCallback(gGluTess, GLU_TESS_END, (GLvoid (*) ()) &tessEndCallback);
    gluTessCallback(gGluTess, GLU_TESS_ERROR, (GLvoid (*) ()) &tessErrorCallback);
    gluTessCallback(gGluTess, GLU_TESS_COMBINE, (GLvoid (*) ()) &tessCombineCallback);
    gluTessCallback(gGluTess, GLU_TESS_EDGE_FLAG, (GLvoid (*) ()) &tessEdgeFlagCallback);
#else
	gluTessCallback(gGluTess, GLU_TESS_VERTEX, (void(__stdcall*)()) &tessVertexCallback);
	gluTessCallback(gGluTess, GLU_TESS_BEGIN, (void(__stdcall*)()) &tessBeginCallback);
	gluTessCallback(gGluTess, GLU_TESS_END, (void(__stdcall*)()) &tessEndCallback);
	gluTessCallback(gGluTess, GLU_TESS_ERROR, (void(__stdcall*)()) &tessErrorCallback);
	gluTessCallback(gGluTess, GLU_TESS_COMBINE, (void(__stdcall*)()) &tessCombineCallback);
	gluTessCallback(gGluTess, GLU_TESS_EDGE_FLAG, (void(__stdcall*)()) &tessEdgeFlagCallback);
#endif
    gluTessProperty(gGluTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);
    
    int i, n = (int)pFlange.size();
    tessVertexCount = 0;
    gluTessBeginPolygon(gGluTess, this);
    gluTessBeginContour(gGluTess);
    for (i=0; i<n; i++) {
        IAEdge *e = pFlange[i];
        if (e==NULL) {
            gluTessEndContour(gGluTess);
            gluTessBeginContour(gGluTess);
        } else {
            gluTessVertex(gGluTess, e->pVertex[0]->pLocalPosition.dataPointer(), e->pVertex[0]);
        }
    }
    gluTessEndContour(gGluTess);
    gluTessEndPolygon(gGluTess);
}


#if 0

#include <stdio.h>
//#include "machine.h"
//#include "types.h"
//#include "util.h"
//#include "imageio.h" /* error codes etc */
//#include "jpeg.h"    /* the protos for this file */

#include "jpeglib.h" /* the IJG jpeg library headers */



int writejpeg(char * name, int xres, int yres, unsigned char *imgdata) {
    FILE *ofp;
    struct jpeg_compress_struct cinfo;   /* JPEG compression struct */
    struct jpeg_error_mgr jerr;          /* JPEG error handler */
    JSAMPROW row_pointer[1];             /* output row buffer */
    int row_stride;                      /* physical row width in output buf */

    if ((ofp = fopen(name, "wb")) == NULL) {
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, ofp);

    cinfo.image_width = xres;
    cinfo.image_height = yres;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
//    jpeg_set_quality(<#j_compress_ptr cinfo#>, <#int quality#>, <#boolean force_baseline#>)
//    jpeg_set_quality(&cinfo, 95, false);

    jpeg_start_compress(&cinfo, TRUE);

    /* Calculate the size of a row in the image */
    row_stride = cinfo.image_width * cinfo.input_components;

    /* compress the JPEG, one scanline at a time into the buffer */
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &(imgdata[(yres - cinfo.next_scanline - 1)*row_stride]);
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fclose(ofp);

    return 0; /* No fatal errors */
}

#endif

extern "C" int potrace_main(unsigned char *pixels256x256);


void IASlice::save(double z, const char *filename)
{
#ifdef __APPLE__ // VisualStudio 2017 misses a lot of the OpenGL names that I used here. Must investigate.
//    const int w = 800, h = 600;

    // https://www.khronos.org/opengl/wiki/Framebuffer_Object_Extension_Examples#Quick_example.2C_render_to_texture_.282D.29

    GLuint color_tex, fb, depth_rb;
    //RGBA8 2D texture, 24 bit depth texture, 256x256
    glGenTextures(1, &color_tex);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //NULL means reserve texture memory, but texels are undefined
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    //-------------------------
    glGenFramebuffersEXT(1, &fb);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    //Attach 2D texture to this FBO
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, color_tex, 0);
    //-------------------------
    glGenRenderbuffersEXT(1, &depth_rb);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_rb);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, 256, 256);
    //-------------------------
    //Attach depth buffer to FBO
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_rb);
    //-------------------------
    //Does the GPU support current FBO configuration?
    GLenum status;
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status)
    {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            printf("good\n");
            break;
        default:
            printf("not so good\n");
            return;
    }
    //-------------------------
    //and now you can render to GL_TEXTURE_2D
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //-------------------------
    glViewport(0, 0, 256, 256);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-100, 100, -100, 100, -200.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //-------------------------
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    //-------------------------
    //**************************
    //RenderATriangle, {0.0, 0.0}, {256.0, 0.0}, {256.0, 256.0}
    //Read http://www.opengl.org/wiki/VBO_-_just_examples
//    glColor3f(1.0, 0.5, 0.5);
//    glBegin(GL_POLYGON);
//    glVertex3f(-10.0, -10.0, 0.0);
//    glVertex3f(-10.0,  10.0, 0.0);
//    glVertex3f( 10.0,  10.0, 0.0);
//    glVertex3f( 10.0, -10.0, 0.0);
//    glEnd();
    this->drawFlange();
    // render...

    //    glGetTexImage(<#GLenum target#>, <#GLint level#>, <#GLenum format#>, <#GLenum type#>, <#GLvoid *pixels#>);
    //    glReadPixels(<#GLint x#>, <#GLint y#>, <#GLsizei width#>, <#GLsizei height#>, <#GLenum format#>, <#GLenum type#>, <#GLvoid *pixels#>)
    //-------------------------
    GLubyte pixels[256*256*4];
    glReadPixels(0, 0, 256, 256, GL_BGR, GL_UNSIGNED_BYTE, pixels);
    //pixels 0, 1, 2 should be white
    //pixel 4 should be black

//    writejpeg((char*)filename, 256, 256, (unsigned char *)pixels);

    potrace_main((unsigned char *)pixels);


    //----------------
    //Bind 0, which means render to back buffer
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    //Delete resources
    glDeleteTextures(1, &color_tex);
    glDeleteRenderbuffersEXT(1, &depth_rb);
    //Bind 0, which means render to back buffer, as a result, fb is unbound
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glDeleteFramebuffersEXT(1, &fb);

    gSceneView->valid(0);


//    FILE *f = fopen(filename, "wb");
//    fclose(f);
#endif
}


