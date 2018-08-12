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
 * A perspective camera for IAModelView.
 */
class IACamera
{
public:
    IACamera(IAModelView *view);
    void draw();
    void rotate(double dx, double dy);
    void drag(double dx, double dy);
    void dolly(double dx, double dy);

private:
    double pXRotation = 0.0;
    double pZRotation = 0.0;
    double pDistance = 400;
    IAVector3d pInterest = { 0.0, 0.0, 0.0 };
    IAModelView *pView = nullptr;
};


#endif /* IA_CAMERA_H */


