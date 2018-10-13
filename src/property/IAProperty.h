//
//  IAProperty.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PROPERTY_H
#define IA_PROPERTY_H


#include <vector>
#include <functional>


class IAController;
class IAProperty;
class Fl_Preferences;


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
    std::vector<IAController*> pControlerList;
};


class IAPropertyEvent : public IAProperty
{
public:
    IAPropertyEvent(std::function<void()>&& cb)
    : IAProperty("event"), pCallback(cb) { }
    void trigger(IAController *ctrl=nullptr);
    std::function<void()> pCallback;
};


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


#endif /* IA_PROPERTY_H */


