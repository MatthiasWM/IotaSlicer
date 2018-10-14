//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAController.h"

#include "view/IAGUIMain.h"
#include "view/IATreeView.h"
#include "property/IAProperty.h"

#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Preferences.H>
#include <FL/filename.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>
#include <FL/Fl_Float_Input.H>


IAController::IAController()
{
}


IAController::~IAController()
{
}


void IAController::propertyValueChanged()
{
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IATreeViewController::IATreeViewController(const char *path, const char *label)
:   pPath(strdup(path)),
pLabel(strdup(label))
{
}


IATreeViewController::~IATreeViewController()
{
    if (pPath) ::free((void*)pPath);
    if (pLabel) ::free((void*)pLabel);
}


Fl_Menu_Item *IATreeViewController::dup(Fl_Menu_Item const *src)
{
    Fl_Menu_Item const *s = src;
    int n = 1;
    while (s->label()!=nullptr) {
        s++; n++;
        // we assum that there are no submenus
    }
    Fl_Menu_Item *ret = (Fl_Menu_Item*)malloc(n*sizeof(Fl_Menu_Item));
    memmove(ret, src, n*sizeof(Fl_Menu_Item));
    return ret;
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IALabelController::IALabelController(const char *path, const char *label, const char *text)
:   IATreeViewController(path, label)
{
    if (text) pText = strdup(text);
}


IALabelController::~IALabelController()
{
    if (pText) ::free((void*)pText);
}


void IALabelController::build(Fl_Tree *treeWidget, Type t)
{
    if (!pWidget) {
        pWidget = new IALabelView(t, treeWidget->w()-40, pLabel);
        if (pText) pWidget->pText->label(pText);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pWidget);
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//

IAFloatController::IAFloatController(const char *path, const char *label,
                                     IAFloatProperty &prop, const char *unit,
                                     std::function<void()>&& cb)
:   IATreeViewController(path, label),
pProperty(prop),
pUnit(strdup(unit)),
pCallback(cb),
pWidget(nullptr)
{
    pProperty.attach(this);
}


IAFloatController::~IAFloatController()
{
    pProperty.detach(this);
    if (pUnit) ::free((void*)pUnit);
    if (pWidget) delete pWidget;
}


void IAFloatController::wCallback(IAFloatView *w, IAFloatController *d)
{
    d->pProperty.set( w->value(), d );
    if (d->pCallback)
        d->pCallback();
}


void IAFloatController::build(Fl_Tree *treeWidget, Type t)
{
    if (!pWidget) {
        pWidget = new IAFloatView(t, treeWidget->w()-40, pLabel);
        pWidget->pInput->label(pUnit);
        pWidget->value(pProperty());
        pWidget->callback((Fl_Callback*)wCallback, this);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pWidget);
}

void IAFloatController::propertyValueChanged()
{
    if (pWidget)
        pWidget->value(pProperty());
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//

IATextController::IATextController(const char *path, const char *label, IATextProperty &prop,
                                   int wdt, const char *unit,
                                   std::function<void()>&& cb)
:   IATreeViewController(path, label),
pProperty(prop),
pWdt(wdt),
pUnit(strdup(unit)),
pCallback(cb),
pWidget(nullptr)
{
    pProperty.attach(this);
}


IATextController::~IATextController()
{
    pProperty.detach(this);
    if (pUnit) ::free((void*)pUnit);
    if (pWidget) delete pWidget;
}


void IATextController::wCallback(IATextView *w, IATextController *d)
{
    d->pProperty.set( w->value(), d );
    if (d->pCallback)
        d->pCallback();
}


void IATextController::build(Fl_Tree *treeWidget, Type t)
{
    if (!pWidget) {
        pWidget = new IATextView(t, treeWidget->w()-40, pLabel);
        pWidget->pInput->label(pUnit);
        pWidget->value(pProperty());
        pWidget->callback((Fl_Callback*)wCallback, this);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pWidget);
}

void IATextController::propertyValueChanged()
{
    if (pWidget)
        pWidget->value(pProperty());
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//

IAFloatChoiceController::IAFloatChoiceController(const char *path, const char *label, IAFloatProperty &prop, const char *unit,
                                                 std::function<void()>&& cb, Fl_Menu_Item *menu)
:   IATreeViewController(path, label),
pProperty(prop),
pUnit(strdup(unit)),
pCallback(cb),
pMenu(dup(menu)),
pWidget(nullptr)
{
    pProperty.attach(this);
}


IAFloatChoiceController::~IAFloatChoiceController()
{
    pProperty.detach(this);
    if (pUnit) ::free((void*)pUnit);
    if (pMenu) ::free((void*)pMenu);
    if (pWidget) delete pWidget;
}


void IAFloatChoiceController::wCallback(IAFloatChoiceView *w, IAFloatChoiceController *d)
{
    d->pProperty.set( w->value(), d );
    if (d->pCallback)
        d->pCallback();
}


void IAFloatChoiceController::build(Fl_Tree *treeWidget, Type t)
{
    if (!pWidget) {
        pWidget = new IAFloatChoiceView(t, treeWidget->w()-40, pLabel);
        pWidget->pChoice->menu(pMenu);
        pWidget->pChoice->label(pUnit);
        pWidget->value( pProperty() );
        pWidget->callback((Fl_Callback*)wCallback, this);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pWidget);
}

void IAFloatChoiceController::propertyValueChanged()
{
    if (pWidget)
        pWidget->value(pProperty());
}



#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IAChoiceController::IAChoiceController(
                                       const char *path, const char *label, IAIntProperty &prop,
                                       std::function<void()>&& cb, Fl_Menu_Item *menu)
:   IATreeViewController(path, label),
    pProperty(prop),
    pMenu(dup(menu)),
    pCallback(cb),
    pWidget(nullptr)
{
    pProperty.attach(this);
}


IAChoiceController::~IAChoiceController()
{
    pProperty.detach(this);
    if (pMenu) ::free((void*)pMenu);
    if (pWidget) delete pWidget;
}


void IAChoiceController::wCallback(IAChoiceView *w, IAChoiceController *d)
{
    d->pProperty.set( w->value(), d );
    if (d->pCallback)
        d->pCallback();
}


void IAChoiceController::build(Fl_Tree *treeWidget, Type)
{
    if (!pWidget) {
        pWidget = new IAChoiceView(0, 0, 200, 13, pLabel);
        pWidget->pChoice->menu(pMenu);
        // FIXME: compare to user_data() in the menu list to find the right entry
        pWidget->value(pProperty());
        pWidget->callback((Fl_Callback*)wCallback, this);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pWidget);
}


void IAChoiceController::propertyValueChanged()
{
    if (pWidget)
        pWidget->value(pProperty());
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IAVectorController::IAVectorController(const char *path, const char *label, const char *text,
                                       IAVectorProperty &prop,
                                       char const* xLabel, char const* xUnits,
                                       char const* yLabel, char const* yUnits,
                                       char const* zLabel, char const* zUnits,
                                       std::function<void()>&& cb)
:   IATreeViewController(path, label),
    pProperty(prop),
    pCallback(cb),
    pXLabel(xLabel), pXUnits(xUnits),
    pYLabel(yLabel), pYUnits(yUnits),
    pZLabel(zLabel), pZUnits(zUnits)
{
    if (text) pText = strdup(text);
    pProperty.attach(this);
}


IAVectorController::~IAVectorController()
{
    pProperty.detach(this);
    if (pLabelWidget) delete pLabelWidget;
    if (pXWidget) delete pXWidget;
    if (pYWidget) delete pYWidget;
    if (pZWidget) delete pZWidget;
}


void IAVectorController::build(Fl_Tree *treeWidget, Type t)
{
    if (!pLabelWidget) {
        pLabelWidget = new IALabelView(t, treeWidget->w()-40, pLabel);
        if (pText) pLabelWidget->pText->label(pText);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pLabelWidget);

    if (!pWidget) {
        pWidget = new IAFloatView(t, treeWidget->w()-40, pLabel);
        pWidget->pInput->label(pUnit);
        pWidget->value(pProperty());
        pWidget->callback((Fl_Callback*)wCallback, this);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pWidget);
    // add x, y, and z
}


void IAVectorController::propertyValueChanged()
{
    if (pXWidget) pXWidget->value(pProperty().x());
    if (pYWidget) pYWidget->value(pProperty().y());
    if (pZWidget) pZWidget->value(pProperty().z());
}


void IAVectorController::wCallback(IAFloatView *w, IAVectorController *d)
{
    IAVector3d v;
    if (d->pXWidget) v.x( d->pXWidget->value() );
    if (d->pYWidget) v.y( d->pYWidget->value() );
    if (d->pZWidget) v.z( d->pZWidget->value() );
    d->pProperty.set( v, d );
    if (d->pCallback)
        d->pCallback();
}


