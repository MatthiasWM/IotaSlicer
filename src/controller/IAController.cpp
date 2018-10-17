//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAController.h"

#include "view/IAGUIMain.h"
#include "view/IATreeItemView.h"
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


void IAController::autoDelete(bool v)
{
    pAutoDelete = v;
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IACallbackController::IACallbackController(IAProperty &prop, std::function<void()>&& cb)
:   pProperty( prop ),
    pCallback( cb )
{
    pProperty.attach(this);
}


IACallbackController::~IACallbackController()
{
    pProperty.detach(this);
}


void IACallbackController::propertyValueChanged()
{
    if (pCallback)
        pCallback();
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IATreeItemController::IATreeItemController(const char *path, const char *label)
:   pPath(strdup(path)),
    pLabel(strdup(label))
{
}


IATreeItemController::~IATreeItemController()
{
    if (pPath) ::free((void*)pPath);
    if (pLabel) ::free((void*)pLabel);
}


Fl_Menu_Item *IATreeItemController::dup(Fl_Menu_Item const *src)
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
:   IATreeItemController(path, label)
{
    if (text) pText = strdup(text);
}


IALabelController::~IALabelController()
{
    if (pText) ::free((void*)pText);
}


void IALabelController::build(Fl_Tree *treeWidget, Type t, int w)
{
    if (!pWidget) {
        pWidget = new IALabelView(t, w, pLabel, pText);
        pWidget->tooltip(pTooltip);
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
:   IATreeItemController(path, label),
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


void IAFloatController::build(Fl_Tree *treeWidget, Type t, int w)
{
    if (!pWidget) {
        pWidget = new IAFloatView(t, w, pLabel, pUnit);
        pWidget->value(pProperty());
        pWidget->callback((Fl_Callback*)wCallback, this);
        pWidget->tooltip(pTooltip);
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
:   IATreeItemController(path, label),
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


void IATextController::build(Fl_Tree *treeWidget, Type t, int w)
{
    if (!pWidget) {
        pWidget = new IATextView(t, w, pLabel, pUnit);
        pWidget->value(pProperty());
        pWidget->callback((Fl_Callback*)wCallback, this);
        pWidget->tooltip(pTooltip);
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
:   IATreeItemController(path, label),
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


void IAFloatChoiceController::build(Fl_Tree *treeWidget, Type t, int w)
{
    if (!pWidget) {
        pWidget = new IAFloatChoiceView(t, w, pLabel, pMenu, pUnit);
        pWidget->value( pProperty() );
        pWidget->callback((Fl_Callback*)wCallback, this);
        pWidget->tooltip(pTooltip);
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


IAChoiceController::IAChoiceController(const char *path, const char *label, IAIntProperty &prop,
                                       std::function<void()>&& cb, Fl_Menu_Item *menu)
:   IATreeItemController(path, label),
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


void IAChoiceController::build(Fl_Tree *treeWidget, Type t, int w)
{
    if (!pWidget) {
        pWidget = new IAChoiceView(t, w, pLabel, pMenu);
        // FIXME: compare to user_data() in the menu list to find the right entry
        pWidget->value(pProperty());
        pWidget->callback((Fl_Callback*)wCallback, this);
        pWidget->tooltip(pTooltip);
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
:   IATreeItemController(path, label),
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
    if (pText) ::free((void*)pText);
    if (pXPath) ::free((void*)pXPath);
    if (pYPath) ::free((void*)pYPath);
    if (pZPath) ::free((void*)pZPath);
}


void IAVectorController::build(Fl_Tree *treeWidget, Type t, int w)
{
    if (!pLabelWidget) {
        pLabelWidget = new IALabelView(t, w, pLabel, pText);
        pLabelWidget->tooltip(pTooltip);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pLabelWidget);

    if (pXLabel && !pXWidget) {
        pXWidget = new IAFloatView(t, w, pXLabel, pXUnits);
        pXWidget->value(pProperty().x());
        pXWidget->callback((Fl_Callback*)wCallback, this);
    }
    if (pXWidget) {
        if (!pXPath) pXPath = strdup(Fl_Preferences::Name("%s/x", pPath));
        pTreeItem = treeWidget->add(pXPath);
        pTreeItem->widget(pXWidget);
    }

    if (pYLabel && !pYWidget) {
        pYWidget = new IAFloatView(t, w, pYLabel, pYUnits);
        pYWidget->value(pProperty().y());
        pYWidget->callback((Fl_Callback*)wCallback, this);
    }
    if (pYWidget) {
        if (!pYPath) pYPath = strdup(Fl_Preferences::Name("%s/y", pPath));
        pTreeItem = treeWidget->add(pYPath);
        pTreeItem->widget(pYWidget);
    }

    if (pZLabel && !pZWidget) {
        pZWidget = new IAFloatView(t, w, pZLabel, pZUnits);
        pZWidget->value(pProperty().z());
        pZWidget->callback((Fl_Callback*)wCallback, this);
    }
    if (pZWidget) {
        if (!pZPath) pZPath = strdup(Fl_Preferences::Name("%s/z", pPath));
        pTreeItem = treeWidget->add(pZPath);
        pTreeItem->widget(pZWidget);
    }
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


