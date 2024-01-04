# LC4PIM {#mainpage}
Logic Compilation for Processing-in-Memory
 
 -------------------------------------------

This project provide a compiler to convert high-level Boolean function into the the low-level primitives support by bit-serial based processing-in-memory system.


## Doxygen
This repo uses Doxygen to generate documentation from source code.

First, you should edit Doxyfile for further configuration and you can refer to the [official manual](https://www.doxygen.nl/manual/index.html) to find more about the details.

Use `doxygen` command (you need to install doxygen first) to generate documentation in the `html` and `latex` folder. We basically use the web browesr and start with the `index.html` in `html`.


## Installation
The ILP part needs coinor libs:
`sudo apt-get install coinor-cbc=2.8.12-1build2 coinor-libcbc-dev=2.8.12-1build2`


## C/C++ to Verilog (IR conversion)
 
## Netlist Synthesis (Compilation)

## Netlist to Instruction (Scheduling + Codegen)  

## Instruction Execution (Execution)
 
## Validation and Benchmark

We built a simulator and use existing benchmark to validate the functionality of our PIMLC.

The [simulator](https://github.com/Intelligent-Computing-Research-Group/PIMLC-Simulator) runs the instructions generated by PIMLC. It compares the results with the output of Verilator.


The benchmark is chosen from [EFPL Combinational Benchmark Suite](https://github.com/lsils/benchmarks) with some modifications. Use `git submodule update --init` to get the [benchmark submodule](https://github.com/Intelligent-Computing-Research-Group/PIMLC-Benchmark).

