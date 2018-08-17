//
//  IACamera.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_CAMERA_H
#define IA_CAMERA_H

#include "../geometry/IAVector3d.h"


class IAModelView;


/**
 * Base class for all cameras.
 */
class IACamera
{
public:
    IACamera(IAModelView *view);
    virtual ~IACamera() { }
    virtual void draw() = 0;
    virtual void rotate(double dx, double dy) { }
    virtual void drag(double dx, double dy) { }
    virtual void dolly(double dx, double dy) { }

protected:
    IAModelView *pView = nullptr;
};


/**
 * A perspective camera for IAModelView.
 */
class IAPerspectiveCamera : public IACamera
{
    typedef IACamera super;
public:
    IAPerspectiveCamera(IAModelView *view);
    void draw() override;
    void rotate(double dx, double dy) override;
    void drag(double dx, double dy) override;
    void dolly(double dx, double dy) override;

private:
#if 1
    double pXRotation = -0.55;
    double pZRotation = 5.81;
    double pDistance = 216;
#else
    double pXRotation = 0.0;
    double pZRotation = 0.0;
    double pDistance = 400;
#endif
    IAVector3d pInterest = { 0.0, 0.0, 0.0 };
};


/**
 * An orthogonal camera for IAModelView.
 */
class IAOrthoCamera : public IACamera
{
    typedef IACamera super;
public:
    IAOrthoCamera(IAModelView *view, int direction);
    void draw() override;
    void rotate(double dx, double dy) override;
    void drag(double dx, double dy) override;
    void dolly(double dx, double dy) override;

private:
    double pZoom = 200;
    IAVector3d pInterest = { 0.0, 0.0, 0.0 };
};


#endif /* IA_CAMERA_H */

