#include "lua_bindings.h"
#include "../../core/communication/packet.h"
#include "../../globals.h"

static int LuaSendMessage(lua_State* L) {

    Packet p;

    lua_getfield(L,1,"sender");
    p.sender = lua_tostring(L,-1);
    lua_pop(L,1);

    lua_getfield(L,1,"est_x");
    p.est_x = lua_tonumber(L,-1);
    lua_pop(L,1);

    lua_getfield(L,1,"est_y");
    p.est_y = lua_tonumber(L,-1);
    lua_pop(L,1);

    g_message_bus.Send(p);

    return 0;
}

void RegisterLuaBindings(lua_State* L) {

    lua_register(
        L,
        "send_message",
        LuaSendMessage
    );
}