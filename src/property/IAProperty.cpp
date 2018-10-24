//
//  IAProperty.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAProperty.h"

#include "controller/IAController.h"

#include <FL/Fl_Preferences.H>

#include <algorithm>


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


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



#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IAProperty::IAProperty(const char *name)
:   pName(name)
{
}


IAProperty::~IAProperty()
{
    int i = (int)pControllerList.size();
    while (i>0) {
        // deleting c may remove c from the controller list, so use an index
        // and not an iterator!
        IAController *c = pControllerList[i-1];
        if (c->isAutoDelete())
            delete c;
        --i;
    }
}


void IAProperty::attach(IAController *ctrl)
{
    pControllerList.push_back(ctrl);
}


void IAProperty::detach(IAController* ctrl)
{
    auto c = std::find(pControllerList.begin(), pControllerList.end(), ctrl);
    if (c!=pControllerList.end())
        pControllerList.erase(c);
}



#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


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
        for (auto &c: pControllerList) {
            if (c!=ctrl)
                c->propertyValueChanged(this);
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


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


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
        for (auto &c: pControllerList) {
            if (c!=ctrl)
                c->propertyValueChanged(this);
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


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


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
    if (!_equals(value)) {
        _set(value);
        for (auto &c: pControllerList) {
            if (c!=ctrl)
                c->propertyValueChanged(this);
        }
    }
}

void IATextProperty::_set(char const* value)
{
    if (_equals(value)) return;
    if (pValue)
        ::free((void*)pValue);
    if (value)
        pValue = ::strdup(value);
    else
        pValue = nullptr;
}

bool IATextProperty::_equals(char const* value)
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


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IAFilenameProperty::IAFilenameProperty(char const* name, char const* value)
:   IATextProperty(name, value)
{
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IAVectorProperty::IAVectorProperty(const char *name, IAVector3d const& value)
:   IAProperty(name),
    pValue(value)
{
}


IAVectorProperty::IAVectorProperty(const char *name, IAVector3d const& value, std::function<void()>&& cb)
:   IAVectorProperty(name, value)
{
    pCallback = cb;
}


IAVectorProperty::~IAVectorProperty()
{
}


void IAVectorProperty::set(IAVector3d const& v, IAController *ctrl)
{
    if (pValue!=v) {
        pValue = v;
        if (pCallback) pCallback();
        for (auto &c: pControllerList) {
            if (c!=ctrl)
                c->propertyValueChanged(this);
        }
    }
}


void IAVectorProperty::read(Fl_Preferences &prefs)
{
    IAVector3d v;
    prefs.get(Fl_Preferences::Name("%s.x", pName), v.dataPointer()[0], pValue.x());
    prefs.get(Fl_Preferences::Name("%s.y", pName), v.dataPointer()[1], pValue.y());
    prefs.get(Fl_Preferences::Name("%s.z", pName), v.dataPointer()[2], pValue.z());
    set(v);
}


void IAVectorProperty::write(Fl_Preferences &prefs)
{
    prefs.set(Fl_Preferences::Name("%s.x", pName), pValue.x());
    prefs.set(Fl_Preferences::Name("%s.y", pName), pValue.y());
    prefs.set(Fl_Preferences::Name("%s.z", pName), pValue.z());
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IAPresetProperty::IAPresetProperty(IATextProperty &presetClass, char const* name, char const* value)
:   super(name, value),
    pPresetClass(presetClass)
{
}

IAPresetProperty::~IAPresetProperty()
{
}


void IAPresetProperty::set(char const* value, IAController *ctrl)
{
    if (!_equals(value)) {
        _set(value);
        load();
        for (auto &c: pControllerList) {
            if (c!=ctrl)
                c->propertyValueChanged(this);
        }
    }
}


void IAPresetProperty::load()
{
    char path[FL_PATH_MAX];
    snprintf(path, FL_PATH_MAX, "%spresets/%s/",
             Iota.gPreferences.printerDefinitionsPath(),
             pPresetClass());
    Fl_Preferences presetFile(path, "Iota Slicer Preset", pName);
    Fl_Preferences presets(presetFile, pValue);
    // kludge to tell controllers that we will now change ALL clients
    for (auto &c: pControllerList) c->propertyValueChanged((IAProperty*)1);
    for (auto &p: pClientList) {
        p->read(presets);
    }
    // kludge to tell controllers that we are done changing ALL clients
    for (auto &c: pControllerList) c->propertyValueChanged((IAProperty*)2);
}


void IAPresetProperty::save(const char *newTag)
{
    if (newTag) _set(newTag);
    char path[FL_PATH_MAX];
    snprintf(path, FL_PATH_MAX, "%spresets/%s/",
             Iota.gPreferences.printerDefinitionsPath(),
             pPresetClass());
    Fl_Preferences presetFile(path, "Iota Slicer Preset", pName);
    Fl_Preferences presets(presetFile, pValue);
    for (auto &p: pClientList) {
        p->write(presets);
    }
}


void IAPresetProperty::erase(const char *tag)
{
    if (!tag) tag = pValue;
    char path[FL_PATH_MAX];
    snprintf(path, FL_PATH_MAX, "%spresets/%s/",
             Iota.gPreferences.printerDefinitionsPath(),
             pPresetClass());
    Fl_Preferences presetFile(path, "Iota Slicer Preset", pName);
    presetFile.deleteGroup(tag);
}


void IAPresetProperty::listPresets(std::vector< std::string > &list)
{
    char path[FL_PATH_MAX];
    snprintf(path, FL_PATH_MAX, "%spresets/%s/",
             Iota.gPreferences.printerDefinitionsPath(),
             pPresetClass());
    Fl_Preferences presetFile(path, "Iota Slicer Preset", pName);
    int n = presetFile.groups();
    for (int i=0; i<n; i++) {
        const char *name = presetFile.group(i);
        list.push_back(name);
    }
}


void IAPresetProperty::addClient(IAProperty &prop)
{
    pClientList.push_back(&prop);
}


void IAPresetProperty::attachClients(IAController *ctrl)
{
    for (auto &p: pClientList) {
        p->attach(ctrl);
    }
}

