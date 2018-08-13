//
//  geometry.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAVertex.h"

#include "main.h"


IAVertex::IAVertex()
{
}

IAVertex::IAVertex(const IAVertex *v)
{
    pInitialPosition = v->pInitialPosition;
    pPosition = v->pPosition;
    pNormal = v->pNormal;
    pTex = v->pTex;
    pNNormal = v->pNNormal;
}

void IAVertex::addNormal(const IAVector3d &v)
{
    IAVector3d vn(v);
    vn.normalize();
    pNormal += vn;
    pNNormal++;
}

void IAVertex::averageNormal()
{
    if (pNNormal>0) {
        double len = 1.0/pNNormal;
        pNormal *= len;
    }
}

void IAVertex::print()
{
    printf("v=[%g, %g, %g]\n", pPosition.x(), pPosition.y(), pPosition.z());
}

void IAVertex::shrinkBy(double s)
{
    pPosition.set(
                  pInitialPosition.x() - pNormal.x() * s,
                  pInitialPosition.y() - pNormal.y() * s,
                  pInitialPosition.z() - pNormal.z() * s
                  );
}

void IAVertex::projectTexture(double w, double h, int type)
{
    switch (type) {
        case IA_PROJECTION_FRONT:
            pTex.set(pPosition.x()/w+0.5, -pPosition.z()/h+0.5, 0.0);
            break;
        case IA_PROJECTION_CYLINDER:
            break;
        case IA_PROJECTION_SPHERE:
            break;
    }
}

