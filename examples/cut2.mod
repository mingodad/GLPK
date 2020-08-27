param xx := 10;
display xx;
let xx := 20;
display xx;

param zz{1..5} default 0;
display zz[2];
let zz[2] := 22;
display zz[2];

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

problem Pattern_Gen;

if nPAT > 0 then {
	problem Mix;
	#problem Mix2; #problem creation not allowed here
}

display Cutting_Opt, Pattern_Gen, Mix;

let nPAT := 0;
for {i in WIDTHS} {
   let nPAT := nPAT + 1;
   let nbr[i,nPAT] := floor (roll_width/i);
   let {i2 in WIDTHS: i2 <> i} nbr[i2,nPAT] := 0;
}
display nPAT;
display nbr;

repeat {
   solve Cutting_Opt;
   let {i in WIDTHS} price[i] := Fill[i].dual;
   display price;

   solve Pattern_Gen;
   if Reduced_Cost < -0.00001 then {
      let nPAT := nPAT + 1;
      let {i in WIDTHS} nbr[i,nPAT] := Use[i];
      display Use;
   }
   else break;
}

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
param price :=
     [20] 0.166667
     [45] 0.416667
     [50] 0.5
     [55] 0.5
     [75] 0.833333;
end;
