//
//  geometry.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAVertex.h"

#include "Iota.h"


/**
 Create a vertex at 0, 0, 0.
 */
IAVertex::IAVertex()
{
}


/**
 Dublicate another vertex.
 */
IAVertex::IAVertex(const IAVertex *v)
{
    pLocalPosition = v->pLocalPosition;
    pNormal = v->pNormal;
    pTex = v->pTex;
    pNNormal = v->pNNormal;
}


/**
 Add a vector to the current normal and increase the normal count.
 This method is used to calculate the average of the normals of all
 connected triangles.
 \see IAVertex::averageNormal()
 */
void IAVertex::addNormal(const IAVector3d &v)
{
    IAVector3d vn(v);
    vn.normalize();
    pNormal += vn;
    pNNormal++;
}


/**
 Divide the normal vector by the number of normals we accumulated.
 \see IAVertex::addNormal(const IAVector3d &v)
 */
void IAVertex::averageNormal()
{
    if (pNNormal>0) {
        double len = 1.0/pNNormal;
        pNormal *= len;
    }
}


/**
 Print the position of a vertex.
 */
void IAVertex::print()
{
    printf("v=[%g, %g, %g]\n", pLocalPosition.x(), pLocalPosition.y(), pLocalPosition.z());
}


/**
 Move a vertex from its original position along its inverted normal vector.
 Called for every vertex in a mesh, this function effectively shrinks the mesh.
 Make sure that the initial position is correct, and that all normal
 were calculated.
 */
//void IAVertex::shrinkBy(double s)
//{
//    pShrunkPosition.set(
//                  pGlobalPosition.x() - pGlobalNormal.x() * s,
//                  pGlobalPosition.y() - pGlobalNormal.y() * s,
//                  pGlobalPosition.z() - pGlobalNormal.z() * s
//                  );
//}


/**
 Project a texture onto this vertex in a mesh.
 */
void IAVertex::projectTexture(double w, double h, int type)
{
    switch (type) {
        case IA_PROJECTION_FRONT:
            pTex.set(pLocalPosition.x()/w+0.5, -pLocalPosition.z()/h+0.5, 0.0);
            break;
        case IA_PROJECTION_CYLINDER:
            break;
        case IA_PROJECTION_SPHERE:
            break;
    }
}

