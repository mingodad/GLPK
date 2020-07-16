/* glpnet.h (graph and network algorithms) */

/***********************************************************************
*  This code is part of GLPK (GNU Linear Programming Kit).
*
*  Copyright (C) 2000,01,02,03,04,05,06,07,08,2009 Andrew Makhorin,
*  Department for Applied Informatics, Moscow Aviation Institute,
*  Moscow, Russia. All rights reserved. E-mail: <mao@mai2.rcnet.ru>.
*
*  GLPK is free software: you can redistribute it and/or modify it
*  under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  GLPK is distributed in the hope that it will be useful, but WITHOUT
*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
*  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
*  License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with GLPK. If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#ifndef _GLPNET_H
#define _GLPNET_H

#define mc21a _glp_mc21a
int mc21a(int n, const int icn[], const int ip[], const int lenr[],
      int iperm[], int pr[], int arp[], int cv[], int out[]);
/* permutations for zero-free diagonal */

#define mc13d _glp_mc13d
int mc13d(int n, const int icn[], const int ip[], const int lenr[],
      int ior[], int ib[], int lowl[], int numb[], int prev[]);
/* permutations to block triangular form */

#endif

/* eof */
