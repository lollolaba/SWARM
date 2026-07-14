cmake_minimum_required(VERSION 3.16)
project(swarm_4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(ARGOS_PREFIX /usr)

# -------------------------
# INCLUDE
# -------------------------
include_directories(
    ${ARGOS_PREFIX}/include
    ${ARGOS_PREFIX}/include/argos3
    ${CMAKE_SOURCE_DIR}
)

# -------------------------
# LUA
# -------------------------
find_path(LUA_INCLUDE_DIR lua.h
    /usr/include
    /usr/include/lua5.3
    /usr/include/lua5.4
)

if(NOT LUA_INCLUDE_DIR)
    message(FATAL_ERROR "Lua headers not found")
endif()

include_directories(
    ${LUA_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)

# -------------------------
# LOOP FUNCTIONS LIB
# -------------------------
file(GLOB_RECURSE SRC_FILES
    *.cpp)

add_library(
    swarm_loopfunctions SHARED
    ${SRC_FILES})

target_include_directories(
    swarm_loopfunctions PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/middleware
    ${CMAKE_SOURCE_DIR}/simulation
    ${PROJECT_SOURCE_DIR}
)

# -------------------------
# IMPORTANT FIX: FULL PATH LINK (NO -l)
# -------------------------
target_link_libraries(
    swarm_loopfunctions

    ${ARGOS_PREFIX}/lib/argos3/libargos3core_simulator.so
    dl
    pthread
)