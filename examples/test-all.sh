#!/bin/sh
glpsol_cmd=./glpsol
diff_cmd=diff

solve() {
	echo $1
	/usr/bin/time timeout 10s $glpsol_cmd -m $1.mod  -o $1.sol > $1.out
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
solve life_goe
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
solve osemosys
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
solve trick
solve tsp
solve wolfra6d
solve xyacfs
solve yacfs
solve zebra
