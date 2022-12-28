# PIMLC

|  	| TVM 	| PIMLC 	|
|---	|---	|---	|
| Graph-level 	| - Operator fusion<br>- constant folding<br>- static memory planning pass<br>- data layout transformation 	| - operator fusion 	|
| Operator-level 	|    - Loop transformation<br>   - Thread Binding<br>   - Compute Locality<br>   - special memory scope<br>   - Tensorization<br>   - Latency Hiding 	|  	|

The schedule primitives is important in TVM to cover a diverse set of optimization on different hardware backend (inherit primitives and Loop AST from Halide and create new primitives for GPU and accelerator)

In PIMLC, we reuse the schedule primitives adopted in /yosys/abc, 





[1] TVM: An Automated End-to-End Optimizing Compiler for Deep Learning; https://www.usenix.org/system/files/osdi18-chen.pdf
