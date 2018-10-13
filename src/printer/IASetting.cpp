//
//  IASetting.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IASetting.h"

#include "view/IAGUIMain.h"
#include "property/IAProperty.h"

#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Preferences.H>
#include <FL/filename.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>
#include <FL/Fl_Float_Input.H>



IASetting::IASetting(const char *path, const char *label)
:   pPath(strdup(path)),
    pLabel(strdup(label))
{
}


IASetting::~IASetting()
{
    if (pPath) ::free((void*)pPath);
    if (pLabel) ::free((void*)pLabel);
}


Fl_Menu_Item *IASetting::dup(Fl_Menu_Item const *src)
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


class IALabelView : public Fl_Group
{
public:
    IALabelView(IASetting::Type t, int w, const char *label=nullptr)
    :   Fl_Group(0, 0, w, t==IASetting::kSetting?13:15) {
        box(FL_FLAT_BOX);
        begin();
        pLabel = new Fl_Box(0, 0, w/2-4, h());
        pLabel->labelsize(h()-2);
        pLabel->label(label);
        pText = new Fl_Box(w/2, 0, w/2-4, h());
        pText->labelsize(h()-2);
        pText->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
        if (t==IASetting::kSetting) {
            pLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
        } else {
            pLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_RIGHT);
            color(parent()->color());
            pLabel->color(parent()->color());
        }
        end();
    }
    ~IALabelView() { }
    Fl_Box *pLabel;
    Fl_Box *pText;
};


IALabelController::IALabelController(const char *path, const char *label, const char *text)
:   IASetting(path, label)
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

class IAFloatView : public Fl_Group
{
public:
    IAFloatView(IASetting::Type t, int w, const char *label=nullptr)
    :   Fl_Group(0, 0, w, t==IASetting::kSetting?13:15) {
        box(FL_FLAT_BOX);
        begin();
        pLabel = new Fl_Box(0, 0, w/2-4, h());
        pLabel->labelsize(h()-2);
        pLabel->label(label);
        pInput = new Fl_Float_Input(w/2, 0, 65, h());
        pInput->textsize(h()-2);
        pInput->labelsize(h()-2);
        pInput->align(FL_ALIGN_RIGHT);
        pInput->callback((Fl_Callback*)choice_cb);
        if (t==IASetting::kSetting) {
            pLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
        } else {
            pLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_RIGHT);
            color(parent()->color());
            pLabel->color(parent()->color());
            pInput->color(parent()->color());
        }
        end();
    }
    ~IAFloatView() { }
    double value() { return atof(pInput->value()); }
    void value(double v) {
        char buf[80];
        snprintf(buf, 80, "%.2f", v);
        pInput->value(buf);
    }
    static void choice_cb(Fl_Float_Input *w, void *u) {
        w->parent()->do_callback();
    }
    Fl_Box *pLabel = nullptr;
    Fl_Float_Input *pInput = nullptr;
};


IAFloatController::IAFloatController(const char *path, const char *label,
                                     IAFloatProperty &prop, const char *unit,
                                     std::function<void()>&& cb)
:   IASetting(path, label),
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

class IATextView : public Fl_Group
{
public:
    IATextView(IASetting::Type t, int w, const char *label=nullptr)
    :   Fl_Group(0, 0, w, t==IASetting::kSetting?13:15) {
        box(FL_FLAT_BOX);
        begin();
        pLabel = new Fl_Box(0, 0, w/2-4, h());
        pLabel->labelsize(h()-2);
        pLabel->label(label);
        pInput = new Fl_Input(w/2, 0, h()*12, h());
        pInput->textsize(h()-2);
        pInput->labelsize(h()-2);
        pInput->align(FL_ALIGN_RIGHT);
        pInput->callback((Fl_Callback*)choice_cb);
        if (t==IASetting::kSetting) {
            pLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
        } else {
            pLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_RIGHT);
            color(parent()->color());
            pLabel->color(parent()->color());
            pInput->color(parent()->color());
        }
        end();
    }
    ~IATextView() { }
    const char *value() { return pInput->value(); }
    void value(char const* v) { pInput->value(v); }
    static void choice_cb(Fl_Choice *w, void *u) {
        w->parent()->do_callback();
    }
    Fl_Box *pLabel = nullptr;
    Fl_Input *pInput = nullptr;
};


IATextController::IATextController(const char *path, const char *label, IATextProperty &prop,
                                   int wdt, const char *unit,
                                   std::function<void()>&& cb)
:   IASetting(path, label),
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

class IAFloatChoiceView : public Fl_Group
{
public:
    IAFloatChoiceView(IASetting::Type t, int w, const char *label=nullptr)
    :   Fl_Group(0, 0, w, t==IASetting::kSetting?13:15) {
        box(FL_FLAT_BOX);
        begin();
        pLabel = new Fl_Box(0, 0, w/2-4, h());
        pLabel->labelsize(h()-2);
        pLabel->label(label);
        pChoice = new Fl_Input_Choice(w/2, 0, 65, h());
        pChoice->textsize(h()-2);
        pChoice->labelsize(h()-2);
        pChoice->align(FL_ALIGN_RIGHT);
        pChoice->callback((Fl_Callback*)choice_cb);
        if (t==IASetting::kSetting) {
            pLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
        } else {
            pLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_RIGHT);
            color(parent()->color());
            pLabel->color(parent()->color());
            pChoice->color(parent()->color());
        }
        end();
    }
    ~IAFloatChoiceView() { }
    double value() { return atof(pChoice->value()); }
    void value(double v) {
        char buf[80];
        snprintf(buf, 80, "%.2f", v);
        pChoice->value(buf);
    }
    static void choice_cb(Fl_Input_Choice *w, void *u) {
        IAFloatChoiceView *c = (IAFloatChoiceView*)w->parent();
        //c->value( c->value() ); // reinterprete the string that was set by the pulldown
        c->do_callback();
    }
    Fl_Box *pLabel = nullptr;
    Fl_Input_Choice *pChoice = nullptr;
};


IAFloatChoiceController::IAFloatChoiceController(const char *path, const char *label, IAFloatProperty &prop, const char *unit,
                                     std::function<void()>&& cb, Fl_Menu_Item *menu)
:   IASetting(path, label),
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


class IAChoiceView : public Fl_Group
{
public:
    IAChoiceView(int x, int y, int w, int h, const char *label=nullptr)
    :   Fl_Group(x, y, w, h, label) {
        begin();
        align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
        labelsize(11);
        box(FL_FLAT_BOX);
        pChoice = new Fl_Choice(x+100, y, w-100, h);
        // FIXME: calculate sizes using menu entries and unit string length
        pChoice->textsize(11);
        pChoice->labelsize(11);
        pChoice->align(FL_ALIGN_RIGHT);
        pChoice->callback((Fl_Callback*)choice_cb);
        end();
    }
    ~IAChoiceView() { }
    Fl_Choice *pChoice = nullptr;
    int value() {
        return (int)(fl_intptr_t)(pChoice->mvalue()->user_data());
    }
    void value(int v) {
        const Fl_Menu_Item *m = pChoice->menu();
        void *p = (void*)(fl_intptr_t)v;
        for (int i=0; ; i++) {
            if (m[i].label()==nullptr) break;
            if (m[i].user_data()==p) {
                pChoice->value(m+i);
                break;
            }
        }
    }
    static void choice_cb(Fl_Choice *w, void *u) {
        w->parent()->do_callback();
    }
};




IAChoiceController::IAChoiceController(
                           const char *path, const char *label, IAIntProperty &prop,
                           std::function<void()>&& cb, Fl_Menu_Item *menu)
:   IASetting(path, label),
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







