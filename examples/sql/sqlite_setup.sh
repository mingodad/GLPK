#!/bin/sh
# This file can be used to create database glpk in SQLite.
sqlite3 sudoku.db < sudoku_sqlite.sql
sqlite3 transp.db < transp_sqlite.sql
