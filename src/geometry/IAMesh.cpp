//
//  IAMesh.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAMesh.h"

#include "Iota.h"
#include "geometry/IAEdge.h"

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

    for (auto f: triangleList) {
        delete f;
    }
    triangleList.clear();

    for (auto v: vertexList) {
        delete v;
    }
    vertexList.clear();
    vertexMap.clear();
}


/**
 Various test that validate a watertight triangle mesh.
 \todo This was written a long time ago and must be verified.
 */
bool IAMesh::validate()
{
    bool isWatertight = true;
    printf("Validating mesh with %ld triangles, %ld vertices, and %ld edges...\n",
           triangleList.size(), vertexList.size(), edgeList.size());
    if (triangleList.size()>0 && edgeList.size()==0) {
        puts("WARNING: empty edge list!");
    }
    if (edgeList.size()!=triangleList.size()*3) {
        puts("WARNING: invalid edge list size!");
    }
    int i = 0;
    for (auto he: edgeList) {
        if (he) {
            IATriangle *t = he->triangle();
            if (t==nullptr) {
                printf("ERROR: edge %d [%p] is not linked to a triangle!\n", i, he);
                assert(0);
            } else {
                if (t->pEdge[0]!=he && t->pEdge[1]!=he && t->pEdge[2]!=he) {
                    printf("ERROR: face [%p] is not pointing back at edge %d [%p]!\n", t, i, he);
                    assert(0);
                }
            }
            if (he->vertex()==nullptr) {
                printf("ERROR: edge %d [%p] is not linked to a vertex!\n", i, he);
                assert(0);
            }
            if (he->prev()==nullptr) {
                printf("ERROR: edge %d [%p] is not linked to a previous edge!\n", i, he);
                assert(0);
            }
            if (he->next()==nullptr) {
                printf("ERROR: edge %d [%p] is not linked to a next edge!\n", i, he);
                assert(0);
            }
            if (he->twin()!=nullptr) {
                if (he->twin()->twin()!=he) {
                    printf("ERROR: edge %d [%p] twin link is broken!\n", i, he);
                    assert(0);
                }
                if (he->twin()==he) {
                    printf("ERROR: edge %d [%p] twin links to itself!\n", i, he);
                    assert(0);
                }
            } else {
                isWatertight = false;
            }
        } else {
            puts("ERROR: half-edge nullptr found!");
            assert(0);
        }
        i++;
    }
    i = 0;
    for (auto t: triangleList) {
        if (t) {
            if (t->pVertex[0]==0L || t->pVertex[1]==0L || t->pVertex[1]==0L) {
                printf("ERROR: face %d has an empty vertex field.\n", i);
                assert(0);
            }
            if (t->pEdge[0]==0L || t->pEdge[1]==0L || t->pEdge[1]==0L) {
                printf("ERROR: face %d has an empty edge field.\n", i);
                assert(0);
            } else {
                if (t->pEdge[0]->vertex()!=t->pVertex[0]) {
                    printf("ERROR: face %d has an edge0/vertex0 missmatch.\n", i);
                    assert(0);
                }
                if (t->pEdge[1]->vertex()!=t->pVertex[1]) {
                    printf("ERROR: face %d has an edge1/vertex1 missmatch.\n", i);
                    assert(0);
                }
                if (t->pEdge[2]->vertex()!=t->pVertex[2]) {
                    printf("ERROR: face %d has an edge2/vertex2 missmatch.\n", i);
                    assert(0);
                }
                if (t->pEdge[0]->triangle()!=t) {
                    printf("ERROR: face %d edge0 does not point back at face.\n", i);
                    assert(0);
                }
                if (t->pEdge[1]->triangle()!=t) {
                    printf("ERROR: face %d edge1 does not point back at face.\n", i);
                    assert(0);
                }
                if (t->pEdge[2]->triangle()!=t) {
                    printf("ERROR: face %d edge2 does not point back at face.\n", i);
                    assert(0);
                }
            }
        } else {
            puts("ERROR: triangle nullptr found!");
            assert(0);
        }
        i++;
    }
    if (isWatertight) {
        printf("Done. Mesh is watertight.\n");
    } else {
        printf("Done. Mesh is *NOT* watertight.\n");
    }
    return isWatertight;
}


/**
 * Find outside edges and connect them to other outside edges with new triangles.
 *
 * Find half-edges that have no twin and call the fixHole() on them.
 *
 * \todo: verify that this is robust
 */
void IAMesh::fixHoles()
{
    printf("Fixing holes...\n");
    for (auto e: edgeList) {
        while ( e->twin()==nullptr ) { // FIXME: make sure that this is not endless
            fixHole(e);
        }
    }
}


/**
 * Add a triangle in an attempt to fill a hole in the mesh.
 *
 * \todo: verify that this is robust
 */
void IAMesh::fixHole(IAHalfEdge *e)
{
    if (e->twin()) return;

    printf("Fixing a hole...\n");
    IATriangle *fFix;
    if (e->triangle(0))
        fFix = e->triangle(0);
    else
        fFix = e->triangle(1);
    // walk the fan to the left and find the next edge
    IATriangle *fLeft = fFix;
    IAHalfEdge *eLeft = e;
    for (;;) {
        int ix = eLeft->indexIn(fLeft);
        eLeft = fLeft->pEdge[(ix+2)%3];
        if (eLeft->nTriangle()==1)
            break;
        fLeft = eLeft->otherTriangle(fLeft);
    }
    // walk the fan to the right and find the next edge
    IATriangle *fRight = fFix;
    IAHalfEdge *eRight = e;
    for (;;) {
        int ix = eRight->indexIn(fRight);
        eRight = fRight->pEdge[(ix+1)%3];
        if (eRight->nTriangle()==1)
            break;
        fRight = eRight->otherTriangle(fRight);
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
        addNewTriangle(e->vertex(1, fFix), e->vertex(0, fFix), vLeft);
    } else if (fFix==fRight) {
        if (fLeft==fRight) {
            addNewTriangle(fFix->pVertex[2],
                           fFix->pVertex[1],
                           fFix->pVertex[0]);
        } else {
            fixHole(eRight);
        }
    } else {
        // add one more triangle to get closer to filling the hole
        addNewTriangle(e->vertex(1, fFix),
                       e->vertex(0, fFix),
                       eRight->vertex(1, fRight));
    }
}


/**
 * Create a new triangles and corresponding edges and add it to the mesh.
 *
 * \todo we should probably check if this triangle already exists
 * \param v0, v1, v2 Points that make up the triangle. Make sure that these
 *      points were already checked for duplicats.
 * \return the newly created triangle
 */
IATriangle *IAMesh::addNewTriangle(IAVertex *v0, IAVertex *v1, IAVertex *v2)
{
    IATriangle *t = new IATriangle( this );
    t->pVertex[0] = v0;
    t->pVertex[1] = v1;
    t->pVertex[2] = v2;

    IAHalfEdge *e0 = t->pEdge[0] = new IAHalfEdge(t, v0);
    IAHalfEdge *e1 = t->pEdge[1] = new IAHalfEdge(t, v1);
    IAHalfEdge *e2 = t->pEdge[2] = new IAHalfEdge(t, v2);

    e0->setNext(e1);
    e0->setPrev(e2);

    e1->setNext(e2);
    e1->setPrev(e0);

    e2->setNext(e0);
    e2->setPrev(e1);

    addHalfEdge(e0);
    addHalfEdge(e1);
    addHalfEdge(e2);

    triangleList.push_back(t);
    return t;
}


/**
 * Add a fully initialized half-edge to the mesh for management.
 *
 * If the corresponding half-edge already exists, link them as twins.
 * If there is no twin, just add the half-edge to the list.
 * If there is a twin that already found another twin, we may have a damaged
 * mesh that needs to be repaired later. Just add this edge to the list
 * without linking, so maybe another twin will be added later.
 */
IAHalfEdge *IAMesh::addHalfEdge(IAHalfEdge *e)
{
    // if all triangles are orinted correctly, the twin will have vertices
    // in the opposite order.
    IAVertex *v0 = e->vertex();
    IAVertex *v1 = e->next()->vertex();
    IAHalfEdge *matchingHalfEdge = findEdge(v1, v0);
    if (matchingHalfEdge && matchingHalfEdge->twin()==nullptr) {
        e->setTwin(matchingHalfEdge);
        matchingHalfEdge->setTwin(e);
    }
    edgeList.push_back(e);
    edgeMap.insert(std::make_pair(v0->pLocalPosition.length()+v1->pLocalPosition.length(), e));
    return matchingHalfEdge;
}


/**
 Find an edge that connects two vertices.
 */
IAHalfEdge *IAMesh::findEdge(IAVertex *v0, IAVertex *v1)
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
        IAHalfEdge *e = (*it).second;
        IAVertex *ev0 = e->vertex(0);
        IAVertex *ev1 = e->vertex(1);
        if (ev0==v0 && ev1==v1)
            return e;
    }
#endif
    return 0;
}


/**
 Set all face normal counts to 0.
 */
void IAMesh::clearTriangleNormals()
{
    for (auto t: triangleList) {
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
void IAMesh::calculateTriangleNormals()
{
    for (auto t: triangleList) {
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
    for (auto t: triangleList) {
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
    for (auto t: triangleList) {
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
    for (auto t: triangleList) {
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


/**
 Draw all the edges in the mesh.
 */
void IAMesh::drawEdges() {
    glColor3f(0.8f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    for (auto e: edgeList) {
        for (int j = 0; j < 2; ++j) {
            IAVertex *v = e->vertex(j);
            glTexCoord2dv(v->pTex.dataPointer());
            glVertex3dv(v->pLocalPosition.dataPointer());
        }
    }
    glEnd();
}


/**
 Calculate new texture coordinates for all vertices.
 */
void IAMesh::projectTexture(double wMult, double hMult, int type)
{
    double x = 0.0, w = 1.0;
    double y = 0.0, h = 1.0;
    switch (type) {
        case IA_PROJECTION_FRONT:
            x = pMin.x(); w = 1.0 / (pMax.x() - pMin.x()) * wMult;
            y = pMin.z(); h = 1.0 / (pMax.z() - pMin.z()) * hMult;
            for (auto v: vertexList) {
                v->projectTexture(x, y, w, h, type);
            }
            break;
        case IA_PROJECTION_CYLINDRICAL:
            x = 0.0; w = wMult;
            y = pMin.z(); h = 1.0 / (pMax.z() - pMin.z()) * hMult;
            for (auto v: vertexList) {
                v->projectTexture(x, y, w, h, type);
            }
            break;
        case IA_PROJECTION_SPHERICAL:
            break;
    }
}


/**
 * Add a vertex to a mesh, avoiding duplicates.
 *
 * Find an existing vertex with the given coordinates. If none is found,
 * create a new vertex and add it to list.
 *
 * \param the positin of this vertex in mesh space
 * \return the existing or newly created vertex. There is no way of knowing if
 *      the vertex was found or created.
 *
 * \todo there should probably be a minimal toloerance when comparing positions!
 * \todo create a vertex list class and move this methode there
 */
IAVertex *IAMesh::findOrAddNewVertex(IAVector3d const& pos)
{
    double length = pos.length();
    auto itlow = vertexMap.lower_bound(length-0.0001);
    auto itup = vertexMap.upper_bound(length+0.0001);

    for (auto it=itlow; it!=itup; ++it) {
        IAVertex *v = (*it).second;
        if (v->pLocalPosition==pos) {
            return v;
        }
    }

    IAVertex *v = new IAVertex();
    v->pLocalPosition = pos;
    updateBoundingBox(pos);
    vertexList.push_back(v);
    vertexMap.insert(std::make_pair(v->pLocalPosition.length(), v));
    return v;
}


/**
 * Expand the bounding bo to include the given vector.
 */
void IAMesh::updateBoundingBox(IAVector3d const& v)
{
    pMin.setMin(v);
    pMax.setMax(v);
}


/**
 * Draw a sliced version of this mesh.
 */
void IAMesh::drawSliced(double zPlane)
{
    GLdouble equationLowerHalf[4] = { 0.0, 0.0, -1.0, zPlane-0.05 };
    GLdouble equationUpperHalf[4] = { 0.0, 0.0, 1.0, -zPlane+0.05 };

    // --- draw the opaque lower half of the model
    // save the current model matrix
    glPushMatrix();
    // undo the mesh transformation
    glTranslated(-Iota.pMesh->position().x(), -Iota.pMesh->position().y(), -Iota.pMesh->position().z());
    // set the clipping plane in world coordinates
    // TOD0: we should instead simply sve the world coordinates with the view.
    glClipPlane(GL_CLIP_PLANE0, equationLowerHalf);
    glClipPlane(GL_CLIP_PLANE1, equationUpperHalf);
    // use the clipping plane; it clips everything above zPlane
    glEnable(GL_CLIP_PLANE0);
    // restore the matrix for this mesh
    glPopMatrix();
    // draw the model in any shader we like
    drawFlat(Iota.gShowTexture);
    // disable the clipping plane
    glDisable(GL_CLIP_PLANE0);

#if 0   // draw the lid
    // this draw a vector version of the current lid.
    Iota.gMeshSlice.drawFlat(1.0, 0.9, 0.9);
    // to draw the voxel version of the slice, use (in global space)
    // Iota.gMeshSlice.drawFramebuffer();
#endif

#if 0
    // draw a ghoste upper half of the model
    // This may or may not hel orientation. It's currently disabled because
    // it messes up the depth buffer for later slice drawing operations.
    // To fix that, we'd have to add functions drawSlicedLower and
    // drawSlicedUpper, which we call late.
    glEnable(GL_CLIP_PLANE1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    Iota.pMesh->drawFlat(false, 0.6, 0.6, 0.6, 0.1);
    glDisable(GL_CLIP_PLANE1);
#endif

    glDisable(GL_CULL_FACE);
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
    IAVector3d p = ( (pMax - pMin) * -0.5 ) - pMin;
    IAVector3d v = printer->pBuildVolume * 0.5;
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


/**
 * Update all variables concerning global space needed for slicing.
 */
void IAMesh::updateGlobalSpace()
{
    if (pGlobalPositionNeedsUpdate) {
        IAVector3d dp = position();
        for (auto v: vertexList) {
            v->pGlobalPosition = v->pLocalPosition + dp;
            // \todo apply full mesh transformation
        }
        pGlobalPositionNeedsUpdate = false;
    }
}




