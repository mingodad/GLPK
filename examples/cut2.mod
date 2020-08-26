
problem Cutting_Opt;
# ----------------------------------------

param nPAT integer >= 0, default 0;
param roll_width;

set PATTERNS := 1..nPAT;
set WIDTHS;

param orders {WIDTHS} > 0;
param nbr {WIDTHS,PATTERNS} integer >= 0;

check {j in PATTERNS}: sum {i in WIDTHS} i * nbr[i,j] <= roll_width;

var Cut {PATTERNS} integer >= 0;

minimize Number: sum {j in PATTERNS} Cut[j];

subject to Fill {i in WIDTHS}:
   sum {j in PATTERNS} nbr[i,j] * Cut[j] >= orders[i];


problem Pattern_Gen;
# ----------------------------------------

param price {WIDTHS} default 0;

var Use {WIDTHS} integer >= 0;

minimize Reduced_Cost:
   1 - sum {i in WIDTHS} price[i] * Use[i];

subject to Width_Limit:
   sum {i in WIDTHS} i * Use[i] <= roll_width;

problem Mix : Cut, Reduced_Cost;

display Cutting_Opt, Pattern_Gen, Mix;

solve Cutting_Opt;
#solve Pattern_Gen;

data;

param roll_width := 110;
set WIDTHS :=
     20
     45
     50
     55
     75;
param orders :=
     [20] 48
     [45] 35
     [50] 24
     [55] 10
     [75] 8;
param nPAT := 8;
param nbr :=
     [20,1] 5
     [45,1] 0
     [50,1] 0
     [55,1] 0
     [75,1] 0
     [45,2] 2
     [20,2] 0
     [50,2] 0
     [55,2] 0
     [75,2] 0
     [50,3] 2
     [20,3] 0
     [45,3] 0
     [55,3] 0
     [75,3] 0
     [55,4] 2
     [20,4] 0
     [45,4] 0
     [50,4] 0
     [75,4] 0
     [75,5] 1
     [20,5] 0
     [45,5] 0
     [50,5] 0
     [55,5] 0
     [20,6] 1
     [45,6] 0
     [50,6] 0
     [55,6] 0
     [75,6] 1
     [20,7] 1
     [45,7] 2
     [50,7] 0
     [55,7] 0
     [75,7] 0
     [20,8] 3
     [45,8] 0
     [50,8] 1
     [55,8] 0
     [75,8] 0;
param price :=
     [20] 0.166667
     [45] 0.416667
     [50] 0.5
     [55] 0.5
     [75] 0.833333;
end;
