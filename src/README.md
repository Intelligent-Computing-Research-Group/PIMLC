# Source Code Guide

### Import verilog

In your code, you have to use  `BooleanDag *G = v2booleandag(inputfile);` to import verilog file to convert it into a DAG.

This function can be found in [importdag.h](importdag.h) and the `BooleanDag` class can be found in [booleandag](booleandag).

You can also refer to the `benchmark` submodule for more details about a legal verilog input file.



### Schedule


For given processors and BooleanDag, you then need to schedule the tasks to the processors.
You can then refer to [scheduler.h](scheduler.h) (The scheduling is always wrapped by a ILP model to find the best partition in the degree of parallelism *n*, which will not be discussed here).

We have multiple list-based algorithm here (rankuHEFTSchedule, rankuDynamicWeightsSchedule, rankuCPDynamicWeightsSchedule, ...). The final result is stored in a struct called `Schedule`.



### StageProcessors

The main part in the `Schedule` is called `StageProcessors`. You can see [procelem.h](procelem.h) to find more details about it. It is a linked-list, which divide the whole scheduling work into multiple stages, as described in our paper. In general, it includes all the scheduling details of all stages and the `StageProcessors` stored in the `Schedule` is the head of the stages linked-list.


### Others

- The [priority.h](priority.h) includes some priority algorithm which will provide a priority list of the DAG. Different priority algorithms are based on different rules and the results will always satisfy the topological order. This part is mainly for the list-algorithms. 

- The [placement.h](placement.h) is a submodule of the list-scheduling algorithm. As described in our reference, the list-scheduling algorithm can be divided into the priority calculation part and the placement part. This part is resibonsiable fo deciding the target processor of each task one by one.

- The [pimconfig.h](pimconfig.h) and [instruction.h](instruction.h) includes other information and have no corresponding cpp files.

