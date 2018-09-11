//
//  IAVector3d.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#ifndef IA_VECTOR3D_H
#define IA_VECTOR3D_H


/**
 A small class to handle vectors in 3D space.
 */
class IAVector3d
{
public:
    IAVector3d();
    IAVector3d(const IAVector3d&);
    IAVector3d(double*);
    IAVector3d(double, double, double);
    bool operator==(const IAVector3d &rhs) const;
    /** Compare vectors.
     \param rhs compare to me
     \return true, if vectors are different. */
    bool operator!=(const IAVector3d &rhs) const { return (!(operator==(rhs))); }
    IAVector3d &operator=(const IAVector3d &rhs);
    IAVector3d operator+(const IAVector3d &rhs) const;
    IAVector3d operator-(const IAVector3d &rhs) const;
    IAVector3d operator*(double) const;
    IAVector3d operator^(const IAVector3d&) const;

    IAVector3d& operator-=(const IAVector3d&);
    IAVector3d& operator+=(const IAVector3d&);
    IAVector3d& operator*=(double);

    /** Raw accees for OpenGL.
     \return pointer to array of three double values */
    double *dataPointer() { return pV; }
    void setMin(const IAVector3d&);
    void setMax(const IAVector3d&);
    double normalize();
    IAVector3d normalized();
    void setZero();
    void read(float*);
    void read(double*);
    double length() const;

    /** x coordinate of vector.
     \return x coordinate */
    double x() const { return pV[0]; }

    /** y coordinate of vector.
     \return y coordinate */
    double y() const { return pV[1]; }

    /** z coordinate of vector.
     \return z coordinate */
    double z() const { return pV[2]; }

    /** Set the x coordinate of the vector.
     \param v new coordinate */
    void x(double v) { pV[0] = v; }

    /** Set the y coordinate of the vector.
     \param v new coordinate */
    void y(double v) { pV[1] = v; }

    /** Set the z coordinate of the vector.
     \param v new coordinate */
    void z(double v) { pV[2] = v; }

    void set(double, double, double);
    void xRotate(double);
    void yRotate(double);
    void zRotate(double);

private:
    /** Coordinates are stored in an array for fast access through OpenGL. */
    double pV[3] = { 0.0, 0.0, 0.0 };
};


#endif /* IA_VECTOR3D_H */
