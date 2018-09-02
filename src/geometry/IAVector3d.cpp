//
//  IAVector3d.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#include "IAVector3d.h"

#include "IAMath.h"
#include <math.h>


/**
 Create a null vector.
 */
IAVector3d::IAVector3d()
{
}


/**
 Create a duplicate of another vector.
 */
IAVector3d::IAVector3d(const IAVector3d &v)
{
    pV[0] = v.pV[0];
    pV[1] = v.pV[1];
    pV[2] = v.pV[2];
}


/**
 Create a vector from double values in memory.
 */
IAVector3d::IAVector3d(double *v)
{
    pV[0] = v[0];
    pV[1] = v[1];
    pV[2] = v[2];
}


/**
 Create a vector form tree coordinates.
 */
IAVector3d::IAVector3d(double x, double y, double z)
{
    pV[0] = x;
    pV[1] = y;
    pV[2] = z;
}


/**
 * Assignment operator.
 */
IAVector3d &IAVector3d::operator=(const IAVector3d &rhs)
{
    if(this == &rhs)
        return *this;
    pV[0] = rhs.pV[0];
    pV[1] = rhs.pV[1];
    pV[2] = rhs.pV[2];
    return *this;
}


/**
 Set a vector form tree coordinates.
 */
void IAVector3d::set(double x, double y, double z)
{
    pV[0] = x;
    pV[1] = y;
    pV[2] = z;
}


/**
 * Set the lower value for each component of vector a and b.
 */
void IAVector3d::setMin(IAVector3d const& v)
{
    pV[0] = ia_min(pV[0], v.pV[0]);
    pV[1] = ia_min(pV[1], v.pV[1]);
    pV[2] = ia_min(pV[2], v.pV[2]);
}


/**
 * Set the higher value for each component of vector a and b.
 */
void IAVector3d::setMax(IAVector3d const& v)
{
    pV[0] = ia_max(pV[0], v.pV[0]);
    pV[1] = ia_max(pV[1], v.pV[1]);
    pV[2] = ia_max(pV[2], v.pV[2]);
}


/**
 Set a vector from float values in memory.
 */
void IAVector3d::read(float *v)
{
    pV[0] = v[0];
    pV[1] = v[1];
    pV[2] = v[2];
}


/**
 Set a vector from double values in memory.
 */
void IAVector3d::read(double *v)
{
    pV[0] = v[0];
    pV[1] = v[1];
    pV[2] = v[2];
}


/**
 Write vector coordinates to memory.
 */
void IAVector3d::write(double *v) const
{
    v[0] = pV[0];
    v[1] = pV[1];
    v[2] = pV[2];
}


/**
 Calculate the length of a vector.
 */
double IAVector3d::length() const
{
    return sqrt(pV[0]*pV[0]+pV[1]*pV[1]+pV[2]*pV[2]);
}


/**
 Modify the vector to be one unit long.
 */
double IAVector3d::normalize()
{
    double len = length();
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


/**
 Return a new normalized vector.
 */
IAVector3d IAVector3d::normalized()
{
    IAVector3d v = *this;
    v.normalize();
    return v;
}


/**
 Subtract another vector from this vector.
 */
IAVector3d& IAVector3d::operator-=(const IAVector3d &v)
{
    pV[0] -= v.pV[0];
    pV[1] -= v.pV[1];
    pV[2] -= v.pV[2];
    return *this;
}


/**
 * Add two vectors.
 */
IAVector3d IAVector3d::operator+(const IAVector3d &rhs) const
{
    IAVector3d sum(this->pV[0] + rhs.pV[0],
                   this->pV[1] + rhs.pV[1],
                   this->pV[2] + rhs.pV[2]);
    return sum;
}


/**
 * Subtract two vectors.
 */
IAVector3d IAVector3d::operator-(const IAVector3d &rhs) const
{
    IAVector3d diff(this->pV[0] - rhs.pV[0],
                    this->pV[1] - rhs.pV[1],
                    this->pV[2] - rhs.pV[2]);
    return diff;
}


/**
 * Multiply vector with scalar
 */
IAVector3d IAVector3d::operator*(double scl) const
{
    IAVector3d ret(this->pV[0] * scl,
                   this->pV[1] * scl,
                   this->pV[2] * scl);
    return ret;
}


/**
 Add another vector to this vector.
 */
IAVector3d& IAVector3d::operator+=(const IAVector3d &v)
{
    pV[0] += v.pV[0];
    pV[1] += v.pV[1];
    pV[2] += v.pV[2];
    return *this;
}


/**
 Multiply this vector with a scalar.
 */
IAVector3d& IAVector3d::operator*=(double n)
{
    pV[0] *= n;
    pV[1] *= n;
    pV[2] *= n;
    return *this;
}


/**
 Set this vector to the cross product with another vector.
 */
IAVector3d& IAVector3d::cross(const IAVector3d &b)
{
    IAVector3d a(*this);
    pV[0] = a.pV[1]*b.pV[2] - a.pV[2]*b.pV[1];
    pV[1] = a.pV[2]*b.pV[0] - a.pV[0]*b.pV[2];
    pV[2] = a.pV[0]*b.pV[1] - a.pV[1]*b.pV[0];
    return *this;
}


/**
 Set this vector to zero.
 */
void IAVector3d::zero()
{
    pV[0] = 0.0;
    pV[1] = 0.0;
    pV[2] = 0.0;
}


/**
 Rotate vector around x axis by angle a in rad.
 */
void IAVector3d::xRotate(double a)
{
    double s = sin(a);
    double c = cos(a);

    double yy = pV[1] * c - pV[2] * s;
    double zz = pV[1] * s + pV[2] * c;

    pV[1] = yy;
    pV[2] = zz;
}


/**
 Rotate vector around y axis by angle a in rad.
 */
void IAVector3d::yRotate(double a)
{
    double s = sin(a);
    double c = cos(a);

    double zz = pV[2] * c - pV[0] * s;
    double xx = pV[2] * s + pV[0] * c;

    pV[2] = zz;
    pV[0] = xx;
}


/**
 Rotate vector around z axis by angle a in rad.
 */
void IAVector3d::zRotate(double a)
{
    double s = sin(a);
    double c = cos(a);

    double xx = pV[0] * c - pV[1] * s;
    double yy = pV[0] * s + pV[1] * c;

    pV[0] = xx;
    pV[1] = yy;
}


/**
 * Check if two vectors are equal with 1e-7 range.
 */
bool IAVector3d::operator==(const IAVector3d &rhs) const
{
    // TODO: this should actually look at the IEEE representation and ignore the last two bit of the mantissa
    if (fabs(pV[0]-rhs.pV[0])>1e-7) return false;
    if (fabs(pV[1]-rhs.pV[1])>1e-7) return false;
    if (fabs(pV[2]-rhs.pV[2])>1e-7) return false;
    return true;
}



