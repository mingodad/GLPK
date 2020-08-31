
param nWidthX integer >= 0, default 0;
param nPAT integer >= 0, default 0;
param nPAT_top integer >= 0, default 0;
param roll_width;

set PATTERNS := 1..nPAT;
set WIDTHS;

param orders {WIDTHS} > 0;
param nbr {WIDTHS,PATTERNS} integer >= 0;

let nPAT := nWidthX * card(WIDTHS);
let nPAT_top := 0;
for {i in WIDTHS} {
   let nPAT_top := nPAT_top + 1;
   let nbr[i,nPAT_top] := floor (roll_width/i);
   let {i2 in WIDTHS: i2 <> i} nbr[i2,nPAT_top] := 0;
}
#display nPAT, nPAT_top;
#display nbr;

check {j in PATTERNS : j <= nPAT_top}: sum {i in WIDTHS} i * nbr[i,j] <= roll_width;

problem Cutting_Opt;
# ----------------------------------------

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

/*
problem Mix : Cut, Reduced_Cost;

problem Pattern_Gen;

if nPAT > 0 then {
	problem Mix;
	#problem Mix2; #problem creation not allowed here
}

display Cutting_Opt, Pattern_Gen, Mix;
*/
repeat {
#for {1..4} {
   #display nPAT, nPAT_top;
   solve.lp Cutting_Opt;
   let {i in WIDTHS} price[i] := Fill[i].dual;
   #display price;

   solve/*.mip*/ Pattern_Gen;
   #display Reduced_Cost, Use;
   if Reduced_Cost < -0.00001 and nPAT_top < nPAT then
   {
      let nPAT_top := nPAT_top + 1;
      let {i in WIDTHS} nbr[i,nPAT_top] := Use[i];
      #display nPAT;
   }
   else break;
}

if nPAT_top < nPAT then
{
	check {j in PATTERNS : j <= nPAT_top}: sum {i in WIDTHS} i * nbr[i,j] <= roll_width;

	solve/*.mip*/ Cutting_Opt;
	#display Number; #, Cut, nbr;
	param total_length := 0;
	printf "Patterns\n";
	for{j in PATTERNS : j <= nPAT_top && Cut[j] > 0}{
		printf "%4d x { ", Cut[j];
		for{i in WIDTHS: nbr[i,j] > 0}{
			printf "%dx%d ", nbr[i,j], i;
			let total_length := total_length + (Cut[j] * (nbr[i,j] * i));
		}
		printf "}\n";
	}
	param waste_length := (Number * roll_width) - total_length;
	printf "Rolls\n";
	printf "%4d rolls, waste %d -> %g%%\n", Number, waste_length, (waste_length / (Number * roll_width)) * 100;
}
else
{
	printf "!!!!!*****\n***** %s\n!!!!!*****\n",
		"Failed to get a solution, try increase parameter nWidthX value";
}

data;
/*
param roll_width := 110;
param nWidthX := 2;
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
*/

# data from https://en.wikipedia.org/wiki/Cutting_stock_problem
# expected solution 72.7 rolls, which means 73 rolls or more are required
param roll_width := 5600;
param nWidthX := 4;
set WIDTHS :=
	1380
	1520
	1560
	1710
	1820
	1880
	1930
	2000
	2050
	2100
	2140
	2150
	2200;

param orders :=
	[1380]	22
	[1520]	25
	[1560]	12
	[1710]	14
	[1820]	18
	[1880]	18
	[1930]	20
	[2000]	10
	[2050]	12
	[2100]	14
	[2140]	16
	[2150]	18
	[2200]	20;

/*
param roll_width := 10000;
param nWidthX := 4;
param: WIDTHS: orders :=
	7496 72
	7312 1
	7311 85
	7229 88
	7194 10
	7115 59
	6514 81
	6464 51
	6421 16
	6297 40
	6296 58
	6232 20
	6223 18
	6040 79
	5720 43
	5580 51
	5561 52
	5487 50
	5276 27
	5238 64
	4904 87
	4759 90
	4515 59
	4304 56
	4138 39
	4068 29
	4035 30
	3806 60
	3735 91
	3676 81
	3543 91
	3392 39
	3366 2
	2868 33
	2865 25
	2429 14
	2283 56
	2144 34
	2058 83
	1870 68
	1797 54
	1742 22
	1644 23
	1284 33
	1276 19
	1258 86
	1072 85
	1018 19
	803 20
	287 107;
*/
end;
