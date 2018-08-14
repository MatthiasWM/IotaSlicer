//
//  IASlice.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IASlice.h"

#include "main.h"
#include "IAMesh.h"

#include <FL/gl.h>
#include <FL/glu.h>

GLUtesselator *gGluTess = nullptr;


IASlice::IASlice()
{
}

IASlice::~IASlice()
{
    clear();
}

void IASlice::generateFrom(IAMeshList &meshList, double z)
{
    // start a new slice. A slice holds the information from all meshes.
    clear();
    // get the number of meshes in this model
    int i, n = (int)meshList.size();
    // loop through all meshes
    for (i=0; i<n; i++) {
        IAMesh *IAMesh = meshList[i];
        // add all faces in a mesh that intersect with zMin. They will form the lower lid.
        addZSlice(*IAMesh, z);
        // use OpenGL to convert the sorted list of edges into a list of simple polygons
        tesselate();
    }
}

void IASlice::clear()
{
    int i, n = (int)pLid.size();
    for (i=0; i<n; i++) {
        delete pLid[i];
    }
    pLid.clear();
    IAMesh::clear();
}

void IASlice::drawLidEdge()
{
    int i, j, n = (int)pLid.size();
    glColor3f(0.8f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    for (i = 0; i < n; i++) {
        IAEdge *IAEdge = pLid[i];
        if (IAEdge) {
            for (j = 0; j < 2; ++j) {
                IAVertex *IAVertex = IAEdge->pVertex[j];
                if (IAVertex) {
                    glTexCoord2dv(IAVertex->pTex.dataPointer());
                    glVertex3dv(IAVertex->pPosition.dataPointer());
                }
            }
        }
    }
    glEnd();
}


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
        gMeshSlice.addFace(f);
        tessVertexCount = 0;
    }
}

void __stdcall tessCombineCallback(GLdouble coords[3],
                         IAVertex *vertex_data[4],
                         GLfloat weight[4], IAVertex **dataOut )
{
    IAVertex *v = new IAVertex();
    v->pPosition.read(coords);
    gMeshSlice.vertexList.push_back(v);
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

// http://www.cs.man.ac.uk/~toby/alan/software/
// http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml
// https://github.com/greenm01/poly2tri
void IASlice::tesselate()
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
    
    int i, n = (int)pLid.size();
    tessVertexCount = 0;
    gluTessBeginPolygon(gGluTess, this);
    gluTessBeginContour(gGluTess);
    for (i=0; i<n; i++) {
        IAEdge *e = pLid[i];
        if (e==NULL) {
            gluTessEndContour(gGluTess);
            gluTessBeginContour(gGluTess);
        } else {
            gluTessVertex(gGluTess, e->pVertex[0]->pPosition.dataPointer(), e->pVertex[0]);
        }
    }
    gluTessEndContour(gGluTess);
    gluTessEndPolygon(gGluTess);
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
void IASlice::addNextLidVertex(IATrianglePtr &IATriangle, ISVertexPtr &vCutA, int &edgeIndex, double zMin)
{
    // faces are always clockwise
    IAVertex *vOpp = IATriangle->pVertex[(edgeIndex+2)%3];
    int newIndex;
    if (vOpp->pPosition.z()<zMin) {
        newIndex = (edgeIndex+1)%3;
    } else {
        newIndex = (edgeIndex+2)%3;
    }
    IAEdge *eCutB = IATriangle->pEdge[newIndex];
    IAVertex *vCutB = eCutB->findZ(zMin);
    if (!vCutB) {
        puts("ERROR: addNextLidVertex failed, no Z point found!");
    }
    vertexList.push_back(vCutB);
    IAEdge *lidEdge = new IAEdge();
    lidEdge->pVertex[0] = vCutA;
    lidEdge->pVertex[1] = vCutB;
    pLid.push_back(lidEdge);
    
    vCutA = vCutB;
    IATriangle = eCutB->otherFace(IATriangle);
    edgeIndex = eCutB->indexIn(IATriangle);
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
void IASlice::addFirstLidVertex(IATriangle *tri, double zMin)
{
    IATriangle *firstFace = tri;
    // find first edge that crosses zMin
    int edgeIndex = -1;
    if (tri->pVertex[0]->pPosition.z()<zMin && tri->pVertex[1]->pPosition.z()>=zMin) edgeIndex = 0;
    if (tri->pVertex[1]->pPosition.z()<zMin && tri->pVertex[2]->pPosition.z()>=zMin) edgeIndex = 1;
    if (tri->pVertex[2]->pPosition.z()<zMin && tri->pVertex[0]->pPosition.z()>=zMin) edgeIndex = 2;
    if (edgeIndex==-1) {
        puts("ERROR: addFirstLidVertex failed, not crossing zMin!");
    }
    IAVertex *vCutA = tri->pEdge[edgeIndex]->findZ(zMin);
    if (!vCutA) {
        puts("ERROR: addFirstLidVertex failed, no Z point found!");
    }
    vertexList.push_back(vCutA);
    //  addNextLidVertex(IATriangle, vCutA, edgeIndex, zMin);
    int cc = 0;
    for (;;) {
        addNextLidVertex(tri, vCutA, edgeIndex, zMin);
        cc++;
        if (tri->pUsed)
            break;
        tri->pUsed = true;
    }
    printf("%d edges linked\n", cc);
    if (firstFace==tri) {
        puts("It's a loop!");
    } else {
        puts("It's NOT a loop!");
    }
    pLid.push_back(0L);
}

void IASlice::addZSlice(const IAMesh &m, double zMin)
{
    // Get the number of triangles in this mesh
    int i, n = (int)m.faceList.size();
    // run through all faces and mark them as unused
    for (i = 0; i < n; i++) {
        m.faceList[i]->pUsed = false;
    }
    // run through all faces again and add all faces to the first lid that intersect with zMin
    for (i = 0; i < n; i++) {
        IATriangle *IATriangle = m.faceList[i];
        if (IATriangle->pUsed) continue;
        IATriangle->pUsed = true;
        int nBelow = IATriangle->pointsBelowZ(zMin);
        if (nBelow==0) {
            // do nothing, all vertices are above zMin
        } else if (nBelow==1) {
            // starting from here, find all faces that intersect zMin and generate an outline for this slice
            addFirstLidVertex(IATriangle, zMin);
        } else if (nBelow==2) {
            // starting from here, find all faces that intersect zMin and generate an outline for this slice
            addFirstLidVertex(IATriangle, zMin);
        } else if (nBelow==3) {
            // do nothing, all vertices are below zMin
        }
    }
}


