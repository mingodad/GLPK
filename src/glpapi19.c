/* glpapi19.c (flow network problems) */

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

/***********************************************************************
*  NAME
*
*  glp_mincost_lp - convert minimum cost flow problem to LP
*
*  SYNOPSIS
*
*  void glp_mincost_lp(glp_prob *lp, glp_graph *G, int names,
*     int v_rhs, int a_low, int a_cap, int a_cost);
*
*  DESCRIPTION
*
*  The routine glp_mincost_lp builds an LP problem, which corresponds
*  to the minimum cost flow problem on the specified network G. */

void glp_mincost_lp(glp_prob *lp, glp_graph *G, int names, int v_rhs,
      int a_low, int a_cap, int a_cost)
{     glp_vertex *v;
      glp_arc *a;
      int i, j, type, ind[1+2];
      double rhs, low, cap, cost, val[1+2];
      if (!(names == GLP_ON || names == GLP_OFF))
         xerror("glp_mincost_lp: names = %d; invalid parameter\n",
            names);
      if (v_rhs >= 0 && v_rhs > G->v_size - (int)sizeof(double))
         xerror("glp_mincost_lp: v_rhs = %d; invalid offset\n", v_rhs);
      if (a_low >= 0 && a_low > G->a_size - (int)sizeof(double))
         xerror("glp_mincost_lp: a_low = %d; invalid offset\n", a_low);
      if (a_cap >= 0 && a_cap > G->a_size - (int)sizeof(double))
         xerror("glp_mincost_lp: a_cap = %d; invalid offset\n", a_cap);
      if (a_cost >= 0 && a_cost > G->a_size - (int)sizeof(double))
         xerror("glp_mincost_lp: a_cost = %d; invalid offset\n", a_cost)
            ;
      glp_erase_prob(lp);
      if (names) glp_set_prob_name(lp, G->name);
      if (G->nv > 0) glp_add_rows(lp, G->nv);
      for (i = 1; i <= G->nv; i++)
      {  v = G->v[i];
         if (names) glp_set_row_name(lp, i, v->name);
         if (v_rhs >= 0)
            memcpy(&rhs, (char *)v->data + v_rhs, sizeof(double));
         else
            rhs = 0.0;
         glp_set_row_bnds(lp, i, GLP_FX, rhs, rhs);
      }
      if (G->na > 0) glp_add_cols(lp, G->na);
      for (i = 1, j = 0; i <= G->nv; i++)
      {  v = G->v[i];
         for (a = v->out; a != NULL; a = a->t_next)
         {  j++;
            if (names)
            {  char name[50+1];
               sprintf(name, "x[%d,%d]", a->tail->i, a->head->i);
               xassert(strlen(name) < sizeof(name));
               glp_set_col_name(lp, j, name);
            }
            if (a->tail->i != a->head->i)
            {  ind[1] = a->tail->i, val[1] = +1.0;
               ind[2] = a->head->i, val[2] = -1.0;
               glp_set_mat_col(lp, j, 2, ind, val);
            }
            if (a_low >= 0)
               memcpy(&low, (char *)a->data + a_low, sizeof(double));
            else
               low = 0.0;
            if (a_cap >= 0)
               memcpy(&cap, (char *)a->data + a_cap, sizeof(double));
            else
               cap = DBL_MAX;
            if (cap == DBL_MAX)
               type = GLP_LO;
            else if (low != cap)
               type = GLP_DB;
            else
               type = GLP_FX;
            glp_set_col_bnds(lp, j, type, low, cap);
            if (a_cost >= 0)
               memcpy(&cost, (char *)a->data + a_cost, sizeof(double));
            else
               cost = 0.0;
            glp_set_obj_coef(lp, j, cost);
         }
      }
      xassert(j == G->na);
      return;
}

/***********************************************************************
*  NAME
*
*  glp_maxflow_lp - convert maximum flow problem to LP
*
*  SYNOPSIS
*
*  void glp_maxflow_lp(glp_prob *lp, glp_graph *G, int names, int s,
*     int t, int a_cap);
*
*  DESCRIPTION
*
*  The routine glp_maxflow_lp builds an LP problem, which corresponds
*  to the maximum flow problem on the specified network G. */

void glp_maxflow_lp(glp_prob *lp, glp_graph *G, int names, int s,
      int t, int a_cap)
{     glp_vertex *v;
      glp_arc *a;
      int i, j, type, ind[1+2];
      double cap, val[1+2];
      if (!(names == GLP_ON || names == GLP_OFF))
         xerror("glp_maxflow_lp: names = %d; invalid parameter\n",
            names);
      if (!(1 <= s && s <= G->nv))
         xerror("glp_maxflow_lp: s = %d; source node number out of rang"
            "e\n", s);
      if (!(1 <= t && t <= G->nv))
         xerror("glp_maxflow_lp: t = %d: sink node number out of range "
            "\n", t);
      if (s == t)
         xerror("glp_maxflow_lp: s = t = %d; source and sink nodes must"
            " be distinct\n", s);
      if (a_cap >= 0 && a_cap > G->a_size - (int)sizeof(double))
         xerror("glp_maxflow_lp: a_cap = %d; invalid offset\n", a_cap);
      glp_erase_prob(lp);
      if (names) glp_set_prob_name(lp, G->name);
      glp_set_obj_dir(lp, GLP_MAX);
      glp_add_rows(lp, G->nv);
      for (i = 1; i <= G->nv; i++)
      {  v = G->v[i];
         if (names) glp_set_row_name(lp, i, v->name);
         if (i == s)
            type = GLP_LO;
         else if (i == t)
            type = GLP_UP;
         else
            type = GLP_FX;
         glp_set_row_bnds(lp, i, type, 0.0, 0.0);
      }
      if (G->na > 0) glp_add_cols(lp, G->na);
      for (i = 1, j = 0; i <= G->nv; i++)
      {  v = G->v[i];
         for (a = v->out; a != NULL; a = a->t_next)
         {  j++;
            if (names)
            {  char name[50+1];
               sprintf(name, "x[%d,%d]", a->tail->i, a->head->i);
               xassert(strlen(name) < sizeof(name));
               glp_set_col_name(lp, j, name);
            }
            if (a->tail->i != a->head->i)
            {  ind[1] = a->tail->i, val[1] = +1.0;
               ind[2] = a->head->i, val[2] = -1.0;
               glp_set_mat_col(lp, j, 2, ind, val);
            }
            if (a_cap >= 0)
               memcpy(&cap, (char *)a->data + a_cap, sizeof(double));
            else
               cap = DBL_MAX;
            if (cap == DBL_MAX)
               type = GLP_LO;
            else if (cap != 0.0)
               type = GLP_DB;
            else
               type = GLP_FX;
            glp_set_col_bnds(lp, j, type, 0.0, cap);
            if (a->tail->i == s)
               glp_set_obj_coef(lp, j, +1.0);
            else if (a->head->i == s)
               glp_set_obj_coef(lp, j, -1.0);
         }
      }
      xassert(j == G->na);
      return;
}

/* eof */
