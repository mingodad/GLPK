/* glpapi08.c (interior-point method routines) */

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

#include "glpapi.h"
#include "glpipm.h"

/***********************************************************************
*  NAME
*
*  glp_interior - solve LP problem with the interior-point method
*
*  SYNOPSIS
*
*  int glp_interior(glp_prob *lp, const void *parm);
*
*  DESCRIPTION
*
*  The routine glp_interior is a driver to the LP solver based on the
*  interior-point method.
*
*  The parameter parm is reserved for use in the future and must be
*  specified as NULL.
*
*  Currently this routine implements an easy variant of the primal-dual
*  interior-point method based on Mehrotra's technique.
*
*  This routine transforms the original LP problem to an equivalent LP
*  problem in the standard formulation (all constraints are equalities,
*  all variables are non-negative), calls the routine ipm_main to solve
*  the transformed problem, and then transforms an obtained solution to
*  the solution of the original problem.
*
*  RETURNS
*
*  0  The LP problem instance has been successfully solved. This code
*     does not necessarily mean that the solver has found optimal
*     solution. It only means that the solution process was successful.
*
*  GLP_EFAIL
*     The problem has no rows/columns.
*
*  GLP_ENOFEAS
*     The problem has no feasible (primal/dual) solution.
*
*  GLP_ENOCVG
*     Very slow convergence or divergence.
*
*  GLP_EITLIM
*     Iteration limit exceeded.
*
*  GLP_EINSTAB
*     Numerical instability on solving Newtonian system. */

struct dsa
{     /* working area used by glp_interior routine */
      LPX *lp;
      /* original LP instance to be solved */
      int orig_m;
      /* number of rows in original LP */
      int orig_n;
      /* number of columns in original LP */
      int *ref; /* int ref[1+orig_m+orig_n]; */
      /* this array contains references from components of original LP
         to components of transformed LP */
      /*--------------------------------------------------------------*/
      /* following components define transformed LP in standard form:
         minimize    c * x + c[0]
         subject to  A * x = b,   x >= 0 */
      int m;
      /* number of rows (constraints) in transformed LP */
      int n;
      /* number of columns (variables) in transformed LP */
      int size;
      /* size of arrays ia, ja, ar (enlarged automatically) */
      int ne;
      /* number of non-zeros in transformed constraint matrix */
      int *ia; /* int ia[1+size]; */
      int *ja; /* int ja[1+size]; */
      double *ar; /* double ar[1+size]; */
      /* transformed constraint matrix in storage-by-indices format
         which has m rows and n columns */
      double *b; /* double b[1+m]; */
      /* right-hand sides; b[0] is not used */
      double *c; /* double c[1+n]; */
      /* objective coefficients; c[0] is constant term */
      /*--------------------------------------------------------------*/
      /* following arrays define solution of transformed LP computed by
         interior-point solver */
      double *x; /* double x[1+n]; */
      /* values of primal variables */
      double *y; /* double y[1+m]; */
      /* values of dual variables for equality constraints */
      double *z; /* double z[1+n]; */
      /* values of dual variables for non-negativity constraints */
};

static void calc_mn(struct dsa *dsa)
{     /* calculate the number of constraints (m) and variables (n) for
         transformed LP */
      LPX *lp = dsa->lp;
      int orig_m = dsa->orig_m;
      int orig_n = dsa->orig_n;
      int i, j, m = 0, n = 0;
      for (i = 1; i <= orig_m; i++)
      {  switch (lpx_get_row_type(lp, i))
         {  case LPX_FR:                  break;
            case LPX_LO: m++; n++;        break;
            case LPX_UP: m++; n++;        break;
            case LPX_DB: m += 2; n += 2;  break;
            case LPX_FX: m++;             break;
            default: xassert(lp != lp);
         }
      }
      for (j = 1; j <= orig_n; j++)
      {  switch (lpx_get_col_type(lp, j))
         {  case LPX_FR: n += 2;          break;
            case LPX_LO: n++;             break;
            case LPX_UP: n++;             break;
            case LPX_DB: m++; n += 2;     break;
            case LPX_FX:                  break;
            default: xassert(lp != lp);
         }
      }
      dsa->m = m;
      dsa->n = n;
      return;
}

static void new_coef(struct dsa *dsa, int i, int j, double aij)
{     /* add new element to the transformed constraint matrix */
      xassert(1 <= i && i <= dsa->m);
      xassert(1 <= j && j <= dsa->n);
      xassert(aij != 0.0);
      if (dsa->ne == dsa->size)
      {  int *ia = dsa->ia;
         int *ja = dsa->ja;
         double *ar = dsa->ar;
         dsa->size += dsa->size;
         dsa->ia = xcalloc(1+dsa->size, sizeof(int));
         memcpy(&dsa->ia[1], &ia[1], dsa->ne * sizeof(int));
         xfree(ia);
         dsa->ja = xcalloc(1+dsa->size, sizeof(int));
         memcpy(&dsa->ja[1], &ja[1], dsa->ne * sizeof(int));
         xfree(ja);
         dsa->ar = xcalloc(1+dsa->size, sizeof(double));
         memcpy(&dsa->ar[1], &ar[1], dsa->ne * sizeof(double));
         xfree(ar);
      }
      xassert(dsa->ne < dsa->size);
      dsa->ne++;
      dsa->ia[dsa->ne] = i;
      dsa->ja[dsa->ne] = j;
      dsa->ar[dsa->ne] = aij;
      return;
}

static void transform(struct dsa *dsa)
{     /* transform original LP to standard formulation */
      LPX *lp = dsa->lp;
      int orig_m = dsa->orig_m;
      int orig_n = dsa->orig_n;
      int *ref = dsa->ref;
      int m = dsa->m;
      int n = dsa->n;
      double *b = dsa->b;
      double *c = dsa->c;
      int i, j, k, type, t, ii, len, *ind;
      double lb, ub, coef, rii, sjj, *val;
      /* initialize components of transformed LP */
      dsa->ne = 0;
      for (i = 1; i <= m; i++) b[i] = 0.0;
      c[0] = lpx_get_obj_coef(lp, 0);
      for (j = 1; j <= n; j++) c[j] = 0.0;
      /* i and j are, respectively, ordinal number of current row and
         ordinal number of current column in transformed LP */
      i = j = 0;
      /* transform rows (auxiliary variables) */
      for (k = 1; k <= orig_m; k++)
      {  type = lpx_get_row_type(lp, k);
         rii = glp_get_rii(lp, k);
         lb = lpx_get_row_lb(lp, k) * rii;
         ub = lpx_get_row_ub(lp, k) * rii;
         switch (type)
         {  case LPX_FR:
               /* source: -inf < (L.F.) < +inf */
               /* result: ignore free row */
               ref[k] = 0;
               break;
            case LPX_LO:
               /* source: lb <= (L.F.) < +inf */
               /* result: (L.F.) - x' = lb, x' >= 0 */
               i++; j++;
               ref[k] = i;
               new_coef(dsa, i, j, -1.0);
               b[i] = lb;
               break;
            case LPX_UP:
               /* source: -inf < (L.F.) <= ub */
               /* result: (L.F.) + x' = ub, x' >= 0 */
               i++; j++;
               ref[k] = i;
               new_coef(dsa, i, j, +1.0);
               b[i] = ub;
               break;
            case LPX_DB:
               /* source: lb <= (L.F.) <= ub */
               /* result: (L.F.) - x' = lb, x' + x'' = ub - lb */
               i++; j++;
               ref[k] = i;
               new_coef(dsa, i, j, -1.0);
               b[i] = lb;
               i++;
               new_coef(dsa, i, j, +1.0);
               j++;
               new_coef(dsa, i, j, +1.0);
               b[i] = ub - lb;
               break;
            case LPX_FX:
               /* source: (L.F.) = lb */
               /* result: (L.F.) = lb */
               i++;
               ref[k] = i;
               b[i] = lb;
               break;
            default:
               xassert(type != type);
         }
      }
      /* transform columns (structural variables) */
      ind = xcalloc(1+orig_m, sizeof(int));
      val = xcalloc(1+orig_m, sizeof(double));
      for (k = 1; k <= orig_n; k++)
      {  type = lpx_get_col_type(lp, k);
         sjj = glp_get_sjj(lp, k);
         lb = lpx_get_col_lb(lp, k) / sjj;
         ub = lpx_get_col_ub(lp, k) / sjj;
         coef = lpx_get_obj_coef(lp, k) * sjj;
         len = lpx_get_mat_col(lp, k, ind, val);
         for (t = 1; t <= len; t++)
            val[t] *= (glp_get_rii(lp, ind[t]) * sjj);
         switch (type)
         {  case LPX_FR:
               /* source: -inf < x < +inf */
               /* result: x = x' - x'', x' >= 0, x'' >= 0 */
               j++;
               ref[orig_m+k] = j;
               for (t = 1; t <= len; t++)
               {  ii = ref[ind[t]];
                  if (ii != 0) new_coef(dsa, ii, j, +val[t]);
               }
               c[j] = +coef;
               j++;
               for (t = 1; t <= len; t++)
               {  ii = ref[ind[t]];
                  if (ii != 0) new_coef(dsa, ii, j, -val[t]);
               }
               c[j] = -coef;
               break;
            case LPX_LO:
               /* source: lb <= x < +inf */
               /* result: x = lb + x', x' >= 0 */
               j++;
               ref[orig_m+k] = j;
               for (t = 1; t <= len; t++)
               {  ii = ref[ind[t]];
                  if (ii != 0)
                  {  new_coef(dsa, ii, j, val[t]);
                     b[ii] -= val[t] * lb;
                  }
               }
               c[j] = +coef;
               c[0] += c[j] * lb;
               break;
            case LPX_UP:
               /* source: -inf < x <= ub */
               /* result: x = ub - x', x' >= 0 */
               j++;
               ref[orig_m+k] = j;
               for (t = 1; t <= len; t++)
               {  ii = ref[ind[t]];
                  if (ii != 0)
                  {  new_coef(dsa, ii, j, -val[t]);
                     b[ii] -= val[t] * ub;
                  }
               }
               c[j] = -coef;
               c[0] -= c[j] * ub;
               break;
            case LPX_DB:
               /* source: lb <= x <= ub */
               /* result: x = lb + x', x' + x'' = ub - lb */
               j++;
               ref[orig_m+k] = j;
               for (t = 1; t <= len; t++)
               {  ii = ref[ind[t]];
                  if (ii != 0)
                  {  new_coef(dsa, ii, j, val[t]);
                     b[ii] -= val[t] * lb;
                  }
               }
               c[j] = +coef;
               c[0] += c[j] * lb;
               i++;
               new_coef(dsa, i, j, +1.0);
               j++;
               new_coef(dsa, i, j, +1.0);
               b[i] = ub - lb;
               break;
            case LPX_FX:
               /* source: x = lb */
               /* result: just substitute */
               ref[orig_m+k] = 0;
               for (t = 1; t <= len; t++)
               {  ii = ref[ind[t]];
                  if (ii != 0) b[ii] -= val[t] * lb;
               }
               c[0] += coef * lb;
               break;
            default:
               xassert(type != type);
         }
      }
      xfree(ind);
      xfree(val);
      /* end of transformation */
      xassert(i == m && j == n);
      /* change the objective sign in case of maximization */
      if (lpx_get_obj_dir(lp) == LPX_MAX)
         for (j = 0; j <= n; j++) c[j] = -c[j];
      return;
}

static void restore(struct dsa *dsa, double row_pval[],
      double row_dval[], double col_pval[], double col_dval[])
{     /* restore solution of original LP */
      LPX *lp = dsa->lp;
      int orig_m = dsa->orig_m;
      int orig_n = dsa->orig_n;
      int *ref = dsa->ref;
      int m = dsa->m;
      double *x = dsa->x;
      double *y = dsa->y;
      int dir = lpx_get_obj_dir(lp);
      int i, j, k, type, t, len, *ind;
      double lb, ub, rii, sjj, temp, *val;
      /* compute primal values of structural variables */
      for (k = 1; k <= orig_n; k++)
      {  j = ref[orig_m+k];
         type = lpx_get_col_type(lp, k);
         sjj = glp_get_sjj(lp, k);
         lb = lpx_get_col_lb(lp, k) / sjj;
         ub = lpx_get_col_ub(lp, k) / sjj;
         switch (type)
         {  case LPX_FR:
               /* source: -inf < x < +inf */
               /* result: x = x' - x'', x' >= 0, x'' >= 0 */
               col_pval[k] = x[j] - x[j+1];
               break;
            case LPX_LO:
               /* source: lb <= x < +inf */
               /* result: x = lb + x', x' >= 0 */
               col_pval[k] = lb + x[j];
               break;
            case LPX_UP:
               /* source: -inf < x <= ub */
               /* result: x = ub - x', x' >= 0 */
               col_pval[k] = ub - x[j];
               break;
            case LPX_DB:
               /* source: lb <= x <= ub */
               /* result: x = lb + x', x' + x'' = ub - lb */
               col_pval[k] = lb + x[j];
               break;
            case LPX_FX:
               /* source: x = lb */
               /* result: just substitute */
               col_pval[k] = lb;
               break;
            default:
               xassert(type != type);
         }
      }
      /* compute primal values of auxiliary variables */
      /* xR = A * xS */
      ind = xcalloc(1+orig_n, sizeof(int));
      val = xcalloc(1+orig_n, sizeof(double));
      for (k = 1; k <= orig_m; k++)
      {  rii = glp_get_rii(lp, k);
         temp = 0.0;
         len = lpx_get_mat_row(lp, k, ind, val);
         for (t = 1; t <= len; t++)
         {  sjj = glp_get_sjj(lp, ind[t]);
            temp += (rii * val[t] * sjj) * col_pval[ind[t]];
         }
         row_pval[k] = temp;
      }
      xfree(ind);
      xfree(val);
      /* compute dual values of auxiliary variables */
      for (k = 1; k <= orig_m; k++)
      {  type = lpx_get_row_type(lp, k);
         i = ref[k];
         switch (type)
         {  case LPX_FR:
               xassert(i == 0);
               row_dval[k] = 0.0;
               break;
            case LPX_LO:
            case LPX_UP:
            case LPX_DB:
            case LPX_FX:
               xassert(1 <= i && i <= m);
               row_dval[k] = (dir == LPX_MIN ? +1.0 : -1.0) * y[i];
               break;
            default:
               xassert(type != type);
         }
      }
      /* compute dual values of structural variables */
      /* dS = cS - A' * (dR - cR) */
      ind = xcalloc(1+orig_m, sizeof(int));
      val = xcalloc(1+orig_m, sizeof(double));
      for (k = 1; k <= orig_n; k++)
      {  sjj = glp_get_sjj(lp, k);
         temp = lpx_get_obj_coef(lp, k) / sjj;
         len = lpx_get_mat_col(lp, k, ind, val);
         for (t = 1; t <= len; t++)
         {  rii = glp_get_rii(lp, ind[t]);
            temp -= (rii * val[t] * sjj) * row_dval[ind[t]];
         }
         col_dval[k] = temp;
      }
      xfree(ind);
      xfree(val);
      /* unscale solution of original LP */
      for (i = 1; i <= orig_m; i++)
      {  rii = glp_get_rii(lp, i);
         row_pval[i] /= rii;
         row_dval[i] *= rii;
      }
      for (j = 1; j <= orig_n; j++)
      {  sjj = glp_get_sjj(lp, j);
         col_pval[j] *= sjj;
         col_dval[j] /= sjj;
      }
      return;
}

int glp_interior(glp_prob *_lp, const void *parm)
{     /* easy-to-use driver to the interior-point method */
      struct dsa _dsa, *dsa = &_dsa;
      int ret, status;
      double *row_pval, *row_dval, *col_pval, *col_dval;
      if (parm != NULL)
         xerror("glp_interior: parm = %p; invalid parameter\n", parm);
      /* interior-point solution is currently undefined */
      _lp->ipt_stat = GLP_UNDEF;
      _lp->ipt_obj = 0.0;
      /* initialize working area */
      dsa->lp = _lp;
      dsa->orig_m = lpx_get_num_rows(dsa->lp);
      dsa->orig_n = lpx_get_num_cols(dsa->lp);
      dsa->ref = NULL;
      dsa->m = 0;
      dsa->n = 0;
      dsa->size = 0;
      dsa->ne = 0;
      dsa->ia = NULL;
      dsa->ja = NULL;
      dsa->ar = NULL;
      dsa->b = NULL;
      dsa->c = NULL;
      dsa->x = NULL;
      dsa->y = NULL;
      dsa->z = NULL;
      /* check if the problem is empty */
      if (!(dsa->orig_m > 0 && dsa->orig_n > 0))
      {  xprintf("glp_interior: problem has no rows and/or columns\n");
         ret = GLP_EFAIL;
         goto done;
      }
      /* issue warning about dense columns */
      if (dsa->orig_m >= 200)
      {  int j, len, ndc = 0;
         for (j = 1; j <= dsa->orig_n; j++)
         {  len = lpx_get_mat_col(dsa->lp, j, NULL, NULL);
            if ((double)len > 0.30 * (double)dsa->orig_m) ndc++;
         }
         if (ndc == 1)
            xprintf("glp_interior: WARNING: PROBLEM HAS ONE DENSE COLUM"
               "N\n");
         else if (ndc > 0)
            xprintf("glp_interior: WARNING: PROBLEM HAS %d DENSE COLUMN"
               "S\n", ndc);
      }
      /* determine dimension of transformed LP */
      xprintf("glp_interior: original LP problem has %d rows and %d col"
         "umns\n", dsa->orig_m, dsa->orig_n);
      calc_mn(dsa);
      xprintf("glp_interior: transformed LP problem has %d rows and %d "
         "columns\n", dsa->m, dsa->n);
      /* transform original LP to standard formulation */
      dsa->ref = xcalloc(1+dsa->orig_m+dsa->orig_n, sizeof(int));
      dsa->size = lpx_get_num_nz(dsa->lp);
      if (dsa->size < dsa->m) dsa->size = dsa->m;
      if (dsa->size < dsa->n) dsa->size = dsa->n;
      dsa->ne = 0;
      dsa->ia = xcalloc(1+dsa->size, sizeof(int));
      dsa->ja = xcalloc(1+dsa->size, sizeof(int));
      dsa->ar = xcalloc(1+dsa->size, sizeof(double));
      dsa->b = xcalloc(1+dsa->m, sizeof(double));
      dsa->c = xcalloc(1+dsa->n, sizeof(double));
      transform(dsa);
      /* solve the transformed LP problem */
      {  int *A_ptr = xcalloc(1+dsa->m+1, sizeof(int));
         int *A_ind = xcalloc(1+dsa->ne, sizeof(int));
         double *A_val = xcalloc(1+dsa->ne, sizeof(double));
         int i, k, pos, len;
         /* determine row lengths */
         for (i = 1; i <= dsa->m; i++)
            A_ptr[i] = 0;
         for (k = 1; k <= dsa->ne; k++)
            A_ptr[dsa->ia[k]]++;
         /* set up row pointers */
         pos = 1;
         for (i = 1; i <= dsa->m; i++)
            len = A_ptr[i], pos += len, A_ptr[i] = pos;
         A_ptr[dsa->m+1] = pos;
         xassert(pos - 1 == dsa->ne);
         /* build the matrix */
         for (k = 1; k <= dsa->ne; k++)
         {  pos = --A_ptr[dsa->ia[k]];
            A_ind[pos] = dsa->ja[k];
            A_val[pos] = dsa->ar[k];
         }
         xfree(dsa->ia), dsa->ia = NULL;
         xfree(dsa->ja), dsa->ja = NULL;
         xfree(dsa->ar), dsa->ar = NULL;
         dsa->x = xcalloc(1+dsa->n, sizeof(double));
         dsa->y = xcalloc(1+dsa->m, sizeof(double));
         dsa->z = xcalloc(1+dsa->n, sizeof(double));
         ret = ipm_main(dsa->m, dsa->n, A_ptr, A_ind, A_val, dsa->b,
            dsa->c, dsa->x, dsa->y, dsa->z);
         xfree(A_ptr);
         xfree(A_ind);
         xfree(A_val);
         xfree(dsa->b), dsa->b = NULL;
         xfree(dsa->c), dsa->c = NULL;
      }
      /* analyze return code reported by the solver */
      switch (ret)
      {  case 0:
            /* optimal solution found */
            ret = 0;
            break;
         case 1:
            /* problem has no feasible (primal or dual) solution */
            ret = GLP_ENOFEAS;
            break;
         case 2:
            /* no convergence */
            ret = GLP_ENOCVG;
            break;
         case 3:
            /* iteration limit exceeded */
            ret = GLP_EITLIM;
            break;
         case 4:
            /* numerical instability on solving Newtonian system */
            ret = GLP_EINSTAB;
            break;
         default:
            xassert(ret != ret);
      }
      /* recover solution of original LP */
      row_pval = xcalloc(1+dsa->orig_m, sizeof(double));
      row_dval = xcalloc(1+dsa->orig_m, sizeof(double));
      col_pval = xcalloc(1+dsa->orig_n, sizeof(double));
      col_dval = xcalloc(1+dsa->orig_n, sizeof(double));
      restore(dsa, row_pval, row_dval, col_pval, col_dval);
      xfree(dsa->ref), dsa->ref = NULL;
      xfree(dsa->x), dsa->x = NULL;
      xfree(dsa->y), dsa->y = NULL;
      xfree(dsa->z), dsa->z = NULL;
      /* store solution components into the problem object */
      status = (ret == 0 ? LPX_T_OPT : LPX_T_UNDEF);
      lpx_put_ipt_soln(dsa->lp, status, row_pval, row_dval, col_pval,
         col_dval);
      xfree(row_pval);
      xfree(row_dval);
      xfree(col_pval);
      xfree(col_dval);
done: /* return to the calling program */
      return ret;
}

/***********************************************************************
*  NAME
*
*  glp_ipt_status - retrieve status of interior-point solution
*
*  SYNOPSIS
*
*  int glp_ipt_status(glp_prob *lp);
*
*  RETURNS
*
*  The routine glp_ipt_status reports the status of solution found by
*  the interior-point solver as follows:
*
*  GLP_UNDEF  - interior-point solution is undefined;
*  GLP_OPT    - interior-point solution is optimal. */

int glp_ipt_status(glp_prob *lp)
{     int ipt_stat = lp->ipt_stat;
      return ipt_stat;
}

/***********************************************************************
*  NAME
*
*  glp_ipt_obj_val - retrieve objective value (interior point)
*
*  SYNOPSIS
*
*  double glp_ipt_obj_val(glp_prob *lp);
*
*  RETURNS
*
*  The routine glp_ipt_obj_val returns value of the objective function
*  for interior-point solution. */

double glp_ipt_obj_val(glp_prob *lp)
{     struct LPXCPS *cps = lp->cps;
      double z;
      z = lp->ipt_obj;
      if (cps->round && fabs(z) < 1e-9) z = 0.0;
      return z;
}

/***********************************************************************
*  NAME
*
*  glp_ipt_row_prim - retrieve row primal value (interior point)
*
*  SYNOPSIS
*
*  double glp_ipt_row_prim(glp_prob *lp, int i);
*
*  RETURNS
*
*  The routine glp_ipt_row_prim returns primal value of the auxiliary
*  variable associated with i-th row. */

double glp_ipt_row_prim(glp_prob *lp, int i)
{     struct LPXCPS *cps = lp->cps;
      double pval;
      if (!(1 <= i && i <= lp->m))
         xerror("glp_ipt_row_prim: i = %d; row number out of range\n",
            i);
      pval = lp->row[i]->pval;
      if (cps->round && fabs(pval) < 1e-9) pval = 0.0;
      return pval;
}

/***********************************************************************
*  NAME
*
*  glp_ipt_row_dual - retrieve row dual value (interior point)
*
*  SYNOPSIS
*
*  double glp_ipt_row_dual(glp_prob *lp, int i);
*
*  RETURNS
*
*  The routine glp_ipt_row_dual returns dual value (i.e. reduced cost)
*  of the auxiliary variable associated with i-th row. */

double glp_ipt_row_dual(glp_prob *lp, int i)
{     struct LPXCPS *cps = lp->cps;
      double dval;
      if (!(1 <= i && i <= lp->m))
         xerror("glp_ipt_row_dual: i = %d; row number out of range\n",
            i);
      dval = lp->row[i]->dval;
      if (cps->round && fabs(dval) < 1e-9) dval = 0.0;
      return dval;
}

/***********************************************************************
*  NAME
*
*  glp_ipt_col_prim - retrieve column primal value (interior point)
*
*  SYNOPSIS
*
*  double glp_ipt_col_prim(glp_prob *lp, int j);
*
*  RETURNS
*
*  The routine glp_ipt_col_prim returns primal value of the structural
*  variable associated with j-th column. */

double glp_ipt_col_prim(glp_prob *lp, int j)
{     struct LPXCPS *cps = lp->cps;
      double pval;
      if (!(1 <= j && j <= lp->n))
         xerror("glp_ipt_col_prim: j = %d; column number out of range\n"
            , j);
      pval = lp->col[j]->pval;
      if (cps->round && fabs(pval) < 1e-9) pval = 0.0;
      return pval;
}

/***********************************************************************
*  NAME
*
*  glp_ipt_col_dual - retrieve column dual value (interior point)
*
*  SYNOPSIS
*
*  #include "glplpx.h"
*  double glp_ipt_col_dual(glp_prob *lp, int j);
*
*  RETURNS
*
*  The routine glp_ipt_col_dual returns dual value (i.e. reduced cost)
*  of the structural variable associated with j-th column. */

double glp_ipt_col_dual(glp_prob *lp, int j)
{     struct LPXCPS *cps = lp->cps;
      double dval;
      if (!(1 <= j && j <= lp->n))
         xerror("glp_ipt_col_dual: j = %d; column number out of range\n"
            , j);
      dval = lp->col[j]->dval;
      if (cps->round && fabs(dval) < 1e-9) dval = 0.0;
      return dval;
}

/* eof */
