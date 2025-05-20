
#  Cache Performance Simulator

## Project Description
--------------------------------------------------------------------------------
This project implements a configurable **n-way set-associative cache simulator** in C/C++.  
The simulator analyzes cache performance under different configurations and memory access patterns.

The simulator:

- Models a fixed-size cache (**64 KB**).
- Supports variable line sizes: **16, 32, 64, 128 bytes**.
- Supports associativity (number of ways): **1, 2, 4, 8, 16**.
- Uses a **64 MB** memory address space.
- Collects cache **hit/miss statistics** using provided memory reference generators (`memGen1()` to `memGen6()`).
- Outputs results in a format suitable for **graphing and analysis**.

##  How to Compile

###  Prerequisites

- C++17-compatible compiler (e.g., `g++`, `clang++`)
- Optional: `make` for easier compilation



### Or compile manually:

```bash
g++ -std=c++17 -O2 -o cache_simulator src/*.cpp
```

## How to Use

After building the project:

```bash
./cache_simulator
```

Follow the prompts to:

* Choose a memory generator (`memGen1` to `memGen6`)
* Run experiments:

  * **Experiment 1**: Fix number of sets, vary **line size**
  * **Experiment 2**: Fix line size (64B), vary **associativity**
* View cache statistics (hit ratio, miss ratio)
* Save results to `results.csv` for plotting

## Example Output

Running memGen3 with 1,000,000 memory references...
Configuration: Line Size = 64B, Associativity = 4-way
Hits: 695332
Misses: 304668
Hit Ratio: 69.53%


## Plotting the Results

Use Python (e.g., `matplotlib`), Excel, or any tool to plot:

* **Hit Ratio vs. Line Size** (Experiment 1)
* **Hit Ratio vs. Number of Ways** (Experiment 2)

Ensure axes and legends are clearly labeled in your report.

