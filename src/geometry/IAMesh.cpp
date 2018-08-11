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


IAMesh::IAMesh()
{
}

void IAMesh::clear()
{
    int i, n = (int)edgeList.size();
    for (i=0; i<n; i++) {
        delete edgeList[i];
    }
    edgeList.clear();
    n = (int)faceList.size();
    for (i=0; i<n; i++) {
        delete faceList[i];
    }
    faceList.clear();
    n = (int)vertexList.size();
    for (i=0; i<n; i++) {
        delete vertexList[i];
    }
    vertexList.clear();
}


void IAMesh::shrinkTo(double s)
{
    int i, n = (int)vertexList.size();
    for (i=0; i<n; i++) {
        vertexList[i]->shrinkTo(s);
    }
}


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
 */
void IAMesh::fixHoles()
{
    printf("Fixing holes...\n");
#ifdef M_MONKEY
    fixHole(edgeList[21]);
#elif defined M_DRAGON
    return;
#elif defined M_PEPSI
    return;
#endif
    int i;
    for (i=0; i<(int)edgeList.size(); i++) {
        IAEdge *e = edgeList[i];
        while ( e->nFaces()==1 ) // FIXME: make sure that this is not endless
            fixHole(e);
    }
}

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

void IAMesh::addFace(IATriangle *newFace)
{
    newFace->pEdge[0] = addEdge(newFace->pVertex[0], newFace->pVertex[1], newFace);
    newFace->pEdge[1] = addEdge(newFace->pVertex[1], newFace->pVertex[2], newFace);
    newFace->pEdge[2] = addEdge(newFace->pVertex[2], newFace->pVertex[0], newFace);
    faceList.push_back(newFace);
}

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

IAEdge *IAMesh::findEdge(IAVertex *v0, IAVertex *v1)
{
    int i, n = (int)edgeList.size();
    for (i=0; i<n; i++) {
        IAEdge *IAEdge = edgeList.at(i);
        IAVertex *ev0 = IAEdge->pVertex[0];
        IAVertex *ev1 = IAEdge->pVertex[1];
        if ((ev0==v0 && ev1==v1)||(ev0==v1 && ev1==v0))
            return IAEdge;
    }
    return 0;
}

void IAMesh::clearFaceNormals()
{
    int i, n = (int)faceList.size();
    for (i=0; i<n; i++) {
        IATriangle *IATriangle = faceList.at(i);
        IATriangle->pNNormal = 0;
    }
}

void IAMesh::clearVertexNormals()
{
    int i, n = (int)vertexList.size();
    for (i=0; i<n; i++) {
        IAVertex *IAVertex = vertexList.at(i);
        IAVertex->pNNormal = 0;
    }
}

void IAMesh::calculateFaceNormals()
{
    int i, n = (int)faceList.size();
    for (i=0; i<n; i++) {
        IATriangle *IATriangle = faceList.at(i);
        IAVector3d p0(IATriangle->pVertex[0]->pPosition);
        IAVector3d p1(IATriangle->pVertex[1]->pPosition);
        IAVector3d p2(IATriangle->pVertex[2]->pPosition);
        p1 -= p0;
        p2 -= p0;
        IAVector3d n = p1.cross(p2);
        n.normalize();
        IATriangle->pNormal = n;
        IATriangle->pNNormal = 1;
    }
}

void IAMesh::calculateVertexNormals()
{
    int i, n = (int)faceList.size();
    for (i=0; i<n; i++) {
        IATriangle *IATriangle = faceList.at(i);
        IAVector3d n(IATriangle->pNormal);
        IATriangle->pVertex[0]->addNormal(n);
        IATriangle->pVertex[1]->addNormal(n);
        IATriangle->pVertex[2]->addNormal(n);
    }
    n = (int)vertexList.size();
    for (i=0; i<n; i++) {
        IAVertex *IAVertex = vertexList.at(i);
        IAVertex->averageNormal();
    }
}

void IAMesh::drawGouraud() {
    int i, j, n = (int)faceList.size();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    for (i = 0; i < n; i++) {
        IATriangle *IATriangle = faceList[i];
        for (j = 0; j < 3; ++j) {
            IAVertex *IAVertex = IATriangle->pVertex[j];
            glNormal3dv(IAVertex->pNormal.dataPointer());
            glTexCoord2dv(IAVertex->pTex.dataPointer());
            glVertex3dv(IAVertex->pPosition.dataPointer());
        }
    }
    glEnd();
}

void IAMesh::drawFlat(unsigned int color) {
    int i, j, n = (int)faceList.size();
    unsigned char r, g, b;
    Fl::get_color(color, r, g, b);
    glColor3f(r/266.0, g/266.0, b/266.0);
    glBegin(GL_TRIANGLES);
    for (i = 0; i < n; i++) {
        IATriangle *IATriangle = faceList[i];
        for (j = 0; j < 3; ++j) {
            IAVertex *IAVertex = IATriangle->pVertex[j];
            glTexCoord2dv(IAVertex->pTex.dataPointer());
            glVertex3dv(IAVertex->pPosition.dataPointer());
        }
    }
    glEnd();
}

void IAMesh::drawShrunk(unsigned int color, double scale) {
    int i, j, n = (int)faceList.size();
    unsigned char r, g, b;
    Fl::get_color(color, r, g, b);
    glColor3f(r/266.0, g/266.0, b/266.0);
    glBegin(GL_TRIANGLES);
    for (i = 0; i < n; i++) {
        IATriangle *IATriangle = faceList[i];
        for (j = 0; j < 3; ++j) {
            IAVertex *IAVertex = IATriangle->pVertex[j];
            IAVector3d p = IAVertex->pPosition;
            IAVector3d n = IAVertex->pNormal;
            n *= scale;
            p -= n;
            glVertex3dv(p.dataPointer());
        }
    }
    glEnd();
}

void IAMesh::drawEdges() {
    int i, j, n = (int)edgeList.size();
    glColor3f(0.8f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    for (i = 0; i < n; i++) {
        IAEdge *IAEdge = edgeList[i];
        for (j = 0; j < 2; ++j) {
            IAVertex *IAVertex = IAEdge->pVertex[j];
            glTexCoord2dv(IAVertex->pTex.dataPointer());
            glVertex3dv(IAVertex->pPosition.dataPointer());
        }
    }
    glEnd();
}

// -----------------------------------------------------------------------------

void IAMeshList::shrinkTo(double s)
{
    int i, n = size();
    for (i=0; i<n; i++) {
        IAMesh *IAMesh = meshList[i];
        IAMesh->shrinkTo(s);
    }
}

void IAMeshList::drawFlat(unsigned int color)
{
    int i, n = size();
    for (i=0; i<n; i++) {
        IAMesh *IAMesh = meshList[i];
        IAMesh->drawFlat(color);
    }
}


