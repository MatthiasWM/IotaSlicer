//
//  IAMesh.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAMesh.h"

#include "Iota.h"
#include "geometry/IAEdge.h"
#include "printer/IAPrinter.h"

#include <FL/fl_draw.H>
#include <FL/gl.h>
#include <FL/glu.h>


/**
 * Create an empty mesh.
 */
IAMesh::IAMesh()
{
}


/**
 * Clear all resources used by the mesh.
 */
void IAMesh::clear()
{
    for (auto &e: edgeList) {
        delete e;
    }
    edgeList.clear();
    edgeMap.clear();

    for (auto &f: triangleList) {
        delete f;
    }
    triangleList.clear();

    for (auto &v: vertexList) {
        delete v;
    }
    vertexList.clear();
    vertexMap.clear();
}


/**
 * Various test that validate a watertight triangle mesh.
 *
 * \return true, if the mesh is valid and watertight.
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
    for (auto &he: edgeList) {
        if (he) {
            IATriangle *t = he->triangle();
            if (t==nullptr) {
                printf("ERROR: edge %d [%p] is not linked to a triangle!\n", i, he);
                assert(0);
            } else {
                if (t->edge(0)!=he && t->edge(1)!=he && t->edge(2)!=he) {
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
    for (auto &t: triangleList) {
        if (t) {
            if (t->edge(0)==0L || t->edge(1)==0L || t->edge(2)==0L) {
                printf("ERROR: face %d has an empty edge field.\n", i);
                assert(0);
            } else {
                if (t->edge(0)->triangle()!=t) {
                    printf("ERROR: face %d edge0 does not point back at face.\n", i);
                    assert(0);
                }
                if (t->edge(1)->triangle()!=t) {
                    printf("ERROR: face %d edge1 does not point back at face.\n", i);
                    assert(0);
                }
                if (t->edge(2)->triangle()!=t) {
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
 */
void IAMesh::fixHoles()
{
    printf("Fixing holes...\n");
    // we can't use a foreach loop here because edges will be added
    // in the process! Don't use iterators either because std:vector may
    // reallocate the array.
    for (IAHalfEdgeList::size_type i=0; i<edgeList.size(); ++i) {
        IAHalfEdge *e = edgeList[i];
        if ( e->twin()==nullptr ) {
            fixHole(e);
        }
    }
}


/**
 * Add a triangle in an attempt to fill a hole in the mesh.
 *
 * \param e start fixing at this half-edge if it has no twin.
 */
void IAMesh::fixHole(IAHalfEdge *e)
{
    if (e->twin()) return; // defensive

    // cases:
    // 1: just one edge in this triangle is open
    //      find the open edge in the left and right fan and choose the one
    //      that creates the better triangle
    // 2: two edges in this triangle are open
    //      avoid adding a triangle around the point that has both open edges
    // 3: all three edges in this triangle are open
    //      This triangle is basically useless. A good repair tool would
    //      maybe thicken the triangle, or delete it. We are doing a non-
    //      destructive minimum repair, so we just duplicate it

    if (e->next()->twin()==nullptr && e->prev()->twin()==nullptr) {
        // case 3: just add the same triange flipped to create a single
        // manifold triangle
        IATriangle *t = e->triangle();
        addNewTriangle(t->vertex(2), e->next()->vertex(), e->vertex());
    } else if (e->next()->twin()==nullptr) {
        // case 2: our vertex is a good fan candidate
        IAHalfEdge *e2 = e->findPrevSingleEdgeInFan();
        assert(e2);
        addNewTriangle(e->next()->vertex(), e->vertex(), e2->vertex());
    } else if (e->prev()->twin()==nullptr) {
        // case 2: prev->vertex is a good fan candidate
        IAHalfEdge *e2 = e->next()->findNextSingleEdgeInFan();
        assert(e2);
        addNewTriangle(e->next()->vertex(), e->vertex(), e2->next()->vertex());
    } else {
        // case 1: either vertex is a candidate
        IAHalfEdge *e2 = e->next()->findNextSingleEdgeInFan();
        assert(e2);
        addNewTriangle(e->next()->vertex(), e->vertex(), e2->next()->vertex());
    }
}


/**
 * Create a new triangles and corresponding edges and add it to the mesh.
 *
 * It is possible (but not useful) to create two identical triangles. This
 * may confuse the mesh fixer.
 *
 * \param v0, v1, v2 Points that make up the triangle. Make sure that these
 *      points were already checked for duplicats.
 * \return the newly created triangle
 */
IATriangle *IAMesh::addNewTriangle(IAVertex *v0, IAVertex *v1, IAVertex *v2)
{
    IATriangle *t = new IATriangle( this );

    IAHalfEdge *e0 = new IAHalfEdge(t, v0);
    IAHalfEdge *e1 = new IAHalfEdge(t, v1);
    IAHalfEdge *e2 = new IAHalfEdge(t, v2);
    t->setEdges(e0, e1, e2);

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
 *
 * \param e add this edge to the mesh
 *
 * \return the twin for this edge, or a nullptr if the twin was not found
 */
IAHalfEdge *IAMesh::addHalfEdge(IAHalfEdge *e)
{
    // if all triangles are orinted correctly, the twin will have vertices
    // in the opposite order.
    IAVertex *v0 = e->vertex();
    IAVertex *v1 = e->next()->vertex();
    IAHalfEdge *matchingHalfEdge = findSingleEdge(v1, v0);
    if (matchingHalfEdge) {
        e->setTwin(matchingHalfEdge);
        matchingHalfEdge->setTwin(e);
    }
    edgeList.push_back(e);
    edgeMap.insert(std::make_pair(v0->pLocalPosition.length()+v1->pLocalPosition.length(), e));
    return matchingHalfEdge;
}


/**
 * Find an edge that connects two vertices.
 *
 * This finds the first edge that connects two vertices, whether it has a twin
 * or not.
 *
 * \param v0, v1 vertices that make up the half-edge, in the desired order
 *
 * \return a pointer to the edge found, or nullptr if there was none.
 */
IAHalfEdge *IAMesh::findEdge(IAVertex *v0, IAVertex *v1)
{
    double key = v0->pLocalPosition.length()+v1->pLocalPosition.length();
    auto itlow = edgeMap.lower_bound(key-0.0001);
    auto itup = edgeMap.upper_bound(key+0.0001);
    for (auto &it=itlow; it!=itup; ++it) {
        IAHalfEdge *e = (*it).second;
        IAVertex *ev0 = e->vertex();
        IAVertex *ev1 = e->next()->vertex();
        if (ev0==v0 && ev1==v1)
            return e;
    }
    return 0;
}


/**
 * Find an edge that connects two vertices, and that has no twin.
 *
 * \param v0, v1 vertices that make up the half-edge, in the desired order
 *
 * \return a pointer to the edge found, or nullptr if there was none.
 */
IAHalfEdge *IAMesh::findSingleEdge(IAVertex *v0, IAVertex *v1)
{
    double key = v0->pLocalPosition.length()+v1->pLocalPosition.length();
    auto itlow = edgeMap.lower_bound(key-0.0001);
    auto itup = edgeMap.upper_bound(key+0.0001);
    for (auto &it=itlow; it!=itup; ++it) {
        IAHalfEdge *e = (*it).second;
        IAVertex *ev0 = e->vertex();
        IAVertex *ev1 = e->next()->vertex();
        if (ev0==v0 && ev1==v1 && !e->twin())
            return e;
    }
    return 0;
}


/**
 * Set all vertex normals to (0, 0, 0) in prepartion for calculating the
 * point normals.
 */
void IAMesh::clearVertexNormals()
{
    for (auto &v: vertexList) {
        v->pNNormal = 0;
    }
}


/**
 * Calculate all face normals using the cross product of the vectors making
 * up the triangle.
 */
void IAMesh::calculateTriangleNormals()
{
    for (auto &t: triangleList) {
        IAVector3d p0(t->vertex(0)->pLocalPosition);
        IAVector3d p1(t->vertex(1)->pLocalPosition);
        IAVector3d p2(t->vertex(2)->pLocalPosition);
        p1 -= p0;
        p2 -= p0;
        IAVector3d n = (p1 ^ p2).normalized(); // cross product
        t->pNormal = n;
    }
}


/**
 * Calculate all vertex normals by averaging the face normals of all
 * connected triangles.
 */
void IAMesh::calculateVertexNormals()
{
    clearVertexNormals();
    for (auto &t: triangleList) {
        IAVector3d n(t->pNormal);
        t->vertex(0)->addNormal(n);
        t->vertex(1)->addNormal(n);
        t->vertex(2)->addNormal(n);
    }
    for (auto &v: vertexList) {
        v->averageNormal();
    }
}


/**
 * Draw the mesh using the face normals to create flat shading.
 *
 * \param textured if true, activate OpenGL texture rendering
 * \param r, g, b, a the base color of the meshes, or white if the textures
 *      are enabled
 */
void IAMesh::draw(Shader s, float r, float g, float b, float a)
{
    glPushAttrib(GL_ENABLE_BIT);
    switch (s) {
        case kTEXTURED:
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_LIGHTING);
            r = g = b = 1.0;
            break;
        case kFLAT:
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_LIGHTING);
            break;
        case kMASK:
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            break;
    }

    glColor4f(r, g, b, a);
    glBegin(GL_TRIANGLES);
    for (auto &t: triangleList) {
        glNormal3dv(t->pNormal.dataPointer());
        for (int j = 0; j < 3; ++j) {
            IAVertex *v = t->vertex(j);
            if (s==kTEXTURED) glTexCoord2dv(v->pTex.dataPointer());
            glVertex3dv(v->pLocalPosition.dataPointer());
        }
    }
    glEnd();

    glPopAttrib();
}


void IAMesh::drawAngledFaces(double a)
{
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 1.0);

    IAVector3d zVec = { 0.0, 0.0, 1.0 };
    double ref = cos(a/180.0*M_PI);

    glBegin(GL_TRIANGLES);
    for (auto &t: triangleList) {
        double na = t->pNormal.dot(zVec);
        if (na<ref) {
            glNormal3dv(t->pNormal.dataPointer()); // FIXME: global normal!
            for (int j = 0; j < 3; ++j) {
                IAVertex *v = t->vertex(j);
//                glVertex3dv(v->pLocalPosition.dataPointer());
                glVertex3dv(v->pGlobalPosition.dataPointer());
            }
        }
    }
    glEnd();

}


/**
 * Draw all the edges in the mesh.
 */
void IAMesh::drawEdges() {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
//    glPolygonOffset( -1.0, -1.0 );
//    glEnable( GL_POLYGON_OFFSET_LINE );
    for (auto &e: edgeList) {
        if (e->twin()) {
            glColor3f(0.8f, 1.0f, 1.0f);
            glLineWidth(2.0);
            glBegin(GL_LINES);
            glVertex3dv(e->vertex()->pLocalPosition.dataPointer());
            glVertex3dv(e->next()->vertex()->pLocalPosition.dataPointer());
            glEnd();
        } else {
            glColor3f(1.0f, 0.5f, 0.5f);
            glLineWidth(4.0);
            glBegin(GL_LINES);
            glVertex3dv(e->vertex()->pLocalPosition.dataPointer());
            glVertex3dv(e->next()->vertex()->pLocalPosition.dataPointer());
            glEnd();
        }
    }
//    glDisable( GL_POLYGON_OFFSET_LINE );
//    glPolygonOffset( 0.0, 0.0 );
}


/**
 * Calculate new texture coordinates for all vertices.
 *
 * \param wMult, hMult size mutiplicators for the projection types
 * \param type how to project the texture onto the mesh
 *
 * \todo this does not take texture wrapping into account.
 */
void IAMesh::projectTexture(double wMult, double hMult, int type)
{
    double x = 0.0, w = 1.0;
    double y = 0.0, h = 1.0;
    switch (type) {
        case IA_PROJECTION_FRONT:
            x = pMin.x(); w = 1.0 / (pMax.x() - pMin.x()) * wMult;
            y = pMin.z(); h = 1.0 / (pMax.z() - pMin.z()) * hMult;
            for (auto &v: vertexList) {
                v->projectTexture(x, y, w, h, type);
            }
            break;
        case IA_PROJECTION_CYLINDRICAL:
            x = 0.0; w = wMult;
            y = pMin.z(); h = 1.0 / (pMax.z() - pMin.z()) * hMult;
            for (auto &v: vertexList) {
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
 * \param pos the position of this vertex in mesh space
 *
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

    for (auto &it=itlow; it!=itup; ++it) {
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
 *
 * \param v have the bounding box contain this vextor
 */
void IAMesh::updateBoundingBox(IAVector3d const& v)
{
    pMin.setMin(v);
    pMax.setMax(v);
}


/**
 * Draw a sliced version of this mesh.
 *
 * \param zPlane the clipping plane in z
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
    draw(IAMesh::kFLAT);
    // disable the clipping plane
    glDisable(GL_CLIP_PLANE0);

#if 0   // draw the lid
    // this draw a vector version of the current lid.
    Iota.gMeshSlice.drawFlat(1.0, 0.9, 0.9);
    // to draw the voxel version of the slice, use (in global space)
    // Iota.gMeshSlice.drawFramebuffer();
#endif

    glDisable(GL_CULL_FACE);
}


/**
 * Draw upper half of a sliced version of this mesh as a ghost.
 *
 * \param zPlane the clipping plane in z
 */
void IAMesh::drawSlicedGhost(double zPlane)
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

    // draw a ghoste upper half of the model
    // This may or may not hel orientation. It's currently disabled because
    // it messes up the depth buffer for later slice drawing operations.
    // To fix that, we'd have to add functions drawSlicedLower and
    // drawSlicedUpper, which we call late.
    glEnable(GL_CLIP_PLANE1);
    glPopMatrix();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    Iota.pMesh->draw(IAMesh::kFLAT, 0.6, 0.6, 0.6, 0.1);
    glDisable(GL_CLIP_PLANE1);

    glDisable(GL_CULL_FACE);
}


/**
 * Position the mesh on the center point of the printer bed.
 *
 * This method uses the size of the mesh to determine the center on the printbed
 * in X and Y. Z position is set, so that no point of the mesh is below the
 * printbed.
 *
 * \param printer use this printer
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
 *
 * \return the position in world space
 *
 * \todo this should return an entire transformation.
 */
IAVector3d IAMesh::position() const
{
    return pMeshPosition;
}


/**
 * Set a new object position.
 *
 * Never set the pMeshPosition member directly!
 *
 * \param p the new position in global space
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
        for (auto &v: vertexList) {
            v->pGlobalPosition = v->pLocalPosition + dp;
            // \todo apply full mesh transformation
        }
        pGlobalPositionNeedsUpdate = false;
    }
}




