
problem Cutting_Opt;
# ----------------------------------------

param nPAT integer >= 0, default 0;
param nPAT_top integer >= 0, default 0;
param roll_width;

set PATTERNS := 1..nPAT;
set WIDTHS;

param orders {WIDTHS} > 0;
param nbr {WIDTHS,PATTERNS} integer >= 0;

let nPAT := 2 * card(WIDTHS);
let nPAT_top := 0;
for {i in WIDTHS} {
   let nPAT_top := nPAT_top + 1;
   let nbr[i,nPAT_top] := floor (roll_width/i);
   let {i2 in WIDTHS: i2 <> i} nbr[i2,nPAT_top] := 0;
}
#display nPAT, nPAT_top;
#display nbr;

check {j in PATTERNS : j <= nPAT_top}: sum {i in WIDTHS} i * nbr[i,j] <= roll_width;

var Cut {PATTERNS} integer >= 0;

minimize Number: sum {j in PATTERNS : j <= nPAT_top} Cut[j];

subject to Fill {i in WIDTHS}:
   sum {j in PATTERNS : j <= nPAT_top} nbr[i,j] * Cut[j] >= orders[i];


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

repeat {
#for {1..4} {
   solve.lp Cutting_Opt;
   let {i in WIDTHS} price[i] := Fill[i].dual;
   #display price;

   solve/*.mip*/ Pattern_Gen;
   display Reduced_Cost, Use;
   if Reduced_Cost < -0.00001 and nPAT_top <= nPAT then
   {
      let nPAT_top := nPAT_top + 1;
      let {i in WIDTHS} nbr[i,nPAT_top] := Use[i];
      #display nPAT;
   }
   else break;
}

check {j in PATTERNS : j <= nPAT_top}: sum {i in WIDTHS} i * nbr[i,j] <= roll_width;

solve/*.mip*/ Cutting_Opt;
display Cut;
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
end;
