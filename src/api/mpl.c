/* mpl.c (processing model in GNU MathProg language) */

/***********************************************************************
*  This code is part of GLPK (GNU Linear Programming Kit).
*
*  Copyright (C) 2008-2016 Andrew Makhorin, Department for Applied
*  Informatics, Moscow Aviation Institute, Moscow, Russia. All rights
*  reserved. E-mail: <mao@gnu.org>.
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

#include "mpl.h"
#include "prob.h"

glp_tran *glp_mpl_alloc_wksp(void)
{     /* allocate the MathProg translator workspace */
      glp_tran *tran;
      tran = mpl_initialize();
      return tran;
}

void glp_mpl_init_rand(glp_tran *tran, int seed)
{     /* initialize pseudo-random number generator */
      if (tran->phase != GLP_TRAN_PHASE_INITIAL)
         xerror("glp_mpl_init_rand: invalid call sequence\n");
      rng_init_rand(tran->rand, seed);
      return;
}

int glp_mpl_read_model(glp_tran *tran, const char *fname, int skip)
{     /* read and translate model section */
      int ret;
      if (tran->phase != GLP_TRAN_PHASE_INITIAL)
         xerror("glp_mpl_read_model: invalid call sequence\n");
      ret = mpl_read_model(tran, (char *)fname, skip);
      if (ret == 1 || ret == 2)
         ret = 0;
      else if (ret == 4)
         ret = 1;
      else
         xassert(ret != ret);
      return ret;
}

int glp_mpl_read_data(glp_tran *tran, const char *fname)
{     /* read and translate data section */
      int ret;
      if (!(tran->phase == GLP_TRAN_PHASE_MODEL || tran->phase == GLP_TRAN_PHASE_DATA))
         xerror("glp_mpl_read_data: invalid call sequence\n");
      ret = mpl_read_data(tran, (char *)fname);
      if (ret == 2)
         ret = 0;
      else if (ret == 4)
         ret = 1;
      else
         xassert(ret != ret);
      return ret;
}

int glp_mpl_generate(glp_tran *tran, const char *fname)
{     /* generate the model */
      int ret;
      if (!(tran->phase == GLP_TRAN_PHASE_MODEL || tran->phase == GLP_TRAN_PHASE_DATA))
         xerror("glp_mpl_generate: invalid call sequence\n");
      ret = mpl_generate(tran, (char *)fname);
      if (ret == 3)
         ret = 0;
      else if (ret == 4)
         ret = 1;
      return ret;
}

void glp_mpl_set_solve_callback(glp_tran *tran, glp_solve_callback solve_cb, void *udata)
{
    tran->solve_callback = solve_cb;
    tran->solve_callback_udata = udata;
}

int glp_mpl_set_genall(glp_tran *tran, int bflag)
{
    int rc = tran->gen_all;
    tran->gen_all = bflag;
    return rc;
}

int glp_mpl_set_show_delta(glp_tran *tran, int bflag)
{
    int rc = tran->show_delta;
    tran->show_delta = bflag;
    return rc;
}

int glp_mpl_set_add_missing_param_values(glp_tran *tran, int bflag)
{
    int rc = tran->add_missing_param_values;
    tran->add_missing_param_values = bflag;
    return rc;
}

int glp_mpl_set_msg_lev(glp_tran *tran, int msg_lev)
{
    int rc = tran->msg_lev;
    tran->msg_lev = msg_lev;
    return rc;
}

void glp_mpl_build_prob(glp_tran *tran, glp_prob *prob)
{     /* build LP/MIP problem instance from the model */
      int m, n, i, j, t, kind, type, len, *ind;
      glp_double lb, ub, *val;
      if (tran->phase != GLP_TRAN_PHASE_GENERATE)
         xerror("glp_mpl_build_prob: invalid call sequence\n");
      /* erase the problem object */
      i = glp_get_use_col_row_names(prob);
      /* save use_col_row_names */
      glp_erase_prob(prob);
      glp_set_use_col_row_names(prob, i);
      /* restore use_col_row_names */
      /* set problem name */
      glp_set_prob_name(prob, mpl_get_prob_name(tran));
      /* build rows (constraints) */
      m = mpl_get_num_rows(tran);
      if (m > 0)
         glp_add_rows(prob, m);
      for (i = 1; i <= m; i++)
      {  /* set row name */
         if(prob->use_col_row_names)
             glp_set_row_name(prob, i, mpl_get_row_name(tran, i));
         /* set row bounds */
         type = mpl_get_row_bnds_gd(tran, i, &lb, &ub);
         switch (type)
         {  case MPL_FR: type = GLP_FR; break;
            case MPL_LO: type = GLP_LO; break;
            case MPL_UP: type = GLP_UP; break;
            case MPL_DB: type = GLP_DB; break;
            case MPL_FX: type = GLP_FX; break;
            default: xassert(type != type);
         }
         if (type == GLP_DB && fabs(lb - ub) < GLP_MPL_MIN_9 * (1.0 + fabs(lb)))
         {  type = GLP_FX;
            if (fabs(lb) <= fabs(ub)) ub = lb; else lb = ub;
         }
         glp_set_row_bnds(prob, i, type, lb, ub);
         /* warn about non-zero constant term */
         if (mpl_get_row_c0(tran, i) != 0.0)
            xprintf("glp_mpl_build_prob: row %s; constant term %.12g ig"
               "nored\n",
               mpl_get_row_name(tran, i), mpl_get_row_c0(tran, i));
      }
      /* build columns (variables) */
      n = mpl_get_num_cols(tran);
      if (n > 0)
         glp_add_cols(prob, n);
      for (j = 1; j <= n; j++)
      {  /* set column name */
         if(prob->use_col_row_names)
            glp_set_col_name(prob, j, mpl_get_col_name(tran, j));
         /* set column kind */
         kind = mpl_get_col_kind(tran, j);
         switch (kind)
         {  case MPL_NUM:
               break;
            case MPL_INT:
            case MPL_BIN:
               glp_set_col_kind(prob, j, GLP_IV);
               break;
            default:
               xassert(kind != kind);
         }
         /* set column bounds */
         type = mpl_get_col_bnds_gd(tran, j, &lb, &ub);
         switch (type)
         {  case MPL_FR: type = GLP_FR; break;
            case MPL_LO: type = GLP_LO; break;
            case MPL_UP: type = GLP_UP; break;
            case MPL_DB: type = GLP_DB; break;
            case MPL_FX: type = GLP_FX; break;
            default: xassert(type != type);
         }
         if (kind == MPL_BIN)
         {  if (type == GLP_FR || type == GLP_UP || lb < 0.0) lb = 0.0;
            if (type == GLP_FR || type == GLP_LO || ub > 1.0) ub = 1.0;
            type = GLP_DB;
         }
         if (type == GLP_DB && fabs(lb - ub) < GLP_MPL_MIN_9 * (1.0 + fabs(lb)))
         {  type = GLP_FX;
            if (fabs(lb) <= fabs(ub)) ub = lb; else lb = ub;
         }
         glp_set_col_bnds(prob, j, type, lb, ub);
      }
      /* load the constraint matrix */
      ind = xcalloc(1+n, sizeof(int));
      val = xcalloc(1+n, sizeof(glp_double));
      for (i = 1; i <= m; i++)
      {  len = mpl_get_mat_row_gd(tran, i, ind, val);
         glp_set_mat_row(prob, i, len, ind, val);
      }
      /* build objective function (the first objective is used) */
      for (i = 1; i <= m; i++)
      {  kind = mpl_get_row_kind(tran, i);
         if (kind == MPL_MIN || kind == MPL_MAX)
         {  /* set objective name */
            glp_set_obj_name(prob, mpl_get_row_name(tran, i));
            /* set optimization direction */
            glp_set_obj_dir(prob, kind == MPL_MIN ? GLP_MIN : GLP_MAX);
            /* set constant term */
            glp_set_obj_coef(prob, 0, mpl_get_row_c0(tran, i));
            /* set objective coefficients */
            len = mpl_get_mat_row_gd(tran, i, ind, val);
            for (t = 1; t <= len; t++)
               glp_set_obj_coef(prob, ind[t], val[t]);
            break;
         }
      }
      /* free working arrays */
      xfree(ind);
      xfree(val);
      return;
}

int glp_mpl_postsolve(glp_tran *tran, glp_prob *prob, int sol)
{     /* postsolve the model */
      int i, j, m, n, stat, ret, has_shift;
      glp_double prim, dual;
      if (!(tran->phase == GLP_TRAN_PHASE_GENERATE && !tran->flag_p))
         xerror("glp_mpl_postsolve: invalid call sequence\n");
      if (!(sol == GLP_SOL || sol == GLP_IPT || sol == GLP_MIP))
         xerror("glp_mpl_postsolve: sol = %d; invalid parameter\n",
            sol);
      m = mpl_get_num_rows(tran);
      n = mpl_get_num_cols(tran);
      if (!(m == glp_get_num_rows(prob) &&
            n == glp_get_num_cols(prob)))
         xerror("glp_mpl_postsolve: wrong problem object\n");
      if (!mpl_has_solve_stmt(tran))
      {  ret = 0;
         goto done;
      }
      has_shift = glp_get_obj_shift(prob) != 0.0;
      for (i = 1; i <= m; i++)
      {  if (sol == GLP_SOL)
         {  stat = glp_get_row_stat(prob, i);
            if(i == 1 && has_shift)
                prim = glp_get_obj_val(prob);
            else
                prim = glp_get_row_prim(prob, i);
            dual = glp_get_row_dual(prob, i);
         }
         else if (sol == GLP_IPT)
         {  stat = 0;
            if(i == 1 && has_shift)
                prim = glp_ipt_obj_val(prob);
            else
                prim = glp_ipt_row_prim(prob, i);
            dual = glp_ipt_row_dual(prob, i);
         }
         else if (sol == GLP_MIP)
         {  stat = 0;
            if(i == 1 && has_shift)
                prim = glp_mip_obj_val(prob);
            else
                prim = glp_mip_row_val(prob, i);
            dual = 0.0;
         }
         else
            xassert(sol != sol);
         if (fabs(prim) < GLP_MPL_MIN_9) prim = 0.0;
         if (fabs(dual) < GLP_MPL_MIN_9) dual = 0.0;
         mpl_put_row_soln(tran, i, stat, prim, dual);
      }
      for (j = 1; j <= n; j++)
      {  if (sol == GLP_SOL)
         {  stat = glp_get_col_stat(prob, j);
            prim = glp_get_col_prim(prob, j);
            dual = glp_get_col_dual(prob, j);
         }
         else if (sol == GLP_IPT)
         {  stat = 0;
            prim = glp_ipt_col_prim(prob, j);
            dual = glp_ipt_col_dual(prob, j);
         }
         else if (sol == GLP_MIP)
         {  stat = 0;
            prim = glp_mip_col_val(prob, j);
            dual = 0.0;
         }
         else
            xassert(sol != sol);
         if (fabs(prim) < GLP_MPL_MIN_9) prim = 0.0;
         if (fabs(dual) < GLP_MPL_MIN_9) dual = 0.0;
         mpl_put_col_soln(tran, j, stat, prim, dual);
      }
      ret = mpl_postsolve(tran);
      if (ret == 3)
         ret = 0;
      else if (ret == 4)
         ret = 1;
done: return ret;
}

int glp_mpl_waiting_solve(glp_tran *tran)
{
    return tran->stmt && tran->stmt->type == A_SOLVE;
}

void glp_mpl_free_wksp(glp_tran *tran)
{     /* free the MathProg translator workspace */
      mpl_terminate(tran);
      return;
}

#ifdef WITH_LPSOLVE
#include "lp_lib.h"
//#include "commonlib.h"

lprec *glp_lpsolve_mpl_build_prob(glp_tran *tran)
{
  int     ret, m, n, i, ii, j, *ndx, len, kind, type;
  double  lb, ub, *val, infinite;
  if (tran->phase != GLP_TRAN_PHASE_GENERATE)
     xerror("glp_lpsolve_mpl_build_prob: invalid call sequence\n");
  lprec *lp =  make_lp(0, 0);
  if(!lp) return lp;
  /* set problem name and temporary row storage order */
  lp->set_lp_name(lp, mpl_get_prob_name(tran));
  lp->set_add_rowmode(lp, TRUE);

  infinite = get_infinite(lp);
  /* build columns (variables) */
  n = mpl_get_num_cols(tran);
  for (j = 1; j <= n; j++) {

    /* set column name */
    lp->set_col_name(lp, j, mpl_get_col_name(tran, j));

    /* set column kind */
    kind = mpl_get_col_kind(tran, j);
    switch (kind) {
      case MPL_NUM: break;
      case MPL_INT:
      case MPL_BIN: lp->set_int(lp, j, TRUE);
                    break;
      default:      xassert(kind != kind);
    }

    /* set column bounds */
    type = mpl_get_col_bnds(tran, j, &lb, &ub);
    switch (type) {
      case MPL_FR: break;
      case MPL_LO: break;
      case MPL_UP: break;
      case MPL_DB: break;
      case MPL_FX: break;
      default:     xassert(type != type);
    }
    if (kind == MPL_BIN) {
      if (type == MPL_FR || type == MPL_UP || lb < 0.0) lb = 0.0;
      if (type == MPL_FR || type == MPL_LO || ub > 1.0) ub = 1.0;
      type = MPL_DB;
    }
    if (type == MPL_DB && fabs(lb - ub) < GLP_MPL_MIN_9 * (1.0 + fabs(lb))) {
      type = MPL_FX;
      if (fabs(lb) <= fabs(ub)) ub = lb; else lb = ub;
    }

    //if(ub > infinite) ub = infinite;
    //if(lb < -infinite) lb = -infinite;
    if(type == MPL_FR)
      lp->set_unbounded(lp, j);
    else if(type == MPL_UP) {
      lp->set_unbounded(lp, j);
      lp->set_upbo(lp, j, ub);
    }
    else if(type == MPL_LO)
      lp->set_lowbo(lp, j, lb);
    else {
      lp->set_upbo(lp, j, ub);
      lp->set_lowbo(lp, j, lb);
    }
  }

  /* allocate working arrays */
  ndx = (int *) glp_malloc((1+n) * sizeof(*ndx));
  val = (double *) glp_malloc((1+n) * sizeof(*val));

  /* build objective function (the first objective is used) */
  m = mpl_get_num_rows(tran);
  for (i = 1; i <= m; i++) {

    kind = mpl_get_row_kind(tran, i);
    if (kind == MPL_MIN || kind == MPL_MAX) {

      /* set optimization direction */
      lp->set_sense(lp, (MYBOOL) (kind == MPL_MAX) );

      /* set objective coefficients */
      len = mpl_get_mat_row(tran, i, ndx, val);
      lp->set_obj_fnex(lp, len, val+1, ndx+1);

      /* warn about non-zero constant term */
      if (mpl_get_row_c0(tran, i) != 0.0)
         xprintf("glp_lpsolve_mpl_build_prob: row %s; constant term %.12g ig"
               "nored\n",
               mpl_get_row_name(tran, i), mpl_get_row_c0(tran, i));
      /* set constant term */
      lp->set_rh(lp, 0, mpl_get_row_c0(tran, i));

      /* set objective name */
      lp->set_row_name(lp, 0, mpl_get_row_name(tran, i));
      break;
    }
  }

  /* build rows (constraints) */
  ii = 0;
  for (i = 1; i <= m; i++) {

    /* select constraint rows only */
    kind = mpl_get_row_kind(tran, i);
    if (kind == MPL_MIN || kind == MPL_MAX)
      continue;

    /* set row bounds */
    type = mpl_get_row_bnds(tran, i, &lb, &ub);
    switch (type) {
      case MPL_FR: break;
      case MPL_LO: break;
      case MPL_UP: break;
      case MPL_DB: break;
      case MPL_FX: break;
      default:     xassert(type != type);
    }
    if(type == MPL_FR)
      continue;
    if (type == MPL_DB && fabs(lb - ub) < GLP_MPL_MIN_9 * (1.0 + fabs(lb))) {
      type = MPL_FX;
      if (fabs(lb) <= fabs(ub))
        ub = lb;
      else
        lb = ub;
    }
    if(type == MPL_UP || type == MPL_DB) {
      if(type == MPL_DB)
        lb = ub-lb;
      else
        lb = -1;
      type = ROWTYPE_LE;
    }
    else if(type == MPL_LO) {
      ub = lb;
      lb = -1;
      type = ROWTYPE_GE;
    }
    else if(type == MPL_FX) {
      lb = -1;
      type = ROWTYPE_EQ;
    }

    /* adjust for non-zero constant term in the constraint */
    ub -= mpl_get_row_c0(tran, i);

    /* set constraint coefficients */
    len = mpl_get_mat_row(tran, i, ndx, val);
    lp->add_constraintex(lp, len, val+1, ndx+1, type, ub);
    ii++;
    if(lb > 0)
      lp->set_rh_range(lp, ii, lb);

    /* set constraint name */
    lp->set_row_name(lp, ii, mpl_get_row_name(tran, i));

  }

  /* set status and free working arrays */
  glp_free(ndx);
  glp_free(val);

  /* free resources used by the model translator */
done:

  /* finally transpose the model A matrix */
  lp->set_add_rowmode(lp, FALSE);
  return lp;
}

int glp_lpsolve_mpl_postsolve(glp_tran *tran, lprec *lp, int sol)
{     /* postsolve the model */
      int i, j, m, n, stat, ret, has_shift, *bascolumn = NULL;
      glp_double prim, dual;
      if (!(tran->phase == GLP_TRAN_PHASE_GENERATE && !tran->flag_p))
         xerror("glp_lpsolve_mpl_postsolve: invalid call sequence\n");
      if (!(sol == GLP_SOL || sol == GLP_IPT || sol == GLP_MIP))
         xerror("glp_mpl_postsolve: sol = %d; invalid parameter\n",
            sol);
      m = mpl_get_num_rows(tran);
      n = mpl_get_num_cols(tran);
      if (!(m == get_Norig_rows(lp)+1 &&
            n == get_Norig_columns(lp)))
         xerror("glp_lpsolve_mpl_postsolve: wrong problem object\n");
      if (!mpl_has_solve_stmt(tran))
      {  ret = 0;
         goto done;
      }
      //print_solution(lp, 1);
      //print_constraints(lp, 1);
      //print_duals(lp);
      //print_lp(lp);

      bascolumn = (int*)glp_malloc((1 + m + n)*sizeof(int));
      *bascolumn = 0;
      if (get_basis(lp, bascolumn, TRUE))
      {
          bascolumn[0] = 1;
          //for (i = 0; i < m; i++) printf("basis row[%d] = %d\n", i, bascolumn[i]);
          //for (i = 1; i < n; i++) printf("basis col[%d] = %d\n", i, bascolumn[i+m]);
      }

      has_shift = get_rh(lp, 0) != 0.0;
      for (i = 0; i < m; i++)
      {
         stat = 0;//glp_get_row_stat(prob, i);
         prim = get_var_primalresult(lp, i);
         dual = get_var_dualresult(lp, i);
         //dual = lp->int_vars ? get_var_dualresult(lp, i) : 0.0;

         if (fabs(prim) < GLP_MPL_MIN_9) prim = 0.0;
         if (fabs(dual) < GLP_MPL_MIN_9) dual = 0.0;
         mpl_put_row_soln(tran, i+1, stat, prim, dual);
      }
      for (j = 1; j <= n; j++)
      {
         stat = 0;//glp_get_col_stat(prob, j+m-1);
         prim = get_var_primalresult(lp, j+m-1);
         dual = lp->int_vars ? get_var_dualresult(lp, j+m-1) : 0.0;
         if (fabs(prim) < GLP_MPL_MIN_9) prim = 0.0;
         if (fabs(dual) < GLP_MPL_MIN_9) dual = 0.0;
         mpl_put_col_soln(tran, j, stat, prim, dual);
      }
      ret = mpl_postsolve(tran);
      if (ret == 3)
         ret = 0;
      else if (ret == 4)
         ret = 1;
done:
      if(bascolumn) glp_free(bascolumn);
      return ret;
}

#endif

#ifdef WITH_CBC
#include "Cbc_C_Interface.h"

Cbc_Model *glp_cbc_mpl_build_prob(glp_tran *tran)
{
  int     ret, m, n, i, ii, j, *ndx, len, kind, type;
  glp_double  lb, ub, *val, infinite;
  if (tran->phase != GLP_TRAN_PHASE_GENERATE)
     xerror("glp_cbc_mpl_build_prob: invalid call sequence\n");
  Cbc_Model *lp =  Cbc_newModel();
  if(!lp) return lp;
  /* set problem name and temporary row storage order */
  Cbc_setProblemName(lp, mpl_get_prob_name(tran));

  /* build columns (variables) */
  n = mpl_get_num_cols(tran);
  for (j = 1; j <= n; j++) {

    /* set column kind */
    kind = mpl_get_col_kind(tran, j);
    switch (kind) {
      case MPL_NUM: kind = 0; break;
      case MPL_INT:
      case MPL_BIN: kind = 1; break;
      default:      xassert(kind != kind);
    }

    /* set column bounds */
    type = mpl_get_col_bnds(tran, j, &lb, &ub);
    switch (type) {
      case MPL_FR: break;
      case MPL_LO: break;
      case MPL_UP: break;
      case MPL_DB: break;
      case MPL_FX: break;
      default:     xassert(type != type);
    }
    if (kind == MPL_BIN) {
      if (type == MPL_FR || type == MPL_UP || lb < 0.0) lb = 0.0;
      if (type == MPL_FR || type == MPL_LO || ub > 1.0) ub = 1.0;
      type = MPL_DB;
    }
    if (type == MPL_DB && fabs(lb - ub) < GLP_MPL_MIN_9 * (1.0 + fabs(lb))) {
      type = MPL_FX;
      if (fabs(lb) <= fabs(ub)) ub = lb; else lb = ub;
    }

    Cbc_addCol(lp, mpl_get_col_name(tran, j),
          lb, ub, 0.0, kind, 0, NULL, NULL);
  }


  /* allocate working arrays */
  ndx = (int *) glp_malloc((1+n) * sizeof(*ndx));
  val = (glp_double *) glp_malloc((1+n) * sizeof(*val));

  /* build objective function (the first objective is used) */
  m = mpl_get_num_rows(tran);
  for (i = 1; i <= m; i++) {

    kind = mpl_get_row_kind(tran, i);
    if (kind == MPL_MIN || kind == MPL_MAX) {

      /* set optimization direction */
      lp->set_sense(lp, (MYBOOL) (kind == MPL_MAX) );

      /* set objective coefficients */
      len = mpl_get_mat_row(tran, i, ndx, val);
      lp->set_obj_fnex(lp, len, val+1, ndx+1);

      /* warn about non-zero constant term */
      if (mpl_get_row_c0(tran, i) != 0.0)
         xprintf("glp_cbc_mpl_build_prob: row %s; constant term %.12g ig"
               "nored\n",
               mpl_get_row_name(tran, i), mpl_get_row_c0(tran, i));
      /* set constant term */
      lp->set_rh(lp, 0, mpl_get_row_c0(tran, i));

      /* set objective name */
      lp->set_row_name(lp, 0, mpl_get_row_name(tran, i));
      break;
    }
    Cbc_addRow(Cbc_Model *model, const char *name, int nz,
        const int *cols, const glp_double *coefs, char sense, glp_double rhs);
  }

  /* build rows (constraints) */
  ii = 0;
  for (i = 1; i <= m; i++) {

    /* select constraint rows only */
    kind = mpl_get_row_kind(tran, i);
    if (kind == MPL_MIN || kind == MPL_MAX)
      continue;

    /* set row bounds */
    type = mpl_get_row_bnds(tran, i, &lb, &ub);
    switch (type) {
      case MPL_FR: break;
      case MPL_LO: break;
      case MPL_UP: break;
      case MPL_DB: break;
      case MPL_FX: break;
      default:     xassert(type != type);
    }
    if(type == MPL_FR)
      continue;
    if (type == MPL_DB && fabs(lb - ub) < GLP_MPL_MIN_9 * (1.0 + fabs(lb))) {
      type = MPL_FX;
      if (fabs(lb) <= fabs(ub))
        ub = lb;
      else
        lb = ub;
    }
    if(type == MPL_UP || type == MPL_DB) {
      if(type == MPL_DB)
        lb = ub-lb;
      else
        lb = -1;
      type = ROWTYPE_LE;
    }
    else if(type == MPL_LO) {
      ub = lb;
      lb = -1;
      type = ROWTYPE_GE;
    }
    else if(type == MPL_FX) {
      lb = -1;
      type = ROWTYPE_EQ;
    }

    /* adjust for non-zero constant term in the constraint */
    ub -= mpl_get_row_c0(tran, i);

    /* set constraint coefficients */
    len = mpl_get_mat_row(tran, i, ndx, val);
    lp->add_constraintex(lp, len, val+1, ndx+1, type, ub);
    ii++;
    if(lb > 0)
      lp->set_rh_range(lp, ii, lb);

    /* set constraint name */
    Cbc_setRowName(lp, ii, mpl_get_row_name(tran, i));

    Cbc_addRow(lp, mpl_get_row_name(tran, i), len,
        ndx+1, val+1, char sense, glp_double rhs);
  }
  /* set status and free working arrays */
  glp_free(ndx);
  glp_free(val);

  /* free resources used by the model translator */
done:

  /* finally transpose the model A matrix */
  lp->set_add_rowmode(lp, FALSE);
  return lp;
}

int glp_cbc_mpl_postsolve(glp_tran *tran, Cbc_Model *lp, int sol)
{     /* postsolve the model */
      int i, j, m, n, stat, ret, has_shift, *bascolumn = NULL;
      glp_double prim, dual;
      if (!(tran->phase == GLP_TRAN_PHASE_GENERATE && !tran->flag_p))
         xerror("glp_cbc_mpl_postsolve: invalid call sequence\n");
      if (!(sol == GLP_SOL || sol == GLP_IPT || sol == GLP_MIP))
         xerror("glp_cbc_mpl_postsolve: sol = %d; invalid parameter\n",
            sol);
      m = mpl_get_num_rows(tran);
      n = mpl_get_num_cols(tran);
      if (!(m == Cbc_getNumRows(lp) &&
            n == Cbc_getNumCols(lp)))
         xerror("glp_cbc_mpl_postsolve: wrong problem object\n");
      if (!mpl_has_solve_stmt(tran))
      {  ret = 0;
         goto done;
      }
      //print_solution(lp, 1);
      //print_constraints(lp, 1);
      //print_duals(lp);
      //print_lp(lp);

      bascolumn = (int*)glp_malloc((1 + m + n)*sizeof(int));
      *bascolumn = 0;
      if (get_basis(lp, bascolumn, TRUE))
      {
          bascolumn[0] = 1;
          //for (i = 0; i < m; i++) printf("basis row[%d] = %d\n", i, bascolumn[i]);
          //for (i = 1; i < n; i++) printf("basis col[%d] = %d\n", i, bascolumn[i+m]);
      }

      has_shift = get_rh(lp, 0) != 0.0;
      for (i = 0; i < m; i++)
      {
         stat = 0;//glp_get_row_stat(prob, i);
         prim = get_var_primalresult(lp, i);
         dual = get_var_dualresult(lp, i);
         //dual = lp->int_vars ? get_var_dualresult(lp, i) : 0.0;

         if (fabs(prim) < GLP_MPL_MIN_9) prim = 0.0;
         if (fabs(dual) < GLP_MPL_MIN_9) dual = 0.0;
         mpl_put_row_soln(tran, i+1, stat, prim, dual);
      }
      for (j = 1; j <= n; j++)
      {
         stat = 0;//glp_get_col_stat(prob, j+m-1);
         prim = get_var_primalresult(lp, j+m-1);
         dual = lp->int_vars ? get_var_dualresult(lp, j+m-1) : 0.0;
         if (fabs(prim) < GLP_MPL_MIN_9) prim = 0.0;
         if (fabs(dual) < GLP_MPL_MIN_9) dual = 0.0;
         mpl_put_col_soln(tran, j, stat, prim, dual);
      }
      ret = mpl_postsolve(tran);
      if (ret == 3)
         ret = 0;
      else if (ret == 4)
         ret = 1;
done:
      if(bascolumn) glp_free(bascolumn);
      return ret;
}

#endif

/* eof */
