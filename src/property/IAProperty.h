//
//  IAProperty.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PROPERTY_H
#define IA_PROPERTY_H


#include <vector>


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


class IAPropertyDouble : public IAProperty
{
public:
    IAPropertyDouble(const char *name, double value=0.0);
    virtual ~IAPropertyDouble() override;
    double operator()() const;
    void set(double v, IAController *ctrl=nullptr);
protected:
    double pValue = 0.0;
};


#endif /* IA_PROPERTY_H */


