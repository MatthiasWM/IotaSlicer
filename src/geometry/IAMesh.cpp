//
//  IAMesh.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAMesh.h"

#include "IAEdge.h"

#include <FL/fl_draw.H>
#include <FL/gl.h>
#include <FL/glu.h>


/**
 Create an empty mesh.
 */
IAMesh::IAMesh()
{
}


/**
 Clear all resources used by the mesh.
 */
void IAMesh::clear()
{
    for (auto e: edgeList) {
        delete e;
    }
    edgeList.clear();

    for (auto f: faceList) {
        delete f;
    }
    faceList.clear();

    for (auto v: vertexList) {
        delete v;
    }
    vertexList.clear();
}


/**
 Move all vertices along the negative point normal, effectively shrining the mesh.
 */
void IAMesh::shrinkBy(double s)
{
    for (auto v: vertexList) {
        v->shrinkBy(s);
    }
}


/**
 Various test that validate a watertight triangle mesh.
 \todo This was written a long time ago and must be verified.
 */
bool IAMesh::validate()
{
    if (faceList.size()>0 && edgeList.size()==0) {
        puts("ERROR: empty edge list!");
    }
    int i, n = (int)edgeList.size();
    for (i=0; i<n; i++) {
        IAEdge *e = edgeList[i];
        if (e) {
            if (e->pFace[0]==0L) {
                printf("ERROR: edge %d [%p] without face found!\n", i, e);
            } else if (e->pFace[1]==0L) {
                printf("ERROR: edge %d [%p] with single face found (hole in mesh)!\n", i, e);
            }
            if (e->pFace[0]) {
                if (e->pFace[0]->pEdge[0]!=e && e->pFace[0]->pEdge[1]!=e && e->pFace[0]->pEdge[2]!=e) {
                    printf("ERROR: face [%p] is not pointing back at edge %d [%p]!\n", e->pFace[0], i, e);
                }
            }
            if (e->pFace[1]) {
                if (e->pFace[1]->pEdge[0]!=e && e->pFace[1]->pEdge[1]!=e && e->pFace[1]->pEdge[2]!=e) {
                    printf("ERROR: face [%p] is not pointing back at edge %d [%p]!\n", e->pFace[1], i, e);
                }
            }
            if (e->pVertex[0]==0L || e->pVertex[1]==0L) {
                printf("ERROR: edge %d [%p] missing a vertex reference!\n", i, e);
            }
        } else {
            puts("ERROR: zero edge found!");
        }
    }
    n = (int)faceList.size();
    for (i=0; i<n; i++) {
        IATriangle *f = faceList[i];
        if (f) {
            if (f->pVertex[0]==0L || f->pVertex[1]==0L || f->pVertex[1]==0L) {
                printf("ERROR: face %d has an empty vertex field.\n", i);
            }
            if (f->pEdge[0]==0L || f->pEdge[1]==0L || f->pEdge[1]==0L) {
                printf("ERROR: face %d has an empty edge field.\n", i);
            } else {
                if (f->pEdge[0]->vertex(0, f)!=f->pVertex[0])
                    printf("ERROR: face %d has an edge0/vertex0 missmatch.\n", i);
                if (f->pEdge[0]->vertex(1, f)!=f->pVertex[1])
                    printf("ERROR: face %d has an edge0/vertex1 missmatch.\n", i);
                if (f->pEdge[1]->vertex(0, f)!=f->pVertex[1])
                    printf("ERROR: face %d has an edge1/vertex1 missmatch.\n", i);
                if (f->pEdge[1]->vertex(1, f)!=f->pVertex[2])
                    printf("ERROR: face %d has an edge1/vertex2 missmatch.\n", i);
                if (f->pEdge[2]->vertex(0, f)!=f->pVertex[2])
                    printf("ERROR: face %d has an edge2/vertex2 missmatch.\n", i);
                if (f->pEdge[2]->vertex(1, f)!=f->pVertex[0])
                    printf("ERROR: face %d has an edge2/vertex0 missmatch.\n", i);
                if (f->pEdge[0]->pFace[0]!=f && f->pEdge[0]->pFace[1]!=f)
                    printf("ERROR: face %d edge0 does not point back at face.\n", i);
                if (f->pEdge[1]->pFace[0]!=f && f->pEdge[1]->pFace[1]!=f)
                    printf("ERROR: face %d edge1 does not point back at face.\n", i);
                if (f->pEdge[2]->pFace[0]!=f && f->pEdge[2]->pFace[1]!=f)
                    printf("ERROR: face %d edge2 does not point back at face.\n", i);
            }
        } else {
            puts("ERROR: zero face found!");
        }
    }
    return true;
}


/**
 This function finds edges that have only a single face associated.
 It then adds a face to this edge and the next edge without a second face.
 If three edges are connected and none has a second face, a new triangle
 will fill the hole.

 \todo: verify that this is robust
 */
void IAMesh::fixHoles()
{
    printf("Fixing holes...\n");
    int i;
    for (i=0; i<(int)edgeList.size(); i++) {
        IAEdge *e = edgeList[i];
        while ( e->nFaces()==1 ) // FIXME: make sure that this is not endless
            fixHole(e);
    }
}


/**
 Add a triangle in an attempt to fill a hole in the mesh.

 \todo: verify that this is robust
 */
void IAMesh::fixHole(IAEdge *e)
{
    printf("Fixing a hole...\n");
    IATriangle *fFix;
    if (e->pFace[0])
        fFix = e->pFace[0];
    else
        fFix = e->pFace[1];
    // walk the fan to the left and find the next edge
    IATriangle *fLeft = fFix;
    IAEdge *eLeft = e;
    for (;;) {
        int ix = eLeft->indexIn(fLeft);
        eLeft = fLeft->pEdge[(ix+2)%3];
        if (eLeft->nFaces()==1)
            break;
        fLeft = eLeft->otherFace(fLeft);
    }
    // walk the fan to the right and find the next edge
    IATriangle *fRight = fFix;
    IAEdge *eRight = e;
    for (;;) {
        int ix = eRight->indexIn(fRight);
        eRight = fRight->pEdge[(ix+1)%3];
        if (eRight->nFaces()==1)
            break;
        fRight = eRight->otherFace(fRight);
    }
    // eLeft and eRight are the next connecting edges
    // fLeft and fRight are the only connected faces
    // fLeft and fRight can well be fFix
    IAVertex *vLeft = eLeft->vertex(0, fLeft);
    IAVertex *vRight = eRight->vertex(1, fRight);
    if (eLeft==eRight) {
        // this is a zero size hole: merge the edges
        puts("ERROR: zero size hole!");
    } else if ( vLeft==vRight ) {
        // this triangle fill conpletely fill the hole
        IATriangle *fNew = new IATriangle();
        fNew->pVertex[0] = e->vertex(1, fFix);
        fNew->pVertex[1] = e->vertex(0, fFix);
        fNew->pVertex[2] = vLeft;
        addFace(fNew);
    } else if (fFix==fRight) {
        if (fLeft==fRight) {
            // we have a single triangle without any connections, delete?
            IATriangle *fNew = new IATriangle();
            fNew->pVertex[0] = fFix->pVertex[2];
            fNew->pVertex[1] = fFix->pVertex[1];
            fNew->pVertex[2] = fFix->pVertex[0];
            addFace(fNew);
        } else {
            fixHole(eRight);
        }
    } else {
        // add one more triangle to get closer to filling the hole
        IATriangle *fNew = new IATriangle();
        fNew->pVertex[0] = e->vertex(1, fFix);
        fNew->pVertex[1] = e->vertex(0, fFix);
        fNew->pVertex[2] = eRight->vertex(1, fRight);
        addFace(fNew);
    }
}


/**
 Add a given face to the mesh.
 \todo we should probably check if this triangle already exists
 \todo make sure we do not create duplicate edges
 \todo make sure that we also do not add duplicate points
 */
void IAMesh::addFace(IATriangle *newFace)
{
    newFace->pEdge[0] = addEdge(newFace->pVertex[0], newFace->pVertex[1], newFace);
    newFace->pEdge[1] = addEdge(newFace->pVertex[1], newFace->pVertex[2], newFace);
    newFace->pEdge[2] = addEdge(newFace->pVertex[2], newFace->pVertex[0], newFace);
    faceList.push_back(newFace);
}


/**
 Create a new edge and add it to this mesh.
 \todo check for duplicates
 */
IAEdge *IAMesh::addEdge(IAVertex *v0, IAVertex *v1, IATriangle *face)
{
    IAEdge *e = findEdge(v0, v1);
    if (e) {
        e->pFace[1] = face;
    } else {
        e = new IAEdge();
        e->pVertex[0] = v0;
        e->pVertex[1] = v1;
        e->pFace[0] = face;
        edgeList.push_back(e);
    }
    return e;
}


/**
 Find an edge that connects two vertices.
 */
IAEdge *IAMesh::findEdge(IAVertex *v0, IAVertex *v1)
{
    for (auto e: edgeList) {
        IAVertex *ev0 = e->pVertex[0];
        IAVertex *ev1 = e->pVertex[1];
        if ((ev0==v0 && ev1==v1)||(ev0==v1 && ev1==v0))
            return e;
    }
    return 0;
}


/**
 Set all face normal counts to 0.
 */
void IAMesh::clearFaceNormals()
{
    for (auto t: faceList) {
        t->pNNormal = 0;
    }
}


/**
 Set all vertex normals to 0.
 */
void IAMesh::clearVertexNormals()
{
    for (auto v: vertexList) {
        v->pNNormal = 0;
    }
}


/**
 Calculate all face normals using the cross product of the vectors making up the triangle.
 */
void IAMesh::calculateFaceNormals()
{
    for (auto t: faceList) {
        IAVector3d p0(t->pVertex[0]->pPosition);
        IAVector3d p1(t->pVertex[1]->pPosition);
        IAVector3d p2(t->pVertex[2]->pPosition);
        p1 -= p0;
        p2 -= p0;
        IAVector3d n = p1.cross(p2);
        n.normalize();
        t->pNormal = n;
        t->pNNormal = 1;
    }
}


/**
 Calculate all vertex normals by averaging the face normals of all connected triangles.
 */
void IAMesh::calculateVertexNormals()
{
    for (auto t: faceList) {
        IAVector3d n(t->pNormal);
        t->pVertex[0]->addNormal(n);
        t->pVertex[1]->addNormal(n);
        t->pVertex[2]->addNormal(n);
    }
    for (auto v: vertexList) {
        v->averageNormal();
    }
}


/**
 Draw the mesh using the vertex normals to create Gouraud shading.
 */
void IAMesh::drawGouraud()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    for (auto t: faceList) {
        for (int j = 0; j < 3; ++j) {
            IAVertex *v = t->pVertex[j];
            glNormal3dv(v->pNormal.dataPointer());
            glTexCoord2dv(v->pTex.dataPointer());
            glVertex3dv(v->pPosition.dataPointer());
        }
    }
    glEnd();
}


/**
 Draw the mesh using the face normals to create flat shading.
 */
void IAMesh::drawFlat(float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
    glBegin(GL_TRIANGLES);
    for (auto t: faceList) {
        glNormal3dv(t->pNormal.dataPointer());
        for (int j = 0; j < 3; ++j) {
            IAVertex *v = t->pVertex[j];
            glTexCoord2dv(v->pTex.dataPointer());
            glVertex3dv(v->pPosition.dataPointer());
        }
    }
    glEnd();
}


//void IAMesh::drawShrunk(unsigned int color, double scale) {
//    int i, j, n = (int)faceList.size();
//    unsigned char r, g, b;
//    Fl::get_color(color, r, g, b);
//    glColor3f(r/266.0, g/266.0, b/266.0);
//    glBegin(GL_TRIANGLES);
//    for (i = 0; i < n; i++) {
//        IATriangle *t = faceList[i];
//        glNormal3dv(t->pNormal.dataPointer());
//        for (j = 0; j < 3; ++j) {
//            IAVertex *v = t->pVertex[j];
//            IAVector3d p = v->pPosition;
//            IAVector3d n = v->pNormal;
//            n *= scale;
//            p -= n;
//            glTexCoord2dv(v->pTex.dataPointer());
//            glVertex3dv(p.dataPointer());
//        }
//    }
//    glEnd();
//}


/**
 Draw all the edges in the mesh.
 */
void IAMesh::drawEdges() {
    glColor3f(0.8f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    for (auto e: edgeList) {
        for (int j = 0; j < 2; ++j) {
            IAVertex *v = e->pVertex[j];
            glTexCoord2dv(v->pTex.dataPointer());
            glVertex3dv(v->pPosition.dataPointer());
        }
    }
    glEnd();
}


/**
 Calculate new texture coordinates for all vertices.
 */
void IAMesh::projectTexture(double w, double h, int type)
{
    int i, n = (int)vertexList.size();
    for (i=0; i<n; i++) {
        IAVertex *v = vertexList.at(i);
        v->projectTexture(w, h, type);
    }
}


/**
 Add a point to a mesh, avoiding duplicates.
 \todo: this should be a function of the mesh or its vertex list
 \todo: this must be accelerated by sorting vertices or better, using a map
 \todo: there should probably be a minimal tollerance when comparinf doubles!
 \return the index of the point in the mesh
 */
size_t IAMesh::addPoint(double x, double y, double z)
{
    size_t i, n = vertexList.size();
    for (i = 0; i < n; ++i) {
        IAVertex *v = vertexList[i];
        if (   v->pPosition.x()==x
            && v->pPosition.y()==y
            && v->pPosition.z()==z)
        {
            return i;
        }
    }
    IAVertex *v = new IAVertex();
    v->pPosition.set(x, y, z);
    v->pInitialPosition.set(x, y, z);
    vertexList.push_back(v);
    return n;
}




// -----------------------------------------------------------------------------


/**
 Delete all meshes in the list.
 */
IAMeshList::~IAMeshList()
{
    for (auto m: meshList) {
        delete m;
    }
}


/**
 Shrink all meshes along the point normals.
 */
void IAMeshList::shrinkBy(double s)
{
    for (auto m: meshList) {
        m->shrinkBy(s);
    }
}


/**
 Draw all meshes with a flat shader
 \param textured if true, activate OpenGL texture rendering
 \param r, g, b, a the base color of the meshes, or white if the textures are enabled
 */
void IAMeshList::drawFlat(bool textured, float r, float g, float b, float a)
{
    if (textured) {
        glEnable(GL_TEXTURE_2D);
        r = g = b = 1.0;
    } else {
        glDisable(GL_TEXTURE_2D);
    }
    for (auto m: meshList) {
        m->drawFlat(r, g, b, a);
    }
    if (textured) {
        glDisable(GL_TEXTURE_2D);
    }
}


/**
 Draw all meshes with a Gouraud shader.
 */
void IAMeshList::drawGouraud()
{
    for (auto m: meshList) {
        // glDepthRange (0.1, 1.0);
        m->drawGouraud();
    }
}


/**
 Calculate new texture coordinates for all meshes.
 */
void IAMeshList::projectTexture(double w, double h, int type)
{
    for (auto m: meshList) {
        m->projectTexture(w, h, type);
    }
}






