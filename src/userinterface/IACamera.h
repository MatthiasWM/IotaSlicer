//
//  IACamera.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_CAMERA_H
#define IA_CAMERA_H

#include "geometry/IAVector3d.h"


class IASceneView;


/**
 * Base class for all cameras.
 */
class IACamera
{
public:
    IACamera(IASceneView *view);
    virtual ~IACamera() { }
    virtual void draw() = 0;
    virtual void rotate(double dx, double dy) { }
    virtual void drag(double dx, double dy) { }
    virtual void dolly(double dx, double dy) { }
    virtual void setInterest(IAVector3d &v) { }

protected:
    /** Link back to the scene viewer */
    IASceneView *pView = nullptr;
};


/**
 * A perspective camera for IASceneView.
 */
class IAPerspectiveCamera : public IACamera
{
    typedef IACamera super;
public:
    IAPerspectiveCamera(IASceneView *view);
    void draw() override;
    void rotate(double dx, double dy) override;
    void drag(double dx, double dy) override;
    void dolly(double dx, double dy) override;
    virtual void setInterest(IAVector3d &v) override;

private:
    /** camera rotation around the vertical axis */
    double pXRotation = -0.3;
    /** camera height over interest point */
    double pZRotation = 0.3;
    /** distance from the point of interest */
    double pDistance = 400;
    /** coordinates of target and center of rotation */
    IAVector3d pInterest = { 0.0, 0.0, 0.0 };
};


/**
 * An orthogonal camera for IASceneView.
 */
class IAOrthoCamera : public IACamera
{
    typedef IACamera super;
public:
    IAOrthoCamera(IASceneView *view, int direction);
    void draw() override;
    void rotate(double dx, double dy) override;
    void drag(double dx, double dy) override;
    void dolly(double dx, double dy) override;
    virtual void setInterest(IAVector3d &v) override;

private:
    /** Scaling of the view. */
    double pZoom = 200;
    /** coordinates of target and center of zoom */
    IAVector3d pInterest = { 0.0, 0.0, 0.0 };
};


#endif /* IA_CAMERA_H */


