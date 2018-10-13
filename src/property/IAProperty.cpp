//
//  IAProperty.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAProperty.h"

#include "controller/IAController.h"

#include <FL/Fl_Preferences.H>

#include <algorithm>


IAPropertyList::IAPropertyList()
{
}


IAPropertyList::~IAPropertyList()
{
}


void IAPropertyList::add(IAProperty *prop)
{
    pList.push_back(prop);
}



IAProperty::IAProperty(const char *name)
:   pName(name)
{
}


IAProperty::~IAProperty()
{
}


void IAProperty::attach(IAController *ctrl)
{
    pControlerList.push_back(ctrl);
}


void IAProperty::detach(IAController* ctrl)
{
    auto c = std::find(pControlerList.begin(), pControlerList.end(), ctrl);
    if (c!=pControlerList.end())
        pControlerList.erase(c);
}



IAFloatProperty::IAFloatProperty(const char *name, double value)
:   IAProperty(name),
    pValue(value)
{
}


IAFloatProperty::~IAFloatProperty()
{
}


void IAFloatProperty::set(double v, IAController *ctrl)
{
    if (pValue!=v) {
        pValue = v;
        for (auto &c: pControlerList) {
            if (c!=ctrl)
                c->propertyValueChanged();
        }
    }
}

void IAFloatProperty::read(Fl_Preferences &prefs)
{
    double v;
    prefs.get(pName, v, pValue);
    set(v);
}

void IAFloatProperty::write(Fl_Preferences &prefs)
{
    prefs.set(pName, pValue);
}





IAIntProperty::IAIntProperty(const char *name, int value)
:   IAProperty(name),
    pValue(value)
{
}


IAIntProperty::~IAIntProperty()
{
}


void IAIntProperty::set(int v, IAController *ctrl)
{
    if (pValue!=v) {
        pValue = v;
        for (auto &c: pControlerList) {
            if (c!=ctrl)
                c->propertyValueChanged();
        }
    }
}

void IAIntProperty::read(Fl_Preferences &prefs)
{
    int v;
    prefs.get(pName, v, pValue);
    set(v);
}

void IAIntProperty::write(Fl_Preferences &prefs)
{
    prefs.set(pName, pValue);
}


IATextProperty::IATextProperty(char const* name, char const* value)
:   IAProperty(name),
    pValue(nullptr)
{
    _set(value);
}

IATextProperty::~IATextProperty()
{
    if (pValue)
        ::free((void*)pValue);
}

void IATextProperty::set(char const* value, IAController *ctrl)
{
    if (!_sameAs(value)) {
        _set(value);
        for (auto &c: pControlerList) {
            if (c!=ctrl)
                c->propertyValueChanged();
        }
    }
}

void IATextProperty::_set(char const* value)
{
    if (_sameAs(value)) return;
    if (pValue)
        ::free((void*)pValue);
    if (value)
        pValue = ::strdup(value);
    else
        pValue = nullptr;
}

bool IATextProperty::_sameAs(char const* value)
{
    if (pValue==nullptr) {
        return (value==nullptr);
    } else {
        if (value==nullptr)
            return false;
        return (strcmp(pValue, value)==0);
    }
}

void IATextProperty::read(Fl_Preferences &prefs)
{
    char *v = nullptr;
    prefs.get(pName, v, pValue);
    set(v);
    if (v)
        ::free(v);
}

void IATextProperty::write(Fl_Preferences &prefs)
{
    prefs.set(pName, pValue);
}





