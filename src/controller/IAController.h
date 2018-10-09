//
//  IAController.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_CONTROLLER_H
#define IA_CONTROLLER_H


#include <vector>


class IAPropertyDouble;


class IAController
{
public:
    IAController();
    virtual ~IAController();

    virtual void propertyValueChanged();
};


class IAControllerDouble : public IAController
{
public:
    IAControllerDouble();
    virtual ~IAControllerDouble() override;

    virtual void propertyValueChanged() override;

protected:
    IAPropertyDouble *pProperty = nullptr;
};


#endif /* IA_PRINTER_LIST_CONTROLLER_H */


