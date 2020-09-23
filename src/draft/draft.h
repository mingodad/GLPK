/* draft.h */

/* (reserved for copyright notice) */

#ifndef DRAFT_H
#define DRAFT_H

#if 1 /* 28/III-2016 */
#define GLP_UNDOC 1
#endif
#include "glpk.h"

#if 1 /* 28/XI-2009 */
int _glp_analyze_row(glp_prob *P, int len, const int ind[],
      const glp_double val[], int type, glp_double rhs, glp_double eps, int *_piv,
      glp_double *_x, glp_double *_dx, glp_double *_y, glp_double *_dy, glp_double *_dz);
/* simulate one iteration of dual simplex method */
#endif

#endif

/* eof */
