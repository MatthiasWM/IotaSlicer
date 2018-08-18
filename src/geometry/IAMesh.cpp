//
//  IAMesh.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAMesh.h"

#include "../Iota.h"
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
    edgeMap.clear();

    for (auto f: faceList) {
        delete f;
    }
    faceList.clear();

    for (auto v: vertexList) {
        delete v;
    }
    vertexList.clear();
    vertexMap.clear();
}


/**
 Move all vertices along the negative point normal, effectively shrining the mesh.
 */
//void IAMesh::shrinkBy(double s)
//{
//    for (auto v: vertexList) {
//        v->shrinkBy(s);
//    }
//}


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
        edgeMap.insert(std::make_pair(v0->pLocalPosition.length()+v1->pLocalPosition.length(), e));
    }
    return e;
}


/**
 Find an edge that connects two vertices.
 */
IAEdge *IAMesh::findEdge(IAVertex *v0, IAVertex *v1)
{
#if 0
    for (auto e: edgeList) {
        IAVertex *ev0 = e->pVertex[0];
        IAVertex *ev1 = e->pVertex[1];
        if ((ev0==v0 && ev1==v1)||(ev0==v1 && ev1==v0))
            return e;
    }
#else
    double key = v0->pLocalPosition.length()+v1->pLocalPosition.length();
    auto itlow = edgeMap.lower_bound(key-0.0001);
    auto itup = edgeMap.upper_bound(key+0.0001);
    for (auto it=itlow; it!=itup; ++it) {
        IAEdge *e = (*it).second;
        IAVertex *ev0 = e->pVertex[0];
        IAVertex *ev1 = e->pVertex[1];
        if ((ev0==v0 && ev1==v1)||(ev0==v1 && ev1==v0))
            return e;
    }
#endif
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
        IAVector3d p0(t->pVertex[0]->pLocalPosition);
        IAVector3d p1(t->pVertex[1]->pLocalPosition);
        IAVector3d p2(t->pVertex[2]->pLocalPosition);
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
            glVertex3dv(v->pLocalPosition.dataPointer());
        }
    }
    glEnd();
}


/**
 Draw the mesh using the face normals to create flat shading.
 \param textured if true, activate OpenGL texture rendering
 \param r, g, b, a the base color of the meshes, or white if the textures are enabled
 */
void IAMesh::drawFlat(bool textured, float r, float g, float b, float a)
{
    if (textured) {
        glEnable(GL_TEXTURE_2D);
        r = g = b = 1.0;
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    glColor4f(r, g, b, a);
    glBegin(GL_TRIANGLES);
    for (auto t: faceList) {
        glNormal3dv(t->pNormal.dataPointer());
        for (int j = 0; j < 3; ++j) {
            IAVertex *v = t->pVertex[j];
            glTexCoord2dv(v->pTex.dataPointer());
            glVertex3dv(v->pLocalPosition.dataPointer());
        }
    }
    glEnd();

    if (textured) {
        glDisable(GL_TEXTURE_2D);
    }
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
            glVertex3dv(v->pLocalPosition.dataPointer());
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
    IAVector3d pos(x, y, z);
    size_t n = vertexList.size();
#if 0
    size_t i;
    for (i = 0; i < n; ++i) {
        IAVertex *v = vertexList[i];
        if (   v->pPosition.x()==x
            && v->pPosition.y()==y
            && v->pPosition.z()==z)
        {
            return i;
        }
    }
#else
    double length = pos.length();
    auto itlow = vertexMap.lower_bound(length-0.0001);
    auto itup = vertexMap.upper_bound(length+0.0001);

    for (auto it=itlow; it!=itup; ++it) {
        int ix = (*it).second;
        if (vertexList[ix]->pLocalPosition==pos) {
            return ix;
        }
    }
#endif
    IAVertex *v = new IAVertex();
    v->pLocalPosition = pos;
    updateBoundingBox(pos);
    vertexList.push_back(v);
    vertexMap.insert(std::make_pair(v->pLocalPosition.length(), n));
    return n;
}


void IAMesh::updateBoundingBox(IAVector3d &v)
{
    pMin.setMin(v);
    pMax.setMax(v);
}


/**
 * Draw a sliced version of this mesh.
 */
void IAMesh::drawSliced(double zPlane)
{
    // draw the opaque lower half of the model
    GLdouble equationLowerHalf[4] = { 0.0, 0.0, -1.0, zPlane-0.05 };
    GLdouble equationUpperHalf[4] = { 0.0, 0.0, 1.0, -zPlane+0.05 };
    glClipPlane(GL_CLIP_PLANE0, equationLowerHalf);
    glEnable(GL_CLIP_PLANE0);
    drawFlat(Iota.gShowTexture);
    //        glEnable(GL_TEXTURE_2D);
    //        gMeshList[0]->drawShrunk(FL_WHITE, -2.0);


#if 1   // draw the shell
    // FIXME: this messes up tesselation for the lid!
    // FIXME: we do not need to tesselate at all!
//    glDisable(GL_LIGHTING);
//    glEnable(GL_TEXTURE_2D);
//    glLineWidth(8.0);
//    for (int n = 20; n>0; --n) {
//        shrinkBy(0.1*n);
//        IASlice meshSlice;
//        meshSlice.generateOutlineFrom(this, zPlane);
//        drawFlat(true);
//        glEnable(GL_TEXTURE_2D);
//        glColor3ub(128, 128, 128);
//        meshSlice.drawLidEdge();
//        glDisable(GL_TEXTURE_2D);
//    }
//    shrinkBy(0.0);
//    glLineWidth(1.0);
//    glDisable(GL_TEXTURE_2D);
//    glEnable(GL_LIGHTING);
//    glEnable(GL_DEPTH_TEST);
#endif

#if 0   // draw the lid
    glDisable(GL_CLIP_PLANE0);
    gMeshSlice.drawFlat(1.0, 0.9, 0.9);
#endif

#if 0
    // draw a texture map on the lid
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.0, 1.0, 0.0, 0.1);
    glPushMatrix();
    glTranslated(gPrinter.pBuildVolumeOffset.x(), gPrinter.pBuildVolumeOffset.x(), 0.01);
    glBegin(GL_POLYGON);
    glVertex3d(gPrinter.pBuildVolumeMin.x(),
               gPrinter.pBuildVolumeMin.y(),
               zPlane);
    glVertex3d(gPrinter.pBuildVolumeMax.x(),
               gPrinter.pBuildVolumeMin.y(),
               zPlane);
    glVertex3d(gPrinter.pBuildVolumeMax.x(),
               gPrinter.pBuildVolumeMax.y(),
               zPlane);
    glVertex3d(gPrinter.pBuildVolumeMin.x(),
               gPrinter.pBuildVolumeMax.y(),
               zPlane);
    glEnd();
    glPopMatrix();
#endif

    // draw a ghoste upper half of the mode
    glClipPlane(GL_CLIP_PLANE0, equationUpperHalf);
    glEnable(GL_CLIP_PLANE0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    Iota.pMesh->drawFlat(false, 0.6, 0.6, 0.6, 0.1);

    glDisable(GL_CULL_FACE);
    glDisable(GL_CLIP_PLANE0);
}


/**
 * Position the mesh on the center point of the printer bed.
 *
 * This method uses the size of the mesh to determine the center on the printbed
 * in X and Y. Z position is set, so that no point of the mesh is below the
 * printbed.
 */
void IAMesh::centerOnPrintbed(IAPrinter *printer)
{
    IAVector3d p;
    p += pMax;
    p -= pMin;
    p *= -0.5;
    p -= pMin;

    IAVector3d v = printer->pBuildVolume;
    v *= 0.5;
    p += v;
    
    p.z( -pMin.z() );
    position(p);
}


/**
 * Return a copy of the position of the mesh.
 *
 * Changing position directly would invalidate buffered coordinates.
 * The position must only be changed by calling IAMesh::position(v).
 */
IAVector3d IAMesh::position() const
{
    return pMeshPosition;
}


/**
 * Set a new object position.
 *
 * Never set the pMeshPosition member directly!
 */
void IAMesh::position(const IAVector3d &p)
{
    pMeshPosition = p;
    pGlobalPositionNeedsUpdate = true;
}




