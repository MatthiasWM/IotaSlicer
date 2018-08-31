//
//  IASlice.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IASlice.h"

#include "Iota.h"
#include "IAMesh.h"
#include "userinterface/IAGUIMain.h"
#include "opengl/IAFramebuffer.h"

#include <FL/gl.h>
#include <FL/glu.h>


GLUtesselator *gGluTess = nullptr;


/**
 Create an emoty slice.
 */
IASlice::IASlice()
{
    pFramebuffer = new IAFramebuffer();
    pColorbuffer = new IAFramebuffer();
}


/**
 Free all resources.
 */
IASlice::~IASlice()
{
    clear();
    delete pFramebuffer;
    delete pColorbuffer;
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
    pFramebuffer->clear();
    pColorbuffer->clear();
    IAMesh::clear();
}


/**
 * Set the layer height for this slice.
 *
 * Clears all data if the current slice is different.
 *
 * \return true, if the layer actually changed
 */
bool IASlice::changeZ(double z)
{
    if (pCurrentZ==z) {
        // nothing changed, keep the data
        return false;
    } else {
        // layer
        pCurrentZ = z;
        clear();
        return true;
    }
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
void IASlice::addNextFlangeVertex(IATrianglePtr &t, ISVertexPtr &vCutA, int &edgeIndex)
{
    // setup
    double zMin = pCurrentZ;

    // find the other edge in the triangle that crosses Z. Faces are always clockwise
    // what happens if the triangle has one point exactly on Z?
    IAVertex *vOpp = t->pVertex[(edgeIndex+2)%3];
    int newIndex;
    if (vOpp->pGlobalPosition.z()<zMin) {
        newIndex = (edgeIndex+1)%3;
    } else {
        newIndex = (edgeIndex+2)%3;
    }

    // Cut the new edge at Z
    IAEdge *eCutB = t->pEdge[newIndex];
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
    t = eCutB->otherFace(t);
    edgeIndex = eCutB->indexIn(t);
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


/**
 * Draw the framebuffer as a slice into the current printer outline.
 */
void IASlice::drawFramebuffer()
{
#if 0 // TODO: temp hack
    if (pFramebuffer) {
        pFramebuffer->draw(pCurrentZ);
    }
#else
    if (pColorbuffer) {
        pColorbuffer->draw(pCurrentZ);
    }
#endif
}


/*
 Tesselation magic, better leave untouched.
 */
int tessVertexCount = 0;
IAVertex *tessV0, *tessV1, *tessV2;

#ifdef __APPLE__
#define __stdcall
#endif
#ifdef __LINUX__
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

    // FIXME: this does not belong here!
    pFramebuffer->bindForRendering(); // make sure we have a square in the buffer
    drawFlat(false, 1.0, 1.0, 0.0);
    pFramebuffer->unbindFromRendering();

    // TODO: temporary hack
    pColorbuffer->bindForRendering();
    drawShell();
    pColorbuffer->unbindFromRendering();
}


void IASlice::drawShell()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, gSceneView->tex);
    glEnable(GL_TEXTURE_2D);
    for (auto e: pFlange) {
        if (!e) continue;
        IAVertex *v0 = e->pVertex[0];
        IAVertex *v1 = e->pVertex[1];
        glBegin(GL_QUADS);
        glTexCoord2dv(v0->pTex.dataPointer());
        glVertex3dv((v0->pGlobalPosition+v0->pNormal).dataPointer());
        glVertex3dv((v0->pGlobalPosition-v0->pNormal*5).dataPointer());
        glTexCoord2dv(v1->pTex.dataPointer());
        glVertex3dv((v1->pGlobalPosition-v1->pNormal*5).dataPointer());
        glVertex3dv((v1->pGlobalPosition+v1->pNormal).dataPointer());
        glEnd();
    }
}


