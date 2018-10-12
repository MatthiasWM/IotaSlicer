//
//  IAController.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_CONTROLLER_H
#define IA_CONTROLLER_H


#include "Iota.h"

#include <vector>


class IACtrlTreeItemFloat;
class IAPropertyFloat;
struct Fl_Menu_Item;


class IAController
{
public:
    IAController();
    virtual ~IAController();

    virtual void propertyValueChanged();
};


class IAMenuItemEventController : public IAController
{
public:
    IAMenuItemEventController(IAPropertyEvent &p) : pProperty( p ) { }
    virtual void propertyValueChanged() override;
    void trigger();
protected:
    IAPropertyEvent &pProperty;
    Fl_Menu_Item *pView = nullptr;
};


/**
 * This controller manages the connection between a Floating Point Property
 * and a Floating Point Tree View.
 */
class IACtrlTreeItemFloat : public IAController
{
public:
    IACtrlTreeItemFloat();
    virtual ~IACtrlTreeItemFloat() override;

    virtual void propertyValueChanged() override;

protected:
    IAPropertyFloat *pProperty = nullptr;
    IACtrlTreeItemFloat *pView = nullptr;
};


class IAAppController : public IAController
{
    IAAppController() = delete;
public:
    IAAppController(IAIota &app) : pApp( app ) { }
    IAMenuItemEventController quit { pApp.propQuit };
protected:
    IAIota &pApp;
};


#endif /* IA_PRINTER_LIST_CONTROLLER_H */


