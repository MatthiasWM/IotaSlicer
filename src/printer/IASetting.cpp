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


IASettingChoice::IASettingChoice(const char *path, int &value, std::function<void()>&& cb, Fl_Menu_Item *menu)
:   pPath(strdup(path)),
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

void IASettingChoice::wCallback(Fl_Choice *w, IASettingChoice *d)
{
    d->pValue = (int)(fl_intptr_t)(w->mvalue()->user_data());
    if (d->pCallback) d->pCallback();
}


void IASettingChoice::build()
{
    if (!pWidget) {
        pWidget = new Fl_Choice(1, 1, 120, 1);
        pWidget->textsize(11);
        pWidget->menu(pMenu);
        pWidget->value(pValue); // FIXME: select a menu item by index, should be set by user_data value
        pWidget->callback((Fl_Callback*)wCallback, this);
        pWidget->box(FL_FLAT_BOX);
    }
    pTreeItem = wSessionSettings->add(pPath);
    pTreeItem->widget(pWidget);
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IASettingFloatChoice::IASettingFloatChoice(const char *path, double &value, std::function<void()>&& cb, Fl_Menu_Item *menu)
:   pPath(strdup(path)),
    pValue(value),
    pCallback(cb),
    pMenu(dup(menu)),
    pWidget(nullptr)
{
}


IASettingFloatChoice::~IASettingFloatChoice()
{
    if (pPath) ::free((void*)pPath);
    if (pMenu) ::free((void*)pMenu);
    if (pWidget) delete pWidget;
}


void IASettingFloatChoice::wCallback(Fl_Input_Choice *w, IASettingFloatChoice *d)
{
    d->pValue = atof(w->value());
    if (d->pCallback) d->pCallback();
}


void IASettingFloatChoice::build()
{
    if (!pWidget) {
        pWidget = new Fl_Input_Choice(1, 1, 60, 1);
        pWidget->textsize(11);
        pWidget->labelsize(11);
        pWidget->menu(pMenu);
        char buf[80];
        snprintf(buf, 80, "%.2f", pValue);
        pWidget->value(buf);
        pWidget->callback((Fl_Callback*)wCallback, this);
    }
    pTreeItem = wSessionSettings->add(pPath);
    pTreeItem->widget(pWidget);
}





