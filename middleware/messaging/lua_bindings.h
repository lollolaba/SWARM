#pragma once

extern "C"{
#include <lua.h>
#include <lauxlib.h>
}

void RegisterLuaBindings(lua_State* L);