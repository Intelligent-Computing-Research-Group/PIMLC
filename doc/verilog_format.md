# Verilog Format
Here is the standard format of input verilog files.
Usually, the verilog files are generated in MIG_project.

```
module name( x0 , x1 , ... , xp , y0 , y1 , ... , yq );
  input x0 , x1 , ... , xp ;    // (eg. xp=x255)
  output y0 , y1 , ... , yq ;   // (eg. yq=y127)
  wire n(p+2) , n(p+3) , ... , nk ; // (eg. n(p+2)=n257, nk=n1530)
  assign ni = nj | nk ;
  other assign expressions
  ...
  assign y0 = n(p+2) ;
  ...
  assign yq = n(p+2+q) ;
endmodule

```

The assign expressions should be in the form of:

- INV: `  assign ni = ~nj ;`
- OR: `  assign ni = nj | nk ;`
- AND: `  assign ni = nj & nk ;`
- MAJ: `  assign ni = nj | nk ;`
