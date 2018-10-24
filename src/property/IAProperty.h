//
//  IAProperty.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PROPERTY_H
#define IA_PROPERTY_H


#include "geometry/IAVector3d.h"

#include <vector>
#include <string>
#include <functional>


class IAController;
class IAProperty;
class Fl_Preferences;


/**
 * A list of properties.
 *
 * \see IAProperty
 */
class IAPropertyList
{
public:
    IAPropertyList();
    ~IAPropertyList();
    void add(IAProperty*);
    // read all
    // write all
protected:
    std::vector<IAProperty*> pList;
};


/**
 * A Property can store one or more values, and set and get from Fl_Preferences.
 *
 * Any number of controllers can connect to a property. If the property value
 * changes, all controllers are sent a notification. The controller that changes
 * the value of the property should not be notified.
 */
class IAProperty
{
public:
    IAProperty(const char *name);
    virtual ~IAProperty();
    void attach(IAController *ctr);
    void detach(IAController *ctr);
    virtual void read(Fl_Preferences&) { }
    virtual void write(Fl_Preferences&) { }
protected:
    const char *pName;
    std::vector<IAController*> pControllerList;
    std::function<void()> pCallback;
};


/**
 * This property holds a single floating point value in a variable of type 'double'.
 */
class IAFloatProperty : public IAProperty
{
public:
    IAFloatProperty(const char *name, double value=0.0);
    virtual ~IAFloatProperty() override;
    double operator()() const { return pValue; }
    void set(double v, IAController *ctrl=nullptr);
    virtual void read(Fl_Preferences&) override;
    virtual void write(Fl_Preferences&) override;
protected:
    double pValue = 0.0;
};


/**
 * This property holds a single integer value in a variable of type 'int'.
 */
class IAIntProperty : public IAProperty
{
public:
    IAIntProperty(const char *name, int value=0);
    virtual ~IAIntProperty() override;
    int operator()() const { return pValue; }
    void set(int v, IAController *ctrl=nullptr);
    virtual void read(Fl_Preferences&) override;
    virtual void write(Fl_Preferences&) override;
protected:
    int pValue = 0;
};


/**
 * This property holds a utf-8 string.
 *
 * Memory allocations and deallocations are done inside the class.
 */
class IATextProperty : public IAProperty
{
public:
    IATextProperty(char const* name, char const* value=nullptr);
    virtual ~IATextProperty() override;
    char const* operator()() const { return pValue; }
    void set(char const* value, IAController *ctrl=nullptr);
    virtual void read(Fl_Preferences&) override;
    virtual void write(Fl_Preferences&) override;
protected:
    void _set(char const* value);
    bool _equals(char const* value);
    char *pValue = nullptr;
};


/**
 * This property holds a filename in a utf-8 string.
 *
 * Memory allocations and deallocations are done inside the class.
 */
class IAFilenameProperty : public IATextProperty
{
public:
    IAFilenameProperty(char const* name, char const* value=nullptr);
};


/**
 * This property holds a IAVector3d.
 */
class IAVectorProperty : public IAProperty
{
public:
    IAVectorProperty(char const* name, IAVector3d const& value);
    IAVectorProperty(char const* name, IAVector3d const& value, std::function<void()>&& cb);
    virtual ~IAVectorProperty() override;
    IAVector3d& operator()() { return pValue; }
    IAVector3d const& operator()() const { return pValue; }
    void set(IAVector3d const& value, IAController *ctrl=nullptr);
    virtual void read(Fl_Preferences&) override;
    virtual void write(Fl_Preferences&) override;
protected:
    IAVector3d pValue;
};


/**
 * This property references an extruder in the FDM Printer setup.
 */
class IAExtruderProperty : public IAIntProperty
{
public:
    IAExtruderProperty(char const* name, int value=0)
    : IAIntProperty(name, value) { }
};


/**
 * This property holds the name of a preset and manages linked properties.
 *
 * Memory allocations and deallocations are done inside the class.
 */
class IAPresetProperty : public IATextProperty
{
    typedef IATextProperty super;
public:
    IAPresetProperty(IATextProperty &presetClass, char const* name, char const* value=nullptr);
    virtual ~IAPresetProperty() override;
//    char const* operator()() const { return pValue; }
    void set(char const* value, IAController *ctrl=nullptr);
//    virtual void read(Fl_Preferences&) override;
//    virtual void write(Fl_Preferences&) override;
    void load();
    void save();
    void listPresets(std::vector< std::string > &list);
    // check if preset by this name exists
    // escape and unescape preset names for preferences
    // erase a preset from a file
    // add a fallback set of presets if no file is found or it is empty
    void addClient(IAProperty &prop);
    void attachClients(IAController*);
protected:
//    void _set(char const* value);
//    bool _equals(char const* value);
//    char *pValue = nullptr;
    std::vector<IAProperty*> pClientList;
    IATextProperty &pPresetClass;
};





#endif /* IA_PROPERTY_H */


