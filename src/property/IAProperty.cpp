//
//  IAProperty.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAProperty.h"

#include "controller/IAController.h"


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


void IAProperty::add(IAController *ctrl)
{
    pControlerList.push_back(ctrl);
}


void IAProperty::remove(IAController* ctrl)
{
    auto c = std::find(pControlerList.begin(), pControlerList.end(), ctrl);
    if (c!=pControlerList.end())
        pControlerList.erase(c);
}




IAPropertyDouble::IAPropertyDouble(const char *name, double value)
:   IAProperty(name),
    pValue(value)
{
}


IAPropertyDouble::~IAPropertyDouble()
{
}


double IAPropertyDouble::operator()() const
{
    return pValue;
}


void IAPropertyDouble::set(double v, IAController *ctrl)
{
    pValue = v;
    for (auto &c: pControlerList) {
        if (c!=ctrl)
            c->propertyValueChanged();
    }
}



