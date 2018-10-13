//
//  IASetting.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_SETTING_H
#define IA_SETTING_H


#include "controller/IAController.h"

#include <vector>
#include <functional>

#include <FL/Fl_Preferences.H>


struct Fl_Menu_Item;
class Fl_Input_Choice;
class Fl_Choice;
class Fl_Tree;
class Fl_Tree_Item;
class Fl_Preferences;


// FIXME: deriving from IAController is an awful hack to transition from
// IASettings into IAProperty and IAController (and IAView)
// FIXME: what actually happens whe the tree is cleared? Tree-Items deletd? Widget stay in Group? ???
class IASetting : public IAController
{
public:
    typedef enum { kProperty, kSetting } Type;
    IASetting(const char *path, const char *label);
    virtual ~IASetting();
    virtual void build(Fl_Tree*, Type) { }

    Fl_Menu_Item *dup(Fl_Menu_Item const*);

    char *pPath = nullptr;
    char *pLabel = nullptr;
    Fl_Tree_Item *pTreeItem = nullptr;
};


class IALabelView;

/**
 * Manage a setting that appears in a tree view.
 */
class IALabelController : public IASetting
{
public:
    IALabelController(const char *path, const char *label, const char *text=nullptr);
    virtual ~IALabelController() override;
    virtual void build(Fl_Tree*, Type) override;

    char *pText = nullptr;
    IALabelView *pWidget = nullptr;
};


class IAFloatView;

/**
 * Manage a setting that appears in a tree view.
 */
class IAFloatController : public IASetting
{
public:
    IAFloatController(const char *path, const char *label, IAFloatProperty &prop,
                      const char *unit, std::function<void()>&& cb);
    virtual ~IAFloatController() override;
    virtual void build(Fl_Tree*, Type) override;
    virtual void propertyValueChanged() override;

    static void wCallback(IAFloatView *w, IAFloatController *d);

    IAFloatProperty &pProperty;
    char *pUnit = nullptr;
    std::function<void()> pCallback;
    IAFloatView *pWidget = nullptr;
};


class IATextView;

/**
 * Manage a setting that appears in a tree view.
 */
class IATextController : public IASetting
{
public:
    IATextController(const char *path, const char *label, IATextProperty &prop, int wdt,
                  const char *unit, std::function<void()>&& cb);
    virtual ~IATextController() override;
    virtual void build(Fl_Tree*, Type) override;
    virtual void propertyValueChanged() override;

    static void wCallback(IATextView *w, IATextController *d);

    IATextProperty &pProperty;
    int pWdt;
    char *pUnit = nullptr;
    std::function<void()> pCallback;
    IATextView *pWidget = nullptr;
};


class IAFloatProperty;
class IAFloatChoiceView;

/**
 * Manage a setting that appears in a tree view.
 */
class IAFloatChoiceController : public IASetting
{
public:
    IAFloatChoiceController(const char *path, const char *label, IAFloatProperty &prop,
                         const char *unit, std::function<void()>&& cb,
                         Fl_Menu_Item *menu);
    virtual ~IAFloatChoiceController() override;
    virtual void build(Fl_Tree*, Type) override;
    virtual void propertyValueChanged() override;

    static void wCallback(IAFloatChoiceView *w, IAFloatChoiceController *d);

    IAFloatProperty &pProperty;
    char *pUnit = nullptr;
    std::function<void()> pCallback;
    Fl_Menu_Item *pMenu = nullptr;
    IAFloatChoiceView *pWidget = nullptr;
};


class IAIntProperty;
class IAChoiceView;

/**
 * Manage a setting that appears in a tree view.
 */
class IAChoiceController : public IASetting
{
public:
    IAChoiceController(const char *path, const char *label, IAIntProperty &prop,
                    std::function<void()>&& cb, Fl_Menu_Item *menu);
    virtual ~IAChoiceController() override;
    virtual void build(Fl_Tree*, Type) override;
    virtual void propertyValueChanged() override;

    static void wCallback(IAChoiceView *w, IAChoiceController *d);

    IAIntProperty &pProperty;
    Fl_Menu_Item *pMenu = nullptr;
    std::function<void()> pCallback;
    IAChoiceView *pWidget = nullptr;
};


typedef std::vector<IASetting*> IASettingList;


#endif /* IA_SETTING_H */


