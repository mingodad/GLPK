/* glpnpp04.c (LP/MIP preprocessor) */

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

#include "glpnpp.h"

/***********************************************************************
*  NAME
*
*  npp_clean_prob - perform initial LP/MIP processing
*
*  SYNOPSIS
*
*  #include "glpnpp.h"
*  void npp_clean_prob(NPP *npp);
*
*  DESCRIPTION
*
*  The routine npp_clean_prob performs initial LP/MIP processing that
*  currently includes:
*
*  1) removing free rows;
*
*  2) replacing double-sided constraint rows with almost identical
*     bounds, by equality constraint rows;
*
*  3) removing fixed columns;
*
*  4) replacing double-bounded columns with almost identical bounds by
*     fixed columns and removing those columns;
*
*  5) initial processing constraint coefficients (not implemented);
*
*  6) initial processing objective coefficients (not implemented). */

void npp_clean_prob(NPP *npp)
{     /* perform initial LP/MIP processing */
      NPPROW *row, *next_row;
      NPPCOL *col, *next_col;
      int ret;
      xassert(npp == npp);
      /* process rows which originally are free */
      for (row = npp->r_head; row != NULL; row = next_row)
      {  next_row = row->next;
         if (row->lb == -DBL_MAX && row->ub == +DBL_MAX)
         {  /* process free row */
#ifdef GLP_DEBUG
            xprintf("1");
#endif
            npp_free_row(npp, row);
            /* row was deleted */
         }
      }
      /* process rows which originally are double-sided inequalities */
      for (row = npp->r_head; row != NULL; row = next_row)
      {  next_row = row->next;
         if (row->lb != -DBL_MAX && row->ub != +DBL_MAX &&
             row->lb < row->ub)
         {  ret = npp_make_equality(npp, row);
            if (ret == 0)
               ;
            else if (ret == 1)
            {  /* row was replaced by equality constraint */
#ifdef GLP_DEBUG
               xprintf("2");
#endif
            }
            else
               xassert(ret != ret);
         }
      }
      /* process columns which are originally fixed */
      for (col = npp->c_head; col != NULL; col = next_col)
      {  next_col = col->next;
         if (col->lb == col->ub)
         {  /* process fixed column */
#ifdef GLP_DEBUG
            xprintf("3");
#endif
            npp_fixed_col(npp, col);
            /* column was deleted */
         }
      }
      /* process columns which are originally double-bounded */
      for (col = npp->c_head; col != NULL; col = next_col)
      {  next_col = col->next;
         if (col->lb != -DBL_MAX && col->ub != +DBL_MAX &&
             col->lb < col->ub)
         {  ret = npp_make_fixed(npp, col);
            if (ret == 0)
               ;
            else if (ret == 1)
            {  /* column was replaced by fixed column; process it */
#ifdef GLP_DEBUG
               xprintf("4");
#endif
               npp_fixed_col(npp, col);
               /* column was deleted */
            }
         }
      }
      return;
}

/***********************************************************************
*  NAME
*
*  npp_process_row - perform basic row processing
*
*  SYNOPSIS
*
*  #include "glpnpp.h"
*  int npp_process_row(NPP *npp, NPPROW *row)
*
*  DESCRIPTION
*
*  The routine npp_process_row performs basic row processing that
*  currently includes:
*
*  1) removing empty row;
*
*  2) removing equality constraint row singleton and corresponding
*     column;
*
*  3) removing inequality constraint row singleton and corresponding
*     column if it was fixed;
*
*  4) performing general row analysis;
*
*  5) removing redundant row bounds;
*
*  6) removing forcing row and corresponding columns;
*
*  7) removing row which becomes free due to redundant bounds;
*
*  8) computing implied bounds for all columns in the row and using
*     them to strengthen current column bounds (MIP only).
*
*  Additionally the routine may activate affected rows and/or columns
*  for further processing.
*
*  RETURNS
*
*  0           success;
*
*  GLP_ENOPFS  primal/integer infeasibility detected;
*
*  GLP_ENODFS  dual infeasibility detected. */

int npp_process_row(NPP *npp, NPPROW *row)
{     /* perform basic row processing */
      NPPCOL *col;
      NPPAIJ *aij, *next_aij, *aaa;
      int kase, ret;
      /* row must not be free */
      xassert(!(row->lb == -DBL_MAX && row->ub == +DBL_MAX));
      /* start processing row */
      if (row->ptr == NULL)
      {  /* empty row */
         ret = npp_empty_row(npp, row);
         if (ret == 0)
         {  /* row was deleted */
#ifdef GLP_DEBUG
            xprintf("A");
#endif
            return 0;
         }
         else if (ret == 1)
         {  /* primal infeasibility */
            return GLP_ENOPFS;
         }
         else
            xassert(ret != ret);
      }
      if (row->ptr->r_next == NULL)
      {  /* row singleton */
         col = row->ptr->col;
         if (row->lb == row->ub)
         {  /* equality constraint */
            ret = npp_eq_singlet(npp, row);
            if (ret == 0)
            {  /* column was fixed, row was deleted */
#ifdef GLP_DEBUG
               xprintf("B");
#endif
               /* activate rows affected by column */
               for (aij = col->ptr; aij != NULL; aij = aij->c_next)
                  npp_activate_row(npp, aij->row);
               /* process fixed column */
               npp_fixed_col(npp, col);
               /* column was deleted */
               return 0;
            }
            else if (ret == 1 || ret == 2)
            {  /* primal/integer infeasibility */
               return GLP_ENOPFS;
            }
            else
               xassert(ret != ret);
         }
         else
         {  /* inequality constraint */
            ret = npp_ineq_singlet(npp, row);
            if (0 <= ret && ret <= 3)
            {  /* row was deleted */
#ifdef GLP_DEBUG
               xprintf("C");
#endif
               /* activate column, since its length was changed due to
                  row deletion */
               npp_activate_col(npp, col);
               if (ret >= 2)
               {  /* column bounds changed significantly or column was
                     fixed */
                  /* activate rows affected by column */
                  for (aij = col->ptr; aij != NULL; aij = aij->c_next)
                     npp_activate_row(npp, aij->row);
               }
               if (ret == 3)
               {  /* column was fixed; process it */
#ifdef GLP_DEBUG
                  xprintf("D");
#endif
                  npp_fixed_col(npp, col);
                  /* column was deleted */
               }
               return 0;
            }
            else if (ret == 4)
            {  /* primal infeasibility */
               return GLP_ENOPFS;
            }
            else
               xassert(ret != ret);
         }
      }
      /* general row analysis */
      ret = npp_analyze_row(npp, row);
      xassert(0x00 <= ret && ret <= 0xFF);
      if (ret == 0x33)
      {  /* row bounds are inconsistent with column bounds */
         return GLP_ENOPFS;
      }
      if ((ret & 0x0F) == 0x00)
      {  /* row lower bound does not exist or redundant */
         if (row->lb != -DBL_MAX)
         {  /* remove redundant row lower bound */
#ifdef GLP_DEBUG
            xprintf("E");
#endif
            npp_inactive_bound(npp, row, 0);
         }
      }
      else if ((ret & 0x0F) == 0x01)
      {  /* row lower bound can be active */
         /* see below */
      }
      else if ((ret & 0x0F) == 0x02)
      {  /* row lower bound is a forcing bound */
#ifdef GLP_DEBUG
         xprintf("F");
#endif
         /* process forcing row */
         if (npp_forcing_row(npp, row, 0) == 0)
fixup:   {  /* columns were fixed, row was made free */
            for (aij = row->ptr; aij != NULL; aij = next_aij)
            {  /* process column fixed by forcing row */
#ifdef GLP_DEBUG
               xprintf("G");
#endif
               col = aij->col;
               next_aij = aij->r_next;
               /* activate rows affected by column */
               for (aaa = col->ptr; aaa != NULL; aaa = aaa->c_next)
                  npp_activate_row(npp, aaa->row);
               /* process fixed column */
               npp_fixed_col(npp, col);
               /* column was deleted */
            }
            /* process free row (which now is empty due to deletion of
               all its columns) */
            npp_free_row(npp, row);
            /* row was deleted */
            return 0;
         }
      }
      else
         xassert(ret != ret);
      if ((ret & 0xF0) == 0x00)
      {  /* row upper bound does not exist or redundant */
         if (row->ub != +DBL_MAX)
         {  /* remove redundant row upper bound */
#ifdef GLP_DEBUG
            xprintf("H");
#endif
            npp_inactive_bound(npp, row, 1);
         }
      }
      else if ((ret & 0xF0) == 0x10)
      {  /* row upper bound can be active */
         /* see below */
      }
      else if ((ret & 0xF0) == 0x20)
      {  /* row upper bound is a forcing bound */
#ifdef GLP_DEBUG
         xprintf("I");
#endif
         /* process forcing row */
         if (npp_forcing_row(npp, row, 1) == 0) goto fixup;
      }
      else
         xassert(ret != ret);
      if (row->lb == -DBL_MAX && row->ub == +DBL_MAX)
      {  /* row became free due to redundant bounds removal */
#ifdef GLP_DEBUG
         xprintf("J");
#endif
         /* activate its columns, since their length will change due
            to row deletion */
         for (aij = row->ptr; aij != NULL; aij = aij->r_next)
            npp_activate_col(npp, aij->col);
         /* process free row */
         npp_free_row(npp, row);
         /* row was deleted */
         return 0;
      }
      /* MIP only */
      if (npp->sol == GLP_MIP)
      {  /* determine implied column bounds */
         npp_implied_bounds(npp, row);
         /* and use them to strengthen current column bounds */
         for (aij = row->ptr; aij != NULL; aij = next_aij)
         {  col = aij->col;
            next_aij = aij->r_next;
            for (kase = 0; kase <= 1; kase++)
            {  if (kase == 0)
               {  /* process implied column lower bound */
                  if (col->ll == -DBL_MAX) continue;
                  ret = npp_implied_lower(npp, col, col->ll);
               }
               else
               {  /* process implied column upper bound */
                  if (col->uu == +DBL_MAX) continue;
                  ret = npp_implied_upper(npp, col, col->uu);
               }
               if (ret == 0 || ret == 1)
               {  /* current column bounds did not change or changed,
                     but not significantly */
               }
               else if (ret == 2 || ret == 3)
               {  /* current column bounds changed significantly, or
                     column was fixed */
                  /* activate rows affected by column */
                  for (aaa = col->ptr; aaa != NULL; aaa = aaa->c_next)
                     npp_activate_row(npp, aaa->row);
                  if (ret == 3)
                  {  /* process fixed column */
#ifdef GLP_DEBUG
                     xprintf("K");
#endif
                     npp_fixed_col(npp, col);
                     /* column was deleted */
                     break /* for kase */;
                  }
               }
               else if (ret == 4)
               {  /* primal/integer infeasibility */
                  return GLP_ENOPFS;
               }
               else
                  xassert(ret != ret);
            }
         }
      }
      return 0;
}

/***********************************************************************
*  NAME
*
*  npp_process_col - perform basic column processing
*
*  SYNOPSIS
*
*  #include "glpnpp.h"
*  int npp_process_col(NPP *npp, NPPCOL *col);
*
*  DESCRIPTION
*
*  The routine npp_process_col performs basic column processing that
*  currently includes:
*
*  1) fixing and removing empty column;
*
*  2) removing column singleton, which is implied slack variable, and
*     corresponding row if it becomes free;
*
*  3) removing bounds of column, which is implied free variable, and
*     replacing corresponding row by equality constraint.
*
*  Additionally the routine may activate affected rows and/or columns
*  for further processing.
*
*  RETURNS
*
*  0           success;
*
*  GLP_ENOPFS  primal/integer infeasibility detected;
*
*  GLP_ENODFS  dual infeasibility detected. */

int npp_process_col(NPP *npp, NPPCOL *col)
{     /* perform basic column processing */
      NPPROW *row;
      NPPAIJ *aij;
      int ret;
      /* column must not be fixed */
      xassert(col->lb < col->ub);
      /* start processing column */
      if (col->ptr == NULL)
      {  /* empty column */
         ret = npp_empty_col(npp, col);
         if (ret == 0)
         {  /* column was fixed and deleted */
#ifdef GLP_DEBUG
            xprintf("a");
#endif
            return 0;
         }
         else if (ret == 1)
         {  /* dual infeasibility */
            return GLP_ENODFS;
         }
         else
            xassert(ret != ret);
      }
      if (col->ptr->c_next == NULL)
      {  /* column singleton */
         row = col->ptr->row;
         if (row->lb == row->ub)
         {  /* equality constraint */
            if (!col->is_int)
slack:      {  /* implied slack variable */
#ifdef GLP_DEBUG
               xprintf("b");
#endif
               npp_implied_slack(npp, col);
               /* column was deleted */
               if (row->lb == -DBL_MAX && row->ub == +DBL_MAX)
               {  /* row became free due to implied slack variable */
#ifdef GLP_DEBUG
                  xprintf("c");
#endif
                  /* activate columns affected by row */
                  for (aij = row->ptr; aij != NULL; aij = aij->r_next)
                     npp_activate_col(npp, aij->col);
                  /* process free row */
                  npp_free_row(npp, row);
                  /* row was deleted */
               }
               else
               {  /* row became inequality constraint; activate it
                     since its length changed due to column deletion */
                  npp_activate_row(npp, row);
               }
               return 0;
            }
         }
         else
         {  /* inequality constraint */
            if (!col->is_int)
            {  ret = npp_implied_free(npp, col);
               if (ret == 0)
               {  /* implied free variable */
#ifdef GLP_DEBUG
                  xprintf("d");
#endif
                  /* column bounds were removed, row was replaced by
                     equality constraint */
                  goto slack;
               }
               else if (ret == 1)
               {  /* column is not implied free variable, because its
                     lower and/or upper bounds can be active */
               }
               else if (ret == 2)
               {  /* dual infeasibility */
                  return GLP_ENODFS;
               }
            }
         }
      }
      /* column still exists */
      return 0;
}

/***********************************************************************
*  NAME
*
*  npp_process_prob - perform basic LP/MIP processing
*
*  SYNOPSIS
*
*  #include "glpnpp.h"
*  int npp_process_prob(NPP *npp);
*
*  DESCRIPTION
*
*  The routine npp_process_prob performs basic LP/MIP processing that
*  currently includes:
*
*  1) initial LP/MIP processing (see the routine npp_clean_prob),
*
*  2) basic row processing (see the routine npp_process_row), and
*
*  3) basic column processing (see the routine npp_process_col).
*
*  The routine uses two sets: the set of active rows and the set of
*  active columns. Rows/columns are marked by a flag (the field temp in
*  NPPROW/NPPCOL). If the flag is non-zero, the row/column is active,
*  in which case it is placed in the beginning of the row/column list;
*  otherwise, if the flag is zero, the row/column is inactive, in which
*  case it is placed in the end of the row/column list. If a row/column
*  being currently processed may affect other rows/columns, the latters
*  are activated for further processing.
*
*  RETURNS
*
*  0           success;
*
*  GLP_ENOPFS  primal/integer infeasibility detected;
*
*  GLP_ENODFS  dual infeasibility detected. */

int npp_process_prob(NPP *npp)
{     /* perform basic LP/MIP processing */
      NPPROW *row;
      NPPCOL *col;
      int processing, ret;
#ifdef GLP_DEBUG
      xprintf("Performing basic LP/MIP processing...\n");
#endif
      /* perform initial LP/MIP processing */
      npp_clean_prob(npp);
      /* activate all remaining rows and columns */
      for (row = npp->r_head; row != NULL; row = row->next)
         row->temp = 1;
      for (col = npp->c_head; col != NULL; col = col->next)
         col->temp = 1;
      /* main processing loop */
      processing = 1;
      while (processing)
      {  processing = 0;
         /* process all active rows */
         for (;;)
         {  row = npp->r_head;
            if (row == NULL || !row->temp) break;
            npp_deactivate_row(npp, row);
            ret = npp_process_row(npp, row);
            if (ret != 0) goto done;
            processing = 1;
         }
         /* process all active columns */
         for (;;)
         {  col = npp->c_head;
            if (col == NULL || !col->temp) break;
            npp_deactivate_col(npp, col);
            ret = npp_process_col(npp, col);
            if (ret != 0) goto done;
            processing = 1;
         }
      }
      /* all seems ok */
      ret = 0;
done: xassert(ret == 0 || ret == GLP_ENOPFS || ret == GLP_ENODFS);
#ifdef GLP_DEBUG
      xprintf("\n");
#endif
      return ret;
}

/**********************************************************************/

int npp_simplex(NPP *npp)
{     /* process LP prior to applying primal/dual simplex method */
      int ret;
      xassert(npp->sol == GLP_SOL);
      ret = npp_process_prob(npp);
      return ret;
}

/* eof */
