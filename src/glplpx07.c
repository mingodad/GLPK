/* glplpx07.c (additional utility routines) */

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
#define xfault xerror
#define strerror(errno) xerrmsg()

/*----------------------------------------------------------------------
-- lpx_print_prob - write problem data in plain text format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_print_prob(LPX *lp, char *fname);
--
-- *Description*
--
-- The routine lpx_print_prob writes data from a problem object, which
-- the parameter lp points to, to an output text file, whose name is the
-- character string fname, in plain text format.
--
-- *Returns*
--
-- If the operation is successful, the routine returns zero. Otherwise
-- the routine prints an error message and returns non-zero. */

#define row_name row_name2
#define col_name col_name2

static char *row_name(LPX *lp, int i, char name[255+1])
{     const char *str;
      str = lpx_get_row_name(lp, i);
      if (str == NULL)
         sprintf(name, "R.%d", i);
      else
         strcpy(name, str);
      return name;
}

static char *col_name(LPX *lp, int j, char name[255+1])
{     const char *str;
      str = lpx_get_col_name(lp, j);
      if (str == NULL)
         sprintf(name, "C.%d", j);
      else
         strcpy(name, str);
      return name;
}

int lpx_print_prob(LPX *lp, const char *fname)
{     XFILE *fp;
      int m, n, mip, i, j, len, t, type, *ndx;
      double coef, lb, ub, *val;
      char *str, name[255+1];
      xprintf("lpx_write_prob: writing problem data to `%s'...\n",
         fname);
      fp = xfopen(fname, "w");
      if (fp == NULL)
      {  xprintf("lpx_write_prob: unable to create `%s' - %s\n",
            fname, strerror(errno));
         goto fail;
      }
      m = lpx_get_num_rows(lp);
      n = lpx_get_num_cols(lp);
      mip = (lpx_get_class(lp) == LPX_MIP);
      str = (void *)lpx_get_prob_name(lp);
      xfprintf(fp, "Problem:    %s\n", str == NULL ? "(unnamed)" : str);
      xfprintf(fp, "Class:      %s\n", !mip ? "LP" : "MIP");
      xfprintf(fp, "Rows:       %d\n", m);
      if (!mip)
         xfprintf(fp, "Columns:    %d\n", n);
      else
         xfprintf(fp, "Columns:    %d (%d integer, %d binary)\n",
            n, lpx_get_num_int(lp), lpx_get_num_bin(lp));
      xfprintf(fp, "Non-zeros:  %d\n", lpx_get_num_nz(lp));
      xfprintf(fp, "\n");
      xfprintf(fp, "*** OBJECTIVE FUNCTION ***\n");
      xfprintf(fp, "\n");
      switch (lpx_get_obj_dir(lp))
      {  case LPX_MIN:
            xfprintf(fp, "Minimize:");
            break;
         case LPX_MAX:
            xfprintf(fp, "Maximize:");
            break;
         default:
            xassert(lp != lp);
      }
      str = (void *)lpx_get_obj_name(lp);
      xfprintf(fp, " %s\n", str == NULL ? "(unnamed)" : str);
      coef = lpx_get_obj_coef(lp, 0);
      if (coef != 0.0)
         xfprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, coef,
            "(constant term)");
      for (i = 1; i <= m; i++)
#if 0
      {  coef = lpx_get_row_coef(lp, i);
#else
      {  coef = 0.0;
#endif
         if (coef != 0.0)
            xfprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, coef,
               row_name(lp, i, name));
      }
      for (j = 1; j <= n; j++)
      {  coef = lpx_get_obj_coef(lp, j);
         if (coef != 0.0)
            xfprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, coef,
               col_name(lp, j, name));
      }
      xfprintf(fp, "\n");
      xfprintf(fp, "*** ROWS (CONSTRAINTS) ***\n");
      ndx = xcalloc(1+n, sizeof(int));
      val = xcalloc(1+n, sizeof(double));
      for (i = 1; i <= m; i++)
      {  xfprintf(fp, "\n");
         xfprintf(fp, "Row %d: %s", i, row_name(lp, i, name));
         lpx_get_row_bnds(lp, i, &type, &lb, &ub);
         switch (type)
         {  case LPX_FR:
               xfprintf(fp, " free");
               break;
            case LPX_LO:
               xfprintf(fp, " >= %.*g", DBL_DIG, lb);
               break;
            case LPX_UP:
               xfprintf(fp, " <= %.*g", DBL_DIG, ub);
               break;
            case LPX_DB:
               xfprintf(fp, " >= %.*g <= %.*g", DBL_DIG, lb, DBL_DIG,
                  ub);
               break;
            case LPX_FX:
               xfprintf(fp, " = %.*g", DBL_DIG, lb);
               break;
            default:
               xassert(type != type);
         }
         xfprintf(fp, "\n");
#if 0
         coef = lpx_get_row_coef(lp, i);
#else
         coef = 0.0;
#endif
         if (coef != 0.0)
            xfprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, coef,
               "(objective)");
         len = lpx_get_mat_row(lp, i, ndx, val);
         for (t = 1; t <= len; t++)
            xfprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, val[t],
               col_name(lp, ndx[t], name));
      }
      xfree(ndx);
      xfree(val);
      xfprintf(fp, "\n");
      xfprintf(fp, "*** COLUMNS (VARIABLES) ***\n");
      ndx = xcalloc(1+m, sizeof(int));
      val = xcalloc(1+m, sizeof(double));
      for (j = 1; j <= n; j++)
      {  xfprintf(fp, "\n");
         xfprintf(fp, "Col %d: %s", j, col_name(lp, j, name));
         if (mip)
         {  switch (lpx_get_col_kind(lp, j))
            {  case LPX_CV:
                  break;
               case LPX_IV:
                  xfprintf(fp, " integer");
                  break;
               default:
                  xassert(lp != lp);
            }
         }
         lpx_get_col_bnds(lp, j, &type, &lb, &ub);
         switch (type)
         {  case LPX_FR:
               xfprintf(fp, " free");
               break;
            case LPX_LO:
               xfprintf(fp, " >= %.*g", DBL_DIG, lb);
               break;
            case LPX_UP:
               xfprintf(fp, " <= %.*g", DBL_DIG, ub);
               break;
            case LPX_DB:
               xfprintf(fp, " >= %.*g <= %.*g", DBL_DIG, lb, DBL_DIG,
                  ub);
               break;
            case LPX_FX:
               xfprintf(fp, " = %.*g", DBL_DIG, lb);
               break;
            default:
               xassert(type != type);
         }
         xfprintf(fp, "\n");
         coef = lpx_get_obj_coef(lp, j);
         if (coef != 0.0)
            xfprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, coef,
               "(objective)");
         len = lpx_get_mat_col(lp, j, ndx, val);
         for (t = 1; t <= len; t++)
            xfprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, val[t],
               row_name(lp, ndx[t], name));
      }
      xfree(ndx);
      xfree(val);
      xfprintf(fp, "\n");
      xfprintf(fp, "End of output\n");
      xfflush(fp);
      if (xferror(fp))
      {  xprintf("lpx_write_prob: write error on `%s' - %s\n", fname,
            strerror(errno));
         goto fail;
      }
      xfclose(fp);
      return 0;
fail: if (fp != NULL) xfclose(fp);
      return 1;
}

#undef row_name
#undef col_name

/*----------------------------------------------------------------------
-- lpx_print_sol - write LP problem solution in printable format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_print_sol(LPX *lp, char *fname);
--
-- *Description*
--
-- The routine lpx_print_sol writes the current basic solution of an LP
-- problem, which is specified by the pointer lp, to a text file, whose
-- name is the character string fname, in printable format.
--
-- Information reported by the routine lpx_print_sol is intended mainly
-- for visual analysis.
--
-- *Returns*
--
-- If the operation was successful, the routine returns zero. Otherwise
-- the routine prints an error message and returns non-zero. */

int lpx_print_sol(LPX *lp, const char *fname)
{     XFILE *fp;
      int what, round;
      xprintf(
         "lpx_print_sol: writing LP problem solution to `%s'...\n",
         fname);
      fp = xfopen(fname, "w");
      if (fp == NULL)
      {  xprintf("lpx_print_sol: can't create `%s' - %s\n", fname,
            strerror(errno));
         goto fail;
      }
      /* problem name */
      {  const char *name;
         name = lpx_get_prob_name(lp);
         if (name == NULL) name = "";
         xfprintf(fp, "%-12s%s\n", "Problem:", name);
      }
      /* number of rows (auxiliary variables) */
      {  int nr;
         nr = lpx_get_num_rows(lp);
         xfprintf(fp, "%-12s%d\n", "Rows:", nr);
      }
      /* number of columns (structural variables) */
      {  int nc;
         nc = lpx_get_num_cols(lp);
         xfprintf(fp, "%-12s%d\n", "Columns:", nc);
      }
      /* number of non-zeros (constraint coefficients) */
      {  int nz;
         nz = lpx_get_num_nz(lp);
         xfprintf(fp, "%-12s%d\n", "Non-zeros:", nz);
      }
      /* solution status */
      {  int status;
         status = lpx_get_status(lp);
         xfprintf(fp, "%-12s%s\n", "Status:",
            status == LPX_OPT    ? "OPTIMAL" :
            status == LPX_FEAS   ? "FEASIBLE" :
            status == LPX_INFEAS ? "INFEASIBLE (INTERMEDIATE)" :
            status == LPX_NOFEAS ? "INFEASIBLE (FINAL)" :
            status == LPX_UNBND  ? "UNBOUNDED" :
            status == LPX_UNDEF  ? "UNDEFINED" : "???");
      }
      /* objective function */
      {  char *name;
         int dir;
         double obj;
         name = (void *)lpx_get_obj_name(lp);
         dir = lpx_get_obj_dir(lp);
         obj = lpx_get_obj_val(lp);
         xfprintf(fp, "%-12s%s%s%.10g %s\n", "Objective:",
            name == NULL ? "" : name,
            name == NULL ? "" : " = ", obj,
            dir == LPX_MIN ? "(MINimum)" :
            dir == LPX_MAX ? "(MAXimum)" : "(" "???" ")");
      }
      /* main sheet */
      for (what = 1; what <= 2; what++)
      {  int mn, ij;
         xfprintf(fp, "\n");
         xfprintf(fp, "   No. %-12s St   Activity     Lower bound   Upp"
            "er bound    Marginal\n",
            what == 1 ? "  Row name" : "Column name");
         xfprintf(fp, "------ ------------ -- ------------- -----------"
            "-- ------------- -------------\n");
         mn = (what == 1 ? lpx_get_num_rows(lp) : lpx_get_num_cols(lp));
         for (ij = 1; ij <= mn; ij++)
         {  const char *name;
            int typx, tagx;
            double lb, ub, vx, dx;
            if (what == 1)
            {  name = lpx_get_row_name(lp, ij);
               if (name == NULL) name = "";
               lpx_get_row_bnds(lp, ij, &typx, &lb, &ub);
               round = lpx_get_int_parm(lp, LPX_K_ROUND);
               lpx_set_int_parm(lp, LPX_K_ROUND, 1);
               lpx_get_row_info(lp, ij, &tagx, &vx, &dx);
               lpx_set_int_parm(lp, LPX_K_ROUND, round);
            }
            else
            {  name = lpx_get_col_name(lp, ij);
               if (name == NULL) name = "";
               lpx_get_col_bnds(lp, ij, &typx, &lb, &ub);
               round = lpx_get_int_parm(lp, LPX_K_ROUND);
               lpx_set_int_parm(lp, LPX_K_ROUND, 1);
               lpx_get_col_info(lp, ij, &tagx, &vx, &dx);
               lpx_set_int_parm(lp, LPX_K_ROUND, round);
            }
            /* row/column ordinal number */
            xfprintf(fp, "%6d ", ij);
            /* row column/name */
            if (strlen(name) <= 12)
               xfprintf(fp, "%-12s ", name);
            else
               xfprintf(fp, "%s\n%20s", name, "");
            /* row/column status */
            xfprintf(fp, "%s ",
               tagx == LPX_BS ? "B " :
               tagx == LPX_NL ? "NL" :
               tagx == LPX_NU ? "NU" :
               tagx == LPX_NF ? "NF" :
               tagx == LPX_NS ? "NS" : "??");
            /* row/column primal activity */
            xfprintf(fp, "%13.6g ", vx);
            /* row/column lower bound */
            if (typx == LPX_LO || typx == LPX_DB || typx == LPX_FX)
               xfprintf(fp, "%13.6g ", lb);
            else
               xfprintf(fp, "%13s ", "");
            /* row/column upper bound */
            if (typx == LPX_UP || typx == LPX_DB)
               xfprintf(fp, "%13.6g ", ub);
            else if (typx == LPX_FX)
               xfprintf(fp, "%13s ", "=");
            else
               xfprintf(fp, "%13s ", "");
            /* row/column dual activity */
            if (tagx != LPX_BS)
            {  if (dx == 0.0)
                  xfprintf(fp, "%13s", "< eps");
               else
                  xfprintf(fp, "%13.6g", dx);
            }
            /* end of line */
            xfprintf(fp, "\n");
         }
      }
      xfprintf(fp, "\n");
#if 1
      if (lpx_get_prim_stat(lp) != LPX_P_UNDEF &&
          lpx_get_dual_stat(lp) != LPX_D_UNDEF)
      {  int m = lpx_get_num_rows(lp);
         LPXKKT kkt;
         xfprintf(fp, "Karush-Kuhn-Tucker optimality conditions:\n\n");
         lpx_check_kkt(lp, 1, &kkt);
         xfprintf(fp, "KKT.PE: max.abs.err. = %.2e on row %d\n",
            kkt.pe_ae_max, kkt.pe_ae_row);
         xfprintf(fp, "        max.rel.err. = %.2e on row %d\n",
            kkt.pe_re_max, kkt.pe_re_row);
         switch (kkt.pe_quality)
         {  case 'H':
               xfprintf(fp, "        High quality\n");
               break;
            case 'M':
               xfprintf(fp, "        Medium quality\n");
               break;
            case 'L':
               xfprintf(fp, "        Low quality\n");
               break;
            default:
               xfprintf(fp, "        PRIMAL SOLUTION IS WRONG\n");
               break;
         }
         xfprintf(fp, "\n");
         xfprintf(fp, "KKT.PB: max.abs.err. = %.2e on %s %d\n",
            kkt.pb_ae_max, kkt.pb_ae_ind <= m ? "row" : "column",
            kkt.pb_ae_ind <= m ? kkt.pb_ae_ind : kkt.pb_ae_ind - m);
         xfprintf(fp, "        max.rel.err. = %.2e on %s %d\n",
            kkt.pb_re_max, kkt.pb_re_ind <= m ? "row" : "column",
            kkt.pb_re_ind <= m ? kkt.pb_re_ind : kkt.pb_re_ind - m);
         switch (kkt.pb_quality)
         {  case 'H':
               xfprintf(fp, "        High quality\n");
               break;
            case 'M':
               xfprintf(fp, "        Medium quality\n");
               break;
            case 'L':
               xfprintf(fp, "        Low quality\n");
               break;
            default:
               xfprintf(fp, "        PRIMAL SOLUTION IS INFEASIBLE\n");
               break;
         }
         xfprintf(fp, "\n");
         xfprintf(fp, "KKT.DE: max.abs.err. = %.2e on column %d\n",
            kkt.de_ae_max, kkt.de_ae_col);
         xfprintf(fp, "        max.rel.err. = %.2e on column %d\n",
            kkt.de_re_max, kkt.de_re_col);
         switch (kkt.de_quality)
         {  case 'H':
               xfprintf(fp, "        High quality\n");
               break;
            case 'M':
               xfprintf(fp, "        Medium quality\n");
               break;
            case 'L':
               xfprintf(fp, "        Low quality\n");
               break;
            default:
               xfprintf(fp, "        DUAL SOLUTION IS WRONG\n");
               break;
         }
         xfprintf(fp, "\n");
         xfprintf(fp, "KKT.DB: max.abs.err. = %.2e on %s %d\n",
            kkt.db_ae_max, kkt.db_ae_ind <= m ? "row" : "column",
            kkt.db_ae_ind <= m ? kkt.db_ae_ind : kkt.db_ae_ind - m);
         xfprintf(fp, "        max.rel.err. = %.2e on %s %d\n",
            kkt.db_re_max, kkt.db_re_ind <= m ? "row" : "column",
            kkt.db_re_ind <= m ? kkt.db_re_ind : kkt.db_re_ind - m);
         switch (kkt.db_quality)
         {  case 'H':
               xfprintf(fp, "        High quality\n");
               break;
            case 'M':
               xfprintf(fp, "        Medium quality\n");
               break;
            case 'L':
               xfprintf(fp, "        Low quality\n");
               break;
            default:
               xfprintf(fp, "        DUAL SOLUTION IS INFEASIBLE\n");
               break;
         }
         xfprintf(fp, "\n");
      }
#endif
#if 1
      if (lpx_get_status(lp) == LPX_UNBND)
      {  int m = lpx_get_num_rows(lp);
         int k = lpx_get_ray_info(lp);
         xfprintf(fp, "Unbounded ray: %s %d\n",
            k <= m ? "row" : "column", k <= m ? k : k - m);
         xfprintf(fp, "\n");
      }
#endif
      xfprintf(fp, "End of output\n");
      xfflush(fp);
      if (xferror(fp))
      {  xprintf("lpx_print_sol: can't write to `%s' - %s\n", fname,
            strerror(errno));
         goto fail;
      }
      xfclose(fp);
      return 0;
fail: if (fp != NULL) xfclose(fp);
      return 1;
}

/*----------------------------------------------------------------------
-- lpx_print_ips - write interior point solution in printable format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_print_ips(LPX *lp, char *fname);
--
-- *Description*
--
-- The routine lpx_print_ips writes the current interior point solution
-- of an LP problem, which the parameter lp points to, to a text file,
-- whose name is the character string fname, in printable format.
--
-- Information reported by the routine lpx_print_ips is intended mainly
-- for visual analysis.
--
-- *Returns*
--
-- If the operation was successful, the routine returns zero. Otherwise
-- the routine prints an error message and returns non-zero. */

int lpx_print_ips(LPX *lp, const char *fname)
{     XFILE *fp;
      int what, round;
      xprintf("lpx_print_ips: writing LP problem solution to `%s'...\n",
         fname);
      fp = xfopen(fname, "w");
      if (fp == NULL)
      {  xprintf("lpx_print_ips: can't create `%s' - %s\n", fname,
            strerror(errno));
         goto fail;
      }
      /* problem name */
      {  const char *name;
         name = lpx_get_prob_name(lp);
         if (name == NULL) name = "";
         xfprintf(fp, "%-12s%s\n", "Problem:", name);
      }
      /* number of rows (auxiliary variables) */
      {  int nr;
         nr = lpx_get_num_rows(lp);
         xfprintf(fp, "%-12s%d\n", "Rows:", nr);
      }
      /* number of columns (structural variables) */
      {  int nc;
         nc = lpx_get_num_cols(lp);
         xfprintf(fp, "%-12s%d\n", "Columns:", nc);
      }
      /* number of non-zeros (constraint coefficients) */
      {  int nz;
         nz = lpx_get_num_nz(lp);
         xfprintf(fp, "%-12s%d\n", "Non-zeros:", nz);
      }
      /* solution status */
      {  int status;
         status = lpx_ipt_status(lp);
         xfprintf(fp, "%-12s%s\n", "Status:",
            status == LPX_T_UNDEF  ? "INTERIOR UNDEFINED" :
            status == LPX_T_OPT    ? "INTERIOR OPTIMAL" : "???");
      }
      /* objective function */
      {  char *name;
         int dir;
         double obj;
         name = (void *)lpx_get_obj_name(lp);
         dir = lpx_get_obj_dir(lp);
         obj = lpx_ipt_obj_val(lp);
         xfprintf(fp, "%-12s%s%s%.10g %s\n", "Objective:",
            name == NULL ? "" : name,
            name == NULL ? "" : " = ", obj,
            dir == LPX_MIN ? "(MINimum)" :
            dir == LPX_MAX ? "(MAXimum)" : "(" "???" ")");
      }
      /* main sheet */
      for (what = 1; what <= 2; what++)
      {  int mn, ij;
         xfprintf(fp, "\n");
         xfprintf(fp, "   No. %-12s      Activity     Lower bound   Upp"
            "er bound    Marginal\n",
            what == 1 ? "  Row name" : "Column name");
         xfprintf(fp, "------ ------------    ------------- -----------"
            "-- ------------- -------------\n");
         mn = (what == 1 ? lpx_get_num_rows(lp) : lpx_get_num_cols(lp));
         for (ij = 1; ij <= mn; ij++)
         {  const char *name;
            int typx /*, tagx */;
            double lb, ub, vx, dx;
            if (what == 1)
            {  name = lpx_get_row_name(lp, ij);
               if (name == NULL) name = "";
               lpx_get_row_bnds(lp, ij, &typx, &lb, &ub);
               round = lpx_get_int_parm(lp, LPX_K_ROUND);
               lpx_set_int_parm(lp, LPX_K_ROUND, 1);
               vx = lpx_ipt_row_prim(lp, ij);
               dx = lpx_ipt_row_dual(lp, ij);
               lpx_set_int_parm(lp, LPX_K_ROUND, round);
            }
            else
            {  name = lpx_get_col_name(lp, ij);
               if (name == NULL) name = "";
               lpx_get_col_bnds(lp, ij, &typx, &lb, &ub);
               round = lpx_get_int_parm(lp, LPX_K_ROUND);
               lpx_set_int_parm(lp, LPX_K_ROUND, 1);
               vx = lpx_ipt_col_prim(lp, ij);
               dx = lpx_ipt_col_dual(lp, ij);
               lpx_set_int_parm(lp, LPX_K_ROUND, round);
            }
            /* row/column ordinal number */
            xfprintf(fp, "%6d ", ij);
            /* row column/name */
            if (strlen(name) <= 12)
               xfprintf(fp, "%-12s ", name);
            else
               xfprintf(fp, "%s\n%20s", name, "");
            /* two positions are currently not used */
            xfprintf(fp, "   ");
            /* row/column primal activity */
            xfprintf(fp, "%13.6g ", vx);
            /* row/column lower bound */
            if (typx == LPX_LO || typx == LPX_DB || typx == LPX_FX)
               xfprintf(fp, "%13.6g ", lb);
            else
               xfprintf(fp, "%13s ", "");
            /* row/column upper bound */
            if (typx == LPX_UP || typx == LPX_DB)
               xfprintf(fp, "%13.6g ", ub);
            else if (typx == LPX_FX)
               xfprintf(fp, "%13s ", "=");
            else
               xfprintf(fp, "%13s ", "");
            /* row/column dual activity */
            xfprintf(fp, "%13.6g", dx);
            /* end of line */
            xfprintf(fp, "\n");
         }
      }
      xfprintf(fp, "\n");
      xfprintf(fp, "End of output\n");
      xfflush(fp);
      if (xferror(fp))
      {  xprintf("lpx_print_ips: can't write to `%s' - %s\n", fname,
            strerror(errno));
         goto fail;
      }
      xfclose(fp);
      return 0;
fail: if (fp != NULL) xfclose(fp);
      return 1;
}

/*----------------------------------------------------------------------
-- lpx_print_mip - write MIP problem solution in printable format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_print_mip(LPX *lp, char *fname);
--
-- *Description*
--
-- The routine lpx_print_mip writes a best known integer solution of
-- a MIP problem, which is specified by the pointer lp, to a text file,
-- whose name is the character string fname, in printable format.
--
-- Information reported by the routine lpx_print_mip is intended mainly
-- for visual analysis.
--
-- *Returns*
--
-- If the operation was successful, the routine returns zero. Otherwise
-- the routine prints an error message and returns non-zero. */

int lpx_print_mip(LPX *lp, const char *fname)
{     XFILE *fp;
      int what, round;
#if 0
      if (lpx_get_class(lp) != LPX_MIP)
         fault("lpx_print_mip: error -- not a MIP problem");
#endif
      xprintf(
         "lpx_print_mip: writing MIP problem solution to `%s'...\n",
         fname);
      fp = xfopen(fname, "w");
      if (fp == NULL)
      {  xprintf("lpx_print_mip: can't create `%s' - %s\n", fname,
            strerror(errno));
         goto fail;
      }
      /* problem name */
      {  const char *name;
         name = lpx_get_prob_name(lp);
         if (name == NULL) name = "";
         xfprintf(fp, "%-12s%s\n", "Problem:", name);
      }
      /* number of rows (auxiliary variables) */
      {  int nr;
         nr = lpx_get_num_rows(lp);
         xfprintf(fp, "%-12s%d\n", "Rows:", nr);
      }
      /* number of columns (structural variables) */
      {  int nc, nc_int, nc_bin;
         nc = lpx_get_num_cols(lp);
         nc_int = lpx_get_num_int(lp);
         nc_bin = lpx_get_num_bin(lp);
         xfprintf(fp, "%-12s%d (%d integer, %d binary)\n", "Columns:",
            nc, nc_int, nc_bin);
      }
      /* number of non-zeros (constraint coefficients) */
      {  int nz;
         nz = lpx_get_num_nz(lp);
         xfprintf(fp, "%-12s%d\n", "Non-zeros:", nz);
      }
      /* solution status */
      {  int status;
         status = lpx_mip_status(lp);
         xfprintf(fp, "%-12s%s\n", "Status:",
            status == LPX_I_UNDEF  ? "INTEGER UNDEFINED" :
            status == LPX_I_OPT    ? "INTEGER OPTIMAL" :
            status == LPX_I_FEAS   ? "INTEGER NON-OPTIMAL" :
            status == LPX_I_NOFEAS ? "INTEGER EMPTY" : "???");
      }
      /* objective function */
      {  char *name;
         int dir;
         double mip_obj;
         name = (void *)lpx_get_obj_name(lp);
         dir = lpx_get_obj_dir(lp);
         mip_obj = lpx_mip_obj_val(lp);
         xfprintf(fp, "%-12s%s%s%.10g %s\n", "Objective:",
            name == NULL ? "" : name,
            name == NULL ? "" : " = ", mip_obj,
            dir == LPX_MIN ? "(MINimum)" :
            dir == LPX_MAX ? "(MAXimum)" : "(" "???" ")");
      }
      /* main sheet */
      for (what = 1; what <= 2; what++)
      {  int mn, ij;
         xfprintf(fp, "\n");
         xfprintf(fp, "   No. %-12s      Activity     Lower bound   Upp"
            "er bound\n",
            what == 1 ? "  Row name" : "Column name");
         xfprintf(fp, "------ ------------    ------------- -----------"
            "-- -------------\n");
         mn = (what == 1 ? lpx_get_num_rows(lp) : lpx_get_num_cols(lp));
         for (ij = 1; ij <= mn; ij++)
         {  const char *name;
            int kind, typx;
            double lb, ub, vx;
            if (what == 1)
            {  name = lpx_get_row_name(lp, ij);
               if (name == NULL) name = "";
               kind = LPX_CV;
               lpx_get_row_bnds(lp, ij, &typx, &lb, &ub);
               round = lpx_get_int_parm(lp, LPX_K_ROUND);
               lpx_set_int_parm(lp, LPX_K_ROUND, 1);
               vx = lpx_mip_row_val(lp, ij);
               lpx_set_int_parm(lp, LPX_K_ROUND, round);
            }
            else
            {  name = lpx_get_col_name(lp, ij);
               if (name == NULL) name = "";
               kind = lpx_get_col_kind(lp, ij);
               lpx_get_col_bnds(lp, ij, &typx, &lb, &ub);
               round = lpx_get_int_parm(lp, LPX_K_ROUND);
               lpx_set_int_parm(lp, LPX_K_ROUND, 1);
               vx = lpx_mip_col_val(lp, ij);
               lpx_set_int_parm(lp, LPX_K_ROUND, round);
            }
            /* row/column ordinal number */
            xfprintf(fp, "%6d ", ij);
            /* row column/name */
            if (strlen(name) <= 12)
               xfprintf(fp, "%-12s ", name);
            else
               xfprintf(fp, "%s\n%20s", name, "");
            /* row/column kind */
            xfprintf(fp, "%s  ",
               kind == LPX_CV ? " " : kind == LPX_IV ? "*" : "?");
            /* row/column primal activity */
            xfprintf(fp, "%13.6g", vx);
            /* row/column lower and upper bounds */
            switch (typx)
            {  case LPX_FR:
                  break;
               case LPX_LO:
                  xfprintf(fp, " %13.6g", lb);
                  break;
               case LPX_UP:
                  xfprintf(fp, " %13s %13.6g", "", ub);
                  break;
               case LPX_DB:
                  xfprintf(fp, " %13.6g %13.6g", lb, ub);
                  break;
               case LPX_FX:
                  xfprintf(fp, " %13.6g %13s", lb, "=");
                  break;
               default:
                  xassert(typx != typx);
            }
            /* end of line */
            xfprintf(fp, "\n");
         }
      }
      xfprintf(fp, "\n");
#if 1
      if (lpx_mip_status(lp) != LPX_I_UNDEF)
      {  int m = lpx_get_num_rows(lp);
         LPXKKT kkt;
         xfprintf(fp, "Integer feasibility conditions:\n\n");
         lpx_check_int(lp, &kkt);
         xfprintf(fp, "INT.PE: max.abs.err. = %.2e on row %d\n",
            kkt.pe_ae_max, kkt.pe_ae_row);
         xfprintf(fp, "        max.rel.err. = %.2e on row %d\n",
            kkt.pe_re_max, kkt.pe_re_row);
         switch (kkt.pe_quality)
         {  case 'H':
               xfprintf(fp, "        High quality\n");
               break;
            case 'M':
               xfprintf(fp, "        Medium quality\n");
               break;
            case 'L':
               xfprintf(fp, "        Low quality\n");
               break;
            default:
               xfprintf(fp, "        SOLUTION IS WRONG\n");
               break;
         }
         xfprintf(fp, "\n");
         xfprintf(fp, "INT.PB: max.abs.err. = %.2e on %s %d\n",
            kkt.pb_ae_max, kkt.pb_ae_ind <= m ? "row" : "column",
            kkt.pb_ae_ind <= m ? kkt.pb_ae_ind : kkt.pb_ae_ind - m);
         xfprintf(fp, "        max.rel.err. = %.2e on %s %d\n",
            kkt.pb_re_max, kkt.pb_re_ind <= m ? "row" : "column",
            kkt.pb_re_ind <= m ? kkt.pb_re_ind : kkt.pb_re_ind - m);
         switch (kkt.pb_quality)
         {  case 'H':
               xfprintf(fp, "        High quality\n");
               break;
            case 'M':
               xfprintf(fp, "        Medium quality\n");
               break;
            case 'L':
               xfprintf(fp, "        Low quality\n");
               break;
            default:
               xfprintf(fp, "        SOLUTION IS INFEASIBLE\n");
               break;
         }
         xfprintf(fp, "\n");
      }
#endif
      xfprintf(fp, "End of output\n");
      xfflush(fp);
      if (xferror(fp))
      {  xprintf("lpx_print_mip: can't write to `%s' - %s\n", fname,
            strerror(errno));
         goto fail;
      }
      xfclose(fp);
      return 0;
fail: if (fp != NULL) xfclose(fp);
      return 1;
}

/* eof */
