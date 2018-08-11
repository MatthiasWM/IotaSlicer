//
//  IAVector3d.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#include "IAVector3d.h"

#include <math.h>


IAVector3d::IAVector3d()
{
}

IAVector3d::IAVector3d(const IAVector3d &v)
{
    pV[0] = v.pV[0];
    pV[1] = v.pV[1];
    pV[2] = v.pV[2];
}

IAVector3d::IAVector3d(double *v)
{
    pV[0] = v[0];
    pV[1] = v[1];
    pV[2] = v[2];
}

IAVector3d::IAVector3d(double x, double y, double z)
{
    pV[0] = x;
    pV[1] = y;
    pV[2] = z;
}

void IAVector3d::set(double x, double y, double z)
{
    pV[0] = x;
    pV[1] = y;
    pV[2] = z;
}

void IAVector3d::read(float *v)
{
    pV[0] = v[0];
    pV[1] = v[1];
    pV[2] = v[2];
}

void IAVector3d::read(double *v)
{
    pV[0] = v[0];
    pV[1] = v[1];
    pV[2] = v[2];
}

void IAVector3d::write(double *v)
{
    v[0] = pV[0];
    v[1] = pV[1];
    v[2] = pV[2];
}

double IAVector3d::normalize()
{
    double len = sqrt(pV[0]*pV[0]+pV[1]*pV[1]+pV[2]*pV[2]);
    if (len==0.0) {
        len = 1.0;
    } else {
        len = 1.0/len;
    }
    pV[0] *= len;
    pV[1] *= len;
    pV[2] *= len;
    return len;
}

IAVector3d& IAVector3d::operator-=(const IAVector3d &v)
{
    pV[0] -= v.pV[0];
    pV[1] -= v.pV[1];
    pV[2] -= v.pV[2];
    return *this;
}

IAVector3d& IAVector3d::operator+=(const IAVector3d &v)
{
    pV[0] += v.pV[0];
    pV[1] += v.pV[1];
    pV[2] += v.pV[2];
    return *this;
}

IAVector3d& IAVector3d::operator*=(double n)
{
    pV[0] *= n;
    pV[1] *= n;
    pV[2] *= n;
    return *this;
}

IAVector3d& IAVector3d::cross(const IAVector3d &b)
{
    IAVector3d a(*this);
    pV[0] = a.pV[1]*b.pV[2] - a.pV[2]*b.pV[1];
    pV[1] = a.pV[2]*b.pV[0] - a.pV[0]*b.pV[2];
    pV[2] = a.pV[0]*b.pV[1] - a.pV[1]*b.pV[0];
    return *this;
}

void IAVector3d::zero()
{
    pV[0] = 0.0;
    pV[1] = 0.0;
    pV[2] = 0.0;
}

