/* MAGIC, Magic Square */

/* Written in GNU MathProg by Andrew Makhorin <mao@gnu.org> */

/* In recreational mathematics, a magic square of order n is an
   arrangement of n^2 numbers, usually distinct integers, in a square,
   such that n numbers in all rows, all columns, and both diagonals sum
   to the same constant. A normal magic square contains the integers
   from 1 to n^2.

   (From Wikipedia, the free encyclopedia.) */

param n, integer, > 0, default 4;
/* square order */

set N := 1..n^2;
/* integers to be placed */

set N1 := 1..n;

var x{i in N1, j in N1, k in N}, binary;
/* x[i,j,k] = 1 means that cell (i,j) contains integer k */

s.t. a{i in N1, j in N1}: sum{k in N} x[i,j,k] = 1;
/* each cell must be assigned exactly one integer */

s.t. b{k in N}: sum{i in N1, j in N1} x[i,j,k] = 1;
/* each integer must be assigned exactly to one cell */

var s;
/* the magic sum */

s.t. r{i in N1}: sum{j in N1, k in N} k * x[i,j,k] = s;
/* the sum in each row must be the magic sum */

s.t. c{j in N1}: sum{i in N1, k in N} k * x[i,j,k] = s;
/* the sum in each column must be the magic sum */

s.t. d: sum{i in N1, k in N} k * x[i,i,k] = s;
/* the sum in the diagonal must be the magic sum */

s.t. e: sum{i in N1, k in N} k * x[i,n-i+1,k] = s;
/* the sum in the co-diagonal must be the magic sum */

solve;

printf "\n";
printf "Magic sum is %d\n", s;
printf "\n";
for{i in N1}
{  printf{j in N1} "%3d", sum{k in N} k * x[i,j,k];
   printf "\n";
}
printf "\n";

end;
