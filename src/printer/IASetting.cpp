//
//  IASetting.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IASetting.h"

#include "userinterface/IAGUIMain.h"

#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Preferences.H>
#include <FL/filename.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>



IASetting::IASetting()
{
}


IASetting::~IASetting()
{
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


class IAFLChoice : public Fl_Group
{
public:
    IAFLChoice(int x, int y, int w, int h, const char *label=nullptr)
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
    ~IAFLChoice() { }
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


IASettingChoice::IASettingChoice(
    const char *path, const char *label, int &value,
    std::function<void()>&& cb, Fl_Menu_Item *menu)
:   pPath(strdup(path)),
    pLabel(strdup(label)),
    pValue(value),
    pCallback(cb),
    pMenu(dup(menu)),
    pWidget(nullptr)
{
}


IASettingChoice::~IASettingChoice()
{
    if (pPath) ::free((void*)pPath);
    if (pMenu) ::free((void*)pMenu);
    if (pWidget) delete pWidget;
}

void IASettingChoice::wCallback(IAFLChoice *w, IASettingChoice *d)
{
    d->pValue = w->value();
    if (d->pCallback) d->pCallback();
}


void IASettingChoice::build()
{
    if (!pWidget) {
        pWidget = new IAFLChoice(0, 0, 200, 13, pLabel);
        pWidget->pChoice->menu(pMenu);
        pWidget->value(pValue);
        pWidget->callback((Fl_Callback*)wCallback, this);
    }
    pTreeItem = wSessionSettings->add(pPath);
    pTreeItem->widget(pWidget);
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//

class IAFLFloatChoice : public Fl_Group
{
public:
    IAFLFloatChoice(int x, int y, int w, int h, const char *label=nullptr)
    :   Fl_Group(x, y, w, h, label) {
        begin();
        align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
        labelsize(11);
        box(FL_FLAT_BOX);
        pChoice = new Fl_Input_Choice(x+100, y, w-135, h);
        // FIXME: calculate sizes using menu entries and unit string length
        pChoice->textsize(11);
        pChoice->labelsize(11);
        pChoice->align(FL_ALIGN_RIGHT);
        pChoice->callback((Fl_Callback*)choice_cb);
        end();
    }
    ~IAFLFloatChoice() { }
    Fl_Input_Choice *pChoice = nullptr;
    double value() { return atof(pChoice->value()); }
    void value(double v) {
        char buf[80];
        snprintf(buf, 80, "%.2f", v);
        pChoice->value(buf);
    }
    static void choice_cb(Fl_Input_Choice *w, void *u) {
        w->parent()->do_callback();
    }
};


IASettingFloatChoice::IASettingFloatChoice(
    const char *path, const char *label, double &value, const char *unit,
    std::function<void()>&& cb, Fl_Menu_Item *menu)
:   pPath(strdup(path)),
    pLabel(strdup(label)),
    pValue(value),
    pUnit(strdup(unit)),
    pCallback(cb),
    pMenu(dup(menu)),
    pWidget(nullptr)
{
}


IASettingFloatChoice::~IASettingFloatChoice()
{
    if (pPath) ::free((void*)pPath);
    if (pLabel) ::free((void*)pLabel);
    if (pUnit) ::free((void*)pUnit);
    if (pMenu) ::free((void*)pMenu);
    if (pWidget) delete pWidget;
}


void IASettingFloatChoice::wCallback(IAFLFloatChoice *w, IASettingFloatChoice *d)
{
    d->pValue = w->value();
    if (d->pCallback) d->pCallback();
}


void IASettingFloatChoice::build()
{
    if (!pWidget) {
        pWidget = new IAFLFloatChoice(0, 0, 200, 13, pLabel);
        pWidget->pChoice->menu(pMenu);
        pWidget->pChoice->label(pUnit);
        pWidget->value(pValue);
        pWidget->callback((Fl_Callback*)wCallback, this);
    }
    pTreeItem = wSessionSettings->add(pPath);
    pTreeItem->widget(pWidget);
}





