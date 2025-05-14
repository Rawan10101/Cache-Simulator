# Cache Performance Simulator

Project Description:
--------------------------------------------------------------------------------
This project implements a configurable n-way set-associative cache simulator in C/C++. 
The simulator analyzes cache performance under different configurations and 
memory access patterns.

The simulator:
- Models a fixed-size cache (64 KB).
- Supports variable line sizes: 16, 32, 64, 128 bytes.
- Supports associativity (number of ways): 1, 2, 4, 8, 16.
- Uses a 64 MB memory address space.
- Collects cache hit/miss statistics using provided memory reference generators.
