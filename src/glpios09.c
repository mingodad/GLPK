/* glpios09.c */

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

#include "glpios.h"

struct csa
{     /* common storage area */
      int *dn_cnt; /* int dn_cnt[1+n]; */
      /* dn_cnt[j] is the number of subproblems, whose LP relaxations
         have been solved and which are down-branches for variable x[j];
         dn_cnt[j] = 0 means the down pseudocost is uninitialized */
      double *dn_sum; /* double dn_sum[1+n]; */
      /* dn_sum[j] is the sum of per unit degradations of the objective
         over all dn_cnt[j] subproblems */
      int *up_cnt; /* int up_cnt[1+n]; */
      /* up_cnt[j] is the number of subproblems, whose LP relaxations
         have been solved and which are up-branches for variable x[j];
         up_cnt[j] = 0 means the up pseudocost is uninitialized */
      double *up_sum; /* double up_sum[1+n]; */
      /* up_sum[j] is the sum of per unit degradations of the objective
         over all up_cnt[j] subproblems */
};

void *ios_pcost_init(glp_tree *tree)
{     /* initialize working data used on pseudocost branching */
      struct csa *csa;
      int n = tree->n, j;
      csa = xmalloc(sizeof(struct csa));
      csa->dn_cnt = xcalloc(1+n, sizeof(int));
      csa->dn_sum = xcalloc(1+n, sizeof(double));
      csa->up_cnt = xcalloc(1+n, sizeof(int));
      csa->up_sum = xcalloc(1+n, sizeof(double));
      for (j = 1; j <= n; j++)
      {  csa->dn_cnt[j] = csa->up_cnt[j] = 0;
         csa->dn_sum[j] = csa->up_sum[j] = 0.0;
      }
      return csa;
}

static double eval_degrad(glp_prob *P, int j, double bnd)
{     /* compute degradation of the objective on fixing x[j] at given
         value with a limited number of dual simplex iterations */
      /* this routine fixes column x[j] at specified value bnd,
         solves resulting LP, and returns a lower bound to degradation
         of the objective, degrad >= 0 */
      glp_prob *lp;
      glp_smcp parm;
      int ret;
      double degrad;
      /* the current basis must be optimal */
      xassert(glp_get_status(P) == GLP_OPT);
      /* create a copy of P */
      lp = glp_create_prob();
      glp_copy_prob(lp, P, 0);
      /* fix column x[j] at specified value */
      glp_set_col_bnds(lp, j, GLP_FX, bnd, bnd);
      /* try to solve resulting LP */
      glp_init_smcp(&parm);
      parm.msg_lev = GLP_MSG_OFF;
      parm.meth = GLP_DUAL;
      parm.it_lim = 30;
      parm.out_dly = 1000;
      parm.meth = GLP_DUAL;
      ret = glp_simplex(lp, &parm);
      if (ret == 0 || ret == GLP_EITLIM)
      {  if (glp_get_prim_stat(lp) == GLP_NOFEAS)
         {  /* resulting LP has no primal feasible solution */
            degrad = DBL_MAX;
         }
         else if (glp_get_dual_stat(lp) == GLP_FEAS)
         {  /* resulting basis is optimal or at least dual feasible,
               so we have the correct lower bound to degradation */
            if (P->dir == GLP_MIN)
               degrad = lp->obj_val - P->obj_val;
            else if (P->dir == GLP_MAX)
               degrad = P->obj_val - lp->obj_val;
            else
               xassert(P != P);
            /* degradation cannot be negative by definition */
            /* note that the lower bound to degradation may be close
               to zero even if its exact value is zero due to round-off
               errors on computing the objective value */
            if (degrad < 1e-6 * (1.0 + 0.001 * fabs(P->obj_val)))
               degrad = 0.0;
         }
         else
         {  /* the final basis reported by the simplex solver is dual
               infeasible, so we cannot determine a non-trivial lower
               bound to degradation */
            degrad = 0.0;
         }
      }
      else
      {  /* the simplex solver failed */
         degrad = 0.0;
      }
      /* delete the copy of P */
      glp_delete_prob(lp);
      return degrad;
}

void ios_pcost_branch(glp_tree *tree);

void ios_pcost_update(glp_tree *tree)
{     /* update history information for pseudocost branching */
      /* this routine is called every time when LP relaxation of the
         current subproblem has been solved to optimality with all lazy
         and cutting plane constraints included */
      int j;
      double dx, dz, psi;
      struct csa *csa = tree->pcost;
      xassert(csa != NULL);
      xassert(tree->curr != NULL);
      /* if the current subproblem is the root, skip updating */
      if (tree->curr->up == NULL) goto skip;
      /* determine branching variable x[j], which was used in the
         parent subproblem to create the current subproblem */
      j = tree->curr->up->br_var;
      xassert(1 <= j && j <= tree->n);
      /* determine the change dx[j] = new x[j] - old x[j],
         where new x[j] is a value of x[j] in optimal solution to LP
         relaxation of the current subproblem, old x[j] is a value of
         x[j] in optimal solution to LP relaxation of the parent
         subproblem */
      dx = tree->mip->col[j]->prim - tree->curr->up->br_val;
      xassert(dx != 0.0);
      /* determine corresponding change dz = new dz - old dz in the
         objective function value */
      dz = tree->mip->obj_val - tree->curr->up->lp_obj;
      /* determine per unit degradation of the objective function */
      psi = fabs(dz / dx);
      /* update history information */
      if (dx < 0.0)
      {  /* the current subproblem is down-branch */
         csa->dn_cnt[j]++;
         csa->dn_sum[j] += psi;
      }
      else /* dx > 0.0 */
      {  /* the current subproblem is up-branch */
         csa->up_cnt[j]++;
         csa->up_sum[j] += psi;
      }
skip: return;
}

void ios_pcost_free(glp_tree *tree)
{     /* free working area used on pseudocost branching */
      struct csa *csa = tree->pcost;
      xassert(csa != NULL);
      xfree(csa->dn_cnt);
      xfree(csa->dn_sum);
      xfree(csa->up_cnt);
      xfree(csa->up_sum);
      xfree(csa);
      tree->pcost = NULL;
      return;
}

static double eval_psi(glp_tree *T, int j, int brnch)
{     /* compute estimation of pseudocost of variable x[j] for down-
         or up-branch */
      struct csa *csa = T->pcost;
      double beta, degrad, psi;
      xassert(csa != NULL);
      xassert(1 <= j && j <= T->n);
      if (brnch == GLP_DN_BRNCH)
      {  /* down-branch */
         if (csa->dn_cnt[j] == 0)
         {  /* initialize down pseudocost */
            beta = T->mip->col[j]->prim;
            degrad = eval_degrad(T->mip, j, floor(beta));
            if (degrad == DBL_MAX)
            {  psi = DBL_MAX;
               goto done;
            }
            csa->dn_cnt[j] = 1;
            csa->dn_sum[j] = degrad / (beta - floor(beta));
         }
         psi = csa->dn_sum[j] / (double)csa->dn_cnt[j];
      }
      else if (brnch == GLP_UP_BRNCH)
      {  /* up-branch */
         if (csa->up_cnt[j] == 0)
         {  /* initialize up pseudocost */
            beta = T->mip->col[j]->prim;
            degrad = eval_degrad(T->mip, j, ceil(beta));
            if (degrad == DBL_MAX)
            {  psi = DBL_MAX;
               goto done;
            }
            csa->up_cnt[j] = 1;
            csa->up_sum[j] = degrad / (ceil(beta) - beta);
         }
         psi = csa->up_sum[j] / (double)csa->up_cnt[j];
      }
      else
         xassert(brnch != brnch);
done: return psi;
}

static void progress(glp_tree *T)
{     /* display progress of pseudocost initialization */
      struct csa *csa = T->pcost;
      int j, nv = 0, ni = 0;
      for (j = 1; j <= T->n; j++)
      {  if (glp_ios_can_branch(T, j))
         {  nv++;
            if (csa->dn_cnt[j] > 0 && csa->up_cnt[j] > 0) ni++;
         }
      }
      xprintf("Pseudocosts initialized for %d of %d variables\n",
         ni, nv);
      return;
}

void ios_pcost_branch(glp_tree *T)
{     /* choose branching variable with pseudocost branching */
      xlong_t t = xtime();
      int j, jjj, sel;
      double beta, psi, d1, d2, d, dmax;
      /* initialize the working arrays */
      if (T->pcost == NULL)
         T->pcost = ios_pcost_init(T);
      /* nothing has been chosen so far */
      jjj = 0, dmax = -1.0;
      /* go through the list of branching candidates */
      for (j = 1; j <= T->n; j++)
      {  if (!glp_ios_can_branch(T, j)) continue;
         /* determine primal value of x[j] in optimal solution to LP
            relaxation of the current subproblem */
         beta = T->mip->col[j]->prim;
         /* estimate pseudocost of x[j] for down-branch */
         psi = eval_psi(T, j, GLP_DN_BRNCH);
         if (psi == DBL_MAX)
         {  /* down-branch has no primal feasible solution */
            jjj = j, sel = GLP_DN_BRNCH;
            goto brch;
         }
         /* estimate degradation of the objective for down-branch */
         d1 = psi * (beta - floor(beta));
         /* estimate pseudocost of x[j] for up-branch */
         psi = eval_psi(T, j, GLP_UP_BRNCH);
         if (psi == DBL_MAX)
         {  /* up-branch has no primal feasible solution */
            jjj = j, sel = GLP_UP_BRNCH;
            goto brch;
         }
         /* estimate degradation of the objective for up-branch */
         d2 = psi * (ceil(beta) - beta);
         /* determine d = max(d1, d2) */
         d = (d1 > d2 ? d1 : d2);
         /* choose x[j] which provides maximal estimated degradation of
            the objective either in down- or up-branch */
         if (dmax < d)
         {  dmax = d;
            jjj = j;
            /* continue the search from a subproblem, where degradation
               is less than in other one */
            sel = (d1 <= d2 ? GLP_DN_BRNCH : GLP_UP_BRNCH);
         }
         /* display progress of pseudocost initialization */
         if (T->parm->msg_lev >= GLP_ON)
         {  if (xdifftime(xtime(), t) >= 10.0)
            {  progress(T);
               t = xtime();
            }
         }
      }
      if (dmax == 0.0)
      {  /* no degradation is indicated; choose a variable having most
            fractional value */
         ios_mostf_branch(T);
      }
      else
brch:    glp_ios_branch_upon(T, jjj, sel);
      return;
}

/* eof */
