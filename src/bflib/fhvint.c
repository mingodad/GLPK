/* fhvint.c (interface to FHV-factorization) */

/***********************************************************************
*  This code is part of GLPK (GNU Linear Programming Kit).
*
*  Copyright (C) 2012, 2013 Andrew Makhorin, Department for Applied
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

#include "env.h"
#include "fhv.h"
#include "sgf.h"
#include "fhvint.h"

struct FHVINT
{     /* interface to FHV-factorization */
      int valid;
      /* factorization is valid only if this flag is set */
      int n_max;
      /* maximal value of n (increased automatically) */
      FHV *fhv;
      /* FHV-factorization */
      SGF *sgf;
      /* factorizer workspace */
};

FHVINT *fhvint_create(void)
{     /* create interface to FHV-factorization */
      FHVINT *fi;
      fi = talloc(1, FHVINT);
      fi->valid = 0;
      fi->n_max = 0;
      fi->fhv = NULL;
      fi->sgf = NULL;
      return fi;
}

static void setup_v_cols(LUF *luf, int (*col)(void *info, int j,
      int ind[], double val[]), void *info, int ind[], double val[])
{     /* setup matrix V = A in column-wise format */
      int n = luf->n;
      SVA *sva = luf->sva;
      int *sv_ind = sva->ind;
      double *sv_val = sva->val;
      int vc_ref = luf->vc_ref;
      int *vc_ptr = &sva->ptr[vc_ref-1];
      int *vc_len = &sva->len[vc_ref-1];
      int *vc_cap = &sva->cap[vc_ref-1];
      int j, len, ptr;
      for (j = 1; j <= n; j++)
      {  /* get j-th column */
         len = col(info, j, ind, val);
         xassert(0 <= len && len <= n);
         /* enlarge j-th column capacity */
         if (vc_cap[j] < len)
         {  if (sva->r_ptr - sva->m_ptr < len)
            {  sva_more_space(sva, len);
               sv_ind = sva->ind;
               sv_val = sva->val;
            }
            sva_enlarge_cap(sva, vc_ref-1+j, len, 0);
         }
         /* store j-th column */
         ptr = vc_ptr[j];
         memcpy(&sv_ind[ptr], &ind[1], len * sizeof(int));
         memcpy(&sv_val[ptr], &val[1], len * sizeof(double));
         vc_len[j] = len;
      }
      return;
}

static int factorize(FHVINT *fi, int (*col)(void *info, int j,
      int ind[], double val[]), void *info)
{     /* compute FHV-factorization of specified matrix A */
      FHV *fhv = fi->fhv;
      LUF *luf = fhv->luf;
      SVA *sva = luf->sva;
      SGF *sgf = fi->sgf;
      int n = luf->n;
      int vr_ref = luf->vr_ref;
      int *vr_len = &sva->len[vr_ref-1];
      double *vr_piv = luf->vr_piv;
      int vc_ref = luf->vc_ref;
      int *vc_len = &sva->len[vc_ref-1];
      int *pp_ind = luf->pp_ind;
      int *pp_inv = luf->pp_inv;
      int *qq_ind = luf->qq_ind;
      int *qq_inv = luf->qq_inv;
      int *p0_ind = fhv->p0_ind;
      int *p0_inv = fhv->p0_inv;
      int *rs_head = sgf->rs_head;
      int *rs_prev = sgf->rs_prev;
      int *rs_next = sgf->rs_next;
      int *cs_head = sgf->cs_head;
      int *cs_prev = sgf->cs_prev;
      int *cs_next = sgf->cs_next;
      double *vr_max = sgf->vr_max;
      char *flag = sgf->flag;
      double *work = sgf->work;
      int i, j, k, p, q;
      /* setup matrix V = A in column-wise format */
      setup_v_cols(luf, col, info, rs_head, work);
      /* build matrix V = A in row-wise format */
      luf_build_v_rows(luf, rs_head);
      for (i = 1; i <= n; i++)
         vr_piv[i] = 0.0;
      /* P := Q := I */
      for (k = 1; k <= n; k++)
         pp_ind[k] = pp_inv[k] = qq_ind[k] = qq_inv[k] = k;
      /* build active lists and initialize working arrays */
      for (k = 0; k <= n; k++)
         rs_head[k] = cs_head[k] = 0;
      for (k = 1; k <= n; k++)
      {  sgf_activate_row(k);
         sgf_activate_col(k);
         vr_max[k] = -1.0;
         flag[k] = 0;
         work[k] = 0.0;
      }
      /* currently V = A, F = P = Q = I */
      /* main factorization loop */
      for (k = 1; k <= n; k++)
      {  /* choose pivot v[p,q] */
         if (sgf_choose_pivot(sgf, &p, &q) != 0)
            return k; /* failure */
         /* u[i,j] = v[p,q], k <= i, j <= n */
         i = pp_ind[p];
         xassert(k <= i && i <= n);
         j = qq_inv[q];
         xassert(k <= j && j <= n);
         /* move u[i,j] to position u[k,k] by implicit permutations of
          * rows and columns of matrix U */
         luf_swap_u_rows(k, i);
         luf_swap_u_cols(k, j);
         /* perform gaussian elimination */
         sgf_eliminate(sgf, p, q);
      }
      /* defragment SVA; currently all columns of V are empty, so they
       * will have zero capacity as required by luf_build_v_cols */
      sva_defrag_area(sva);
      /* build matrix F in row-wise format */
      luf_build_f_rows(luf, rs_head);
      /* build matrix V in column-wise format */
      luf_build_v_cols(luf, sgf->updat, rs_head);
      /* H := I */
      fhv->nfs = 0;
      /* P0 := P */
      for (k = 1; k <= n; k++)
      {  p0_ind[k] = pp_ind[k];
         p0_inv[k] = pp_inv[k];
      }
      return 0;
}

int fhvint_factorize(FHVINT *fi, int n, int (*col)(void *info, int j,
      int ind[], double val[]), void *info)
{     /* compute FHV-factorization of specified matrix A */
      FHV *fhv;
      LUF *luf;
      SVA *sva;
      SGF *sgf;
      xassert(n > 0);
      fi->valid = 0;
      if (fi->n_max == 0)
      {  /* create underlying program objects */
         int n_max = n;
         int nfs_max = 100;
         /* create sparse vector area (SVA) */
         sva = sva_create_area(4 * n + nfs_max, 10 * n);
         /* create sparse LU-factorization (LUF) */
         luf = talloc(1, LUF);
         memset(luf, 0, sizeof(LUF));
         luf->sva = sva;
         luf->vr_piv = talloc(1+n_max, double);
         luf->pp_ind = talloc(1+n_max, int);
         luf->pp_inv = talloc(1+n_max, int);
         luf->qq_ind = talloc(1+n_max, int);
         luf->qq_inv = talloc(1+n_max, int);
         /* create sparse updatable FHV-factorization (FHV) */
         fhv = talloc(1, FHV);
         memset(fhv, 0, sizeof(FHV));
         fhv->luf = luf;
         fhv->nfs_max = nfs_max;
         fhv->hh_ind = talloc(1+nfs_max, int);
         fhv->p0_ind = talloc(1+n_max, int);
         fhv->p0_inv = talloc(1+n_max, int);
         /* create sparse gaussian factorizer workspace (SGF) */
         sgf = talloc(1, SGF);
         memset(sgf, 0, sizeof(SGF));
         sgf->luf = luf;
         sgf->rs_head = talloc(1+n_max, int);
         sgf->rs_prev = talloc(1+n_max, int);
         sgf->rs_next = talloc(1+n_max, int);
         sgf->cs_head = talloc(1+n_max, int);
         sgf->cs_prev = talloc(1+n_max, int);
         sgf->cs_next = talloc(1+n_max, int);
         sgf->vr_max = talloc(1+n_max, double);
         sgf->flag = talloc(1+n_max, char);
         sgf->work = talloc(1+n_max, double);
         sgf->updat = 1;
         sgf->piv_tol = 0.10;
         sgf->piv_lim = 4;
         sgf->suhl = 1;
         sgf->eps_tol = DBL_EPSILON;
         sgf->den_lim = 0.53; /* currently not used */
         /* finalize initialization */
         fi->n_max = n_max;
         fi->fhv = fhv;
         fi->sgf = sgf;
      }
      fhv = fi->fhv;
      luf = fhv->luf;
      sva = luf->sva;
      sgf = fi->sgf;
      if (fi->n_max < n)
      {  /* enlarge underlying program objects */
         int n_max = n + 100;
         tfree(luf->vr_piv);
         tfree(luf->pp_ind);
         tfree(luf->pp_inv);
         tfree(luf->qq_ind);
         tfree(luf->qq_inv);
         tfree(fhv->p0_ind);
         tfree(fhv->p0_inv);
         tfree(sgf->rs_head);
         tfree(sgf->rs_prev);
         tfree(sgf->rs_next);
         tfree(sgf->cs_head);
         tfree(sgf->cs_prev);
         tfree(sgf->cs_next);
         tfree(sgf->vr_max);
         tfree(sgf->flag);
         tfree(sgf->work);
         fi->n_max = n_max;
         luf->vr_piv = talloc(1+n_max, double);
         luf->pp_ind = talloc(1+n_max, int);
         luf->pp_inv = talloc(1+n_max, int);
         luf->qq_ind = talloc(1+n_max, int);
         luf->qq_inv = talloc(1+n_max, int);
         fhv->p0_ind = talloc(1+n_max, int);
         fhv->p0_inv = talloc(1+n_max, int);
         sgf->rs_head = talloc(1+n_max, int);
         sgf->rs_prev = talloc(1+n_max, int);
         sgf->rs_next = talloc(1+n_max, int);
         sgf->cs_head = talloc(1+n_max, int);
         sgf->cs_prev = talloc(1+n_max, int);
         sgf->cs_next = talloc(1+n_max, int);
         sgf->vr_max = talloc(1+n_max, double);
         sgf->flag = talloc(1+n_max, char);
         sgf->work = talloc(1+n_max, double);
      }
      /* clear sparse vector area (SVA) */
      sva->n = 0;
      sva->m_ptr = 1;
      sva->r_ptr = sva->size + 1;
      sva->head = sva->tail = 0;
      /* allocate sparse vectors in SVA */
      luf->n = n;
      luf->fr_ref = sva_alloc_vecs(sva, n);
      luf->fc_ref = sva_alloc_vecs(sva, n);
      luf->vr_ref = sva_alloc_vecs(sva, n);
      luf->vc_ref = sva_alloc_vecs(sva, n);
      fhv->hh_ref = sva_alloc_vecs(sva, fhv->nfs_max);
      /* compute FHV-factorization */
      if (factorize(fi, col, info) != 0)
         return 1; /* failure */
      /* factorization has been successfully computed */
      fi->valid = 1;
      return 0;
}

void fhvint_ftran(FHVINT *fi, double x[])
{     /* solve system A * x = b */
      FHV *fhv = fi->fhv;
      LUF *luf = fhv->luf;
      int n = luf->n;
      int *pp_ind = luf->pp_ind;
      int *pp_inv = luf->pp_inv;
      SGF *sgf = fi->sgf;
      double *work = sgf->work;
      xassert(fi->valid);
      /* A = F * H * V */
      /* x = inv(A) * b = inv(V) * inv(H) * inv(F) * b */
      luf->pp_ind = fhv->p0_ind;
      luf->pp_inv = fhv->p0_inv;
      luf_f_solve(luf, x);
      luf->pp_ind = pp_ind;
      luf->pp_inv = pp_inv;
      fhv_h_solve(fhv, x);
      luf_v_solve(luf, x, work);
      memcpy(&x[1], &work[1], n * sizeof(double));
      return;
}

void fhvint_btran(FHVINT *fi, double x[])
{     /* solve system A'* x = b */
      FHV *fhv = fi->fhv;
      LUF *luf = fhv->luf;
      int n = luf->n;
      int *pp_ind = luf->pp_ind;
      int *pp_inv = luf->pp_inv;
      SGF *sgf = fi->sgf;
      double *work = sgf->work;
      xassert(fi->valid);
      /* A' = (F * H * V)' = V'* H'* F' */
      /* x = inv(A') * b = inv(F') * inv(H') * inv(V') * b */
      luf_vt_solve(luf, x, work);
      fhv_ht_solve(fhv, work);
      luf->pp_ind = fhv->p0_ind;
      luf->pp_inv = fhv->p0_inv;
      luf_ft_solve(luf, work);
      luf->pp_ind = pp_ind;
      luf->pp_inv = pp_inv;
      memcpy(&x[1], &work[1], n * sizeof(double));
      return;
}

int fhvint_update(FHVINT *fi, int j, int len, const int ind[],
      const double val[])
{     /* update FHV-factorization after replacing j-th column of A */
      SGF *sgf = fi->sgf;
      int *ind1 = sgf->rs_head;
      double *val1 = sgf->vr_max;
      double *work = sgf->work;
      int ret;
      xassert(fi->valid);
      ret = fhv_ft_update(fi->fhv, j, len, ind, val, ind1, val1, work);
      if (ret != 0)
         fi->valid = 0;
      return ret;
}

void fhvint_delete(FHVINT *fi)
{     /* delete interface to FHV-factorization */
      if (fi->n_max > 0)
      {  FHV *fhv = fi->fhv;
         LUF *luf = fhv->luf;
         SGF *sgf = fi->sgf;
         /* delete sparse vector area */
         sva_delete_area(luf->sva);
         /* delete sparse LU-factorization (LUF) */
         tfree(luf->vr_piv);
         tfree(luf->pp_ind);
         tfree(luf->pp_inv);
         tfree(luf->qq_ind);
         tfree(luf->qq_inv);
         tfree(luf);
         /* delete sparse updatable FHV-factorization (FHV) */
         tfree(fhv->hh_ind);
         tfree(fhv->p0_ind);
         tfree(fhv->p0_inv);
         tfree(fhv);
         /* delete sparse gaussian factorizer workspace (SGF) */
         tfree(sgf->rs_head);
         tfree(sgf->rs_prev);
         tfree(sgf->rs_next);
         tfree(sgf->cs_head);
         tfree(sgf->cs_prev);
         tfree(sgf->cs_next);
         tfree(sgf->vr_max);
         tfree(sgf->flag);
         tfree(sgf->work);
         tfree(sgf);
      }
      tfree(fi);
      return;
}

/* eof */
