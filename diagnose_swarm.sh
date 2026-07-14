#!/bin/bash

echo "===== SWARM DIAGNOSTIC ====="

ERROR=0

# 1. ARGoS install
echo "[1] Checking ARGoS..."
if command -v argos3 >/dev/null 2>&1; then
    echo "OK: argos3 found -> $(which argos3)"
    argos3 -v | head -n 2
else
    echo "ERROR: argos3 not found"
    ERROR=1
fi

echo ""

# 2. ARGoS prefix sanity
echo "[2] Checking ARGoS install prefix..."
if argos3 -v 2>/dev/null | grep -q "/usr"; then
    echo "OK: ARGoS installed in /usr"
else
    echo "WARN: ARGoS not in /usr (possible mismatch)"
fi

echo ""

# 3. Loop functions build
echo "[3] Checking loop functions library..."
if [ -f build/libswarm_loopfunctions.so ]; then
    echo "OK: libswarm_loopfunctions.so exists"
else
    echo "ERROR: missing build/libswarm_loopfunctions.so"
    ERROR=1
fi

echo ""

# 4. CMake config sanity
echo "[4] Checking CMake config..."
if grep -q "set(ARGOS_PREFIX /usr)" CMakeLists.txt; then
    echo "OK: ARGOS_PREFIX = /usr"
else
    echo "ERROR: ARGOS_PREFIX not set to /usr"
    ERROR=1
fi

echo ""

# 5. Lua controller presence
echo "[5] Checking Lua controller..."
if [ -f agent/controller/underwater_agent.lua ]; then
    echo "OK: Lua controller exists"
else
    echo "ERROR: missing Lua controller"
    ERROR=1
fi

echo ""

# 6. Experiment file
echo "[6] Checking ARGoS experiment file..."
if ls experiments/*.argos >/dev/null 2>&1; then
    echo "OK: experiment file found"
    ls experiments/*.argos
else
    echo "ERROR: no .argos experiment file found"
    ERROR=1
fi

echo ""

# 7. Shared library linkage check
echo "[7] Checking missing shared libs..."
if [ -f build/libswarm_loopfunctions.so ]; then
    ldd build/libswarm_loopfunctions.so | grep "not found" && ERROR=1
else
    echo "SKIP: no library to check"
fi

echo ""

# 8. Quick compile sanity (optional fast check)
echo "[8] Quick compile test (CMake exists)..."
if [ -f build/CMakeCache.txt ]; then
    echo "OK: build directory configured"
else
    echo "WARN: build not configured yet"
fi

echo ""

# FINAL
echo "===== RESULT ====="
if [ $ERROR -eq 0 ]; then
    echo "ALL GOOD - system ready"
else
    echo "ISSUES DETECTED - fix above errors"
fi

exit $ERROR