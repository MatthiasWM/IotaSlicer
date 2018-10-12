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


class IAProperty
{
public:
    IAProperty(const char *name);
    virtual ~IAProperty();
    void add(IAController *ctr);
    void remove(IAController *ctr);
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


/**
 * A Property can store one or more values, and set and get from Fl_Preferences.
 *
 * Any number of controllers can connect to a property. If the property value
 * changes, all controllers are sent a notification. The controller that changes
 * the value of the property should not be notified.
 */
class IAPropertyFloat : public IAProperty
{
public:
    IAPropertyFloat(const char *name, double value=0.0);
    virtual ~IAPropertyFloat() override;
    double operator()() const;
    void set(double v, IAController *ctrl=nullptr);
protected:
    double pValue = 0.0;
};


#endif /* IA_PROPERTY_H */


