//
//  Iota.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#include "lua/IALua.h"

#include "Iota.h"

extern "C" {
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
}

#include <string.h>


static luaL_Reg iaLuaFuncs[] = {
    { "quit", IALua::quit },
    { nullptr, nullptr }
};



int IALua::quit(lua_State *L)
{
    bool forceQuit = false;
    int n = lua_gettop(L);
    if (n==2) {
        luaL_checktype(L, 2, LUA_TBOOLEAN);
        forceQuit = lua_toboolean(L, 2);
    }
    Iota.userMenuFileQuit();
    /** \bug call via AppController and use a property to genearte notifications? */
    lua_pushnumber(L, 1);
    return 1;
}


IALua::IALua()
{
    L = luaL_newstate();   /* opens Lua */
    luaL_openlibs(L);
    lua_newtable(L);
    luaL_setfuncs(L, iaLuaFuncs, 0);
    lua_setglobal(L, "Iota");

}


IALua::~IALua()
{
    if (L) {
        lua_close(L);
    }
}


int IALua::dostring(const char *cmd)
{
    int error = luaL_dostring(L, cmd);
    if (error) {
        fprintf(stderr, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);  // pop error message from the stack 
    }
    return error;
}




