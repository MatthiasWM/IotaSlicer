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






