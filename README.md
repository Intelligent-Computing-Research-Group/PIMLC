# LC4PIM
Logic Compilation for Processing-in-Memory
 
 -------------------------------------------

This project provide a compiler to convert high-level Boolean function into the the low-level primitives support by bit-serial based processing-in-memory system.

## Installation
The ILP part needs coinor libs:
`sudo apt-get install coinor-cbc=2.8.12-1build2 coinor-libcbc-dev=2.8.12-1build2`

## Roadmap

### V-0.1
- [x] Create the docker for LC4PIM
- [x] Basic Boolean function parser
- [ ] use the logic synthesis tool to import the DAG of the Boolean function.

## C/C++ to Verilog (IR conversion)
 
## Netlist Synthesis (Compilation)

## Netlist to Instruction (Scheduling + Codegen)  

## Instruction Execution (Execution)
 
## Validation
 
## Benchmark
The benchmark is chosen from [EFPL](https://github.com/lsils/benchmarks) with some modifiaations. Use `git submodule update --init` to get the [benchmark submodule](https://github.com/Intelligent-Computing-Research-Group/PIMLC-Benchmark).