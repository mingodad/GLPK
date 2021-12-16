#!/bin/sh
glpsol_cmd=${GLPSOL_CMD:-./glpsol}
diff_cmd=diff

solve() {
	echo $1
	/usr/bin/time timeout 60s $glpsol_cmd -m $1.mod $2 -o $1.sol > $1.out
	#/usr/bin/time $glpsol_cmd -m $1.mod $2  -o $1.sol > $1.out
}


solve assign
solve bpp
solve cal
solve cf12a
solve cf12b
solve cflsq
solve color
solve cpp
solve crypto
solve dea
solve diet
solve dist
solve egypt
solve fctp
solve food2
solve food
solve gap
solve graceful
solve graph
solve hashi
solve huge
solve jssp
solve magic
solve maxcut
solve maxflow
solve mfasp
solve mfvsp
solve min01ks
solve misp
solve money
solve mvcp
solve numbrix
solve osemosys "-d atlantis.dat"
solve osemosys_short "-d atlantis.dat"
solve pentomino
solve planarity
solve plan
solve powpl25h
solve powplant
solve prod
solve qfit
solve queens
solve sat
solve shiftcov
solve shikaku
solve sorting
solve spp
solve stigler
solve sudoku
solve tas
solve tiling
solve todd
solve toto
solve train
solve transp
solve tsp
solve wolfra6d
solve xyacfs
solve yacfs
solve zebra

if [ ! -z $1 ] && [ "$1" -eq "all" ]
then
    solve life_goe
    solve mem-default "-d mem-default.dat"
    solve trick
fi

#for fn in *.sol; do echo $fn; $diff_cmd $fn ../../GLPK-4.65/examples/$fn;done > test-all-sol-diff.log 2>&1
#for fn in *.out; do echo $fn; $diff_cmd $fn ../../GLPK-4.65/examples/$fn;done > test-all-out-diff.log 2>&1

