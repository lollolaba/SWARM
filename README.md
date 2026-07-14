# SWARM — Distributed Swarm Localization in ARGoS

## Overview
Swarm_4 is a modular swarm robotics framework for distributed localization, communication, and underwater-inspired networking simulation built on ARGoS3.

The system integrates:
- EKF / Particle Filter / Consensus / Factor Graph localization
- Acoustic channel simulation (loss, SNR, multipath)
- Distributed routing + neighbor discovery
- Lua-based robot controllers

---

## Requirements

- ARGoS3
- Lua 5.3+
- CMake ≥ 3.16
- C++17 compiler

---

## Build

```bash
mkdir build
cd build
cmake ..
make -j
