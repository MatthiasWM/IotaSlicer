//
//  IASlice.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IASlice.h"

#include "Iota.h"
#include "IAMesh.h"
#include "view/IAGUIMain.h"
#include "opengl/IAFramebuffer.h"

#include <FL/gl.h>
#include <FL/glu.h>

#ifdef __APPLE__
// suppress warnings that GLU tesselation is deprecated
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif


GLUtesselator *gGluTess = nullptr;


/**
 Create an emoty slice.
 */
IASlice::IASlice(IAPrinter *printer)
:   pPrinter( printer )
{
#if 0
    pFramebuffer = new IAFramebuffer(printer, IAFramebuffer::RGBA);
#else
    pFramebuffer = new IAFramebuffer(printer, IAFramebuffer::BITMAP);
#endif
    pColorbuffer = new IAFramebuffer(printer, IAFramebuffer::RGBAZ);
}


/**
 Free all resources.
 */
IASlice::~IASlice()
{
    for (auto &e: pRim) { // pLid is an edge list
        delete e;
    }
    pRim.clear();
    delete pFramebuffer;
    delete pColorbuffer;
}


/**
 Free all resources.
 */
void IASlice::clear()
{
    for (auto &e: pRim) { // pLid is an edge list
        delete e;
    }
    pRim.clear();
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
bool IASlice::setNewZ(double z)
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
void IASlice::generateRim(IAMesh *mesh)
{
    clear();
    addRim(mesh);
}


/**
 Create an edge list where the slice intersects with the mesh.
 The egde list runs clockwise for a connected outline, and counterclockwise for
 holes. Every outline loop can followed by a null ptr and more outlines.
 */
void IASlice::addRim(IAMesh *m)
{
    // setup
    m->updateGlobalSpace();

    // this is a pretty daft hack. To avoid boundary cases, we test if any of
    // the model's z coordinates are exactly equal to the slicing plane. If they
    // are, we move the z plane a tiny bit and try again.
    double oldZ = pCurrentZ;
    bool done = true;
    do {
        done = true;
        for (auto &v: m->vertexList) {
            if (v->pGlobalPosition.z()==pCurrentZ) {
                pCurrentZ += 1e-7;
                done = false;
                break;
            }
        }
    } while (!done);

    // run through all faces and mark them as unused
    for (auto &t: m->triangleList) {
        t->pUsed = false;
    }

    // run through all faces again and add all faces to the first lid that intersect with zMin
    for (auto &t: m->triangleList) {
        if (t->pUsed) continue;
        t->pUsed = true;
        if (t->crossesZGlobal(pCurrentZ))
            addFirstRimVertex(t);
    }

    // restore the old setup
    pCurrentZ = oldZ;
}


/**
 * Create the edge that cuts this triangle in half.
 *
 * This finds one half-edge in the triangle that crosses z. It then
 * calls addNextRimVertex which will run along the mesh to find the next
 * edge that crosses z, and so on until we reach the starting triangle again.
 *
 * In a watertight mesh, this should always create a loop.
 *
 * \param t starting triangle.
 *
 * \todo handle cases where a point is exactly on z
 * \todo if addNextRimVertex failed because this is not a watertight model (or
 *       something else went wrong) we still may save the day somewhat by tracing
 *       the flange in the other direction. Either way, the result is
 *       pretty random.
 */
void IASlice::addFirstRimVertex(IATriangle *t)
{
    // case 1: z crosses two edges
    // case 2: z touches one vertex and crosses one edge
    // case 3: z touches one vertex and crosses no edge
    //      triangle must be above z
    // case 3: z touches two vertices
    //      triangle must be below z
    // case 4: z touches all vertices (triangle is coplanar to z)
    //      crossesZGlobal() returns false, ignored here

    double z = pCurrentZ;
    IATriangle *firstTriangle = t;

    IAVector3d &v0 = t->vertex(0)->pGlobalPosition;
    IAVector3d &v1 = t->vertex(1)->pGlobalPosition;
    IAVector3d &v2 = t->vertex(2)->pGlobalPosition;

    if (v0.z()==z && v1.z()==z && v2.z()==z) return; // case 4, defensive

    IAHalfEdge *e = nullptr;
    if ( (v0.z()<z) && (v1.z()>z) ) {
        e = t->edge(0); // case 1
    } else if ( (v1.z()<z) && (v2.z()>z) ) {
        e = t->edge(1); // case 1
    } else if ( (v2.z()<z) && (v0.z()>z) ) {
        e = t->edge(2); // case 1
    } else {
        // boundary condition
        puts("ERROR: addFirstRimVertex boundary condition not implemented!");
        assert(0);
    }

    IAVertex *vCutA = e->findZGlobal(z);
    if (!vCutA) {
        puts("ERROR: addFirstRimVertex failed, no Z point found!");
        assert(0);
    }
    vertexList.push_back(vCutA);

    // find more connected edges
    for (;;) {
        if (!addNextRimVertex(e))
            break;
        t = e->triangle();
        if (t->pUsed)
            break;
        t->pUsed = true;
    }

    if (firstTriangle==t) {
        // puts("It's a loop!");
    } else {
        puts("WARNING: the rim of the slice is not a loop. Model not watertight?");
    }

    // mark the end of an edge list, start with a hole or separate mesh
    pRim.push_back(0L);
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
bool IASlice::addNextRimVertex(IAHalfEdgePtr &e)
{
    // case 1: z crosses two edges
    // case 2: z touches one vertex and crosses one edge
    // case 3: z touches one vertex and crosses no edge
    //      triangle must be above z
    // case 3: z touches two vertices
    //      triangle must be below z
    // case 4: z touches all vertices (triangle is coplanar to z)
    //      triangle is coplanar to z

    // find the other edge in the triangle that crosses Z. Triangles are always clockwise
    // what happens if the triangle has one point exactly on Z?
    /** \todo  >, >= ?? */
    if (e->prev()->vertex()->pGlobalPosition.z()<pCurrentZ) {
        e = e->next();
    } else {
        e = e->prev();
    }

    // Cut the new edge at Z
    IAVertex *vCutB = e->findZGlobal(pCurrentZ);
    if (!vCutB) {
        puts("ERROR: addNextLidVertex failed, no Z point found!");
        assert(0);
    }

    IAEdge *lidEdge = new IAEdge();
    lidEdge->pVertex[0] = vertexList.back();
    lidEdge->pVertex[1] = vCutB;
    vertexList.push_back(vCutB);
    pRim.push_back(lidEdge);

    if (!e->twin())
        return false;

    e = e->twin();
    return true;
}


/**
 Draw the edge where the slice intersects the model.
 */
void IASlice::drawRim()
{
    glColor3f(0.8f, 1.0f, 1.0f);
    glLineWidth(12.0);
    glBegin(GL_LINES);
    for (auto &e: pRim) {
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
#if 0 // temp hack
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
static int tessVertexCount = 0;
static IAVertex *tessV0, *tessV1, *tessV2;
static IASlice *currentSlice = nullptr;

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
        currentSlice->addNewTriangle(tessV0, tessV1, tessV2);
        tessVertexCount = 0;
    }
}

void __stdcall tessCombineCallback(GLdouble coords[3],
                         IAVertex *vertex_data[4],
                         GLfloat weight[4], IAVertex **dataOut )
{
    IAVertex *v = new IAVertex();
    v->pLocalPosition.read(coords);
    // or used mesh.addVertex()? It would save space, but be a bit slower.
    currentSlice->vertexList.push_back(v);
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

 This call requires a flange, so you must call generateRim() first.

 \todo Glu's tesselation calls are deprecated. Please find a library:
 \todo http://www.cs.man.ac.uk/~toby/alan/software/
 \todo http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml
 \todo https://github.com/greenm01/poly2tri
 \todo drawFlat should not be called here!
 \todo drawShell should not be called here!
 */
void IASlice::tesselateLidFromRim()
{
    currentSlice = this;
    if (!gGluTess)
        gGluTess = gluNewTess();

//    gluTessProperty(tess, GLU_TESS_BOUNDARY_ONLY, GL_TRUE);
//    gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

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

    int i, n = (int)pRim.size();
    tessVertexCount = 0;
    gluTessBeginPolygon(gGluTess, this);
    gluTessBeginContour(gGluTess);
    for (i=0; i<n; i++) {
        IAEdge *e = pRim[i];
        if (e==NULL) {
            gluTessEndContour(gGluTess);
            gluTessBeginContour(gGluTess);
        } else {
            gluTessVertex(gGluTess, e->pVertex[0]->pLocalPosition.dataPointer(), e->pVertex[0]);
        }
    }
    gluTessEndContour(gGluTess);
    gluTessEndPolygon(gGluTess);

    currentSlice = nullptr;
}

/**
 * Tesselate the rim and draw the resulting lid.
 */
void IASlice::tesselateAndDrawLid()
{
    pFramebuffer->bindForRendering(); // make sure we have a square in the buffer
    if (pFramebuffer->buffers()==IAFramebuffer::BITMAP) {
        pFramebuffer->drawLid(pRim);
    } else {
        tesselateLidFromRim();
        drawFlat(false, 1.0, 1.0, 0.0);
        pFramebuffer->unbindFromRendering();
    }
}


void IASlice::drawShell()
{
//    /// \bug temporary hack
//    pColorbuffer->bindForRendering();
//    drawShell();
//    pColorbuffer->unbindFromRendering();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, gSceneView->tex);
    glEnable(GL_TEXTURE_2D);
    for (auto &e: pRim) {
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


#ifdef __APPLE__
#pragma clang diagnostic pop
#endif

