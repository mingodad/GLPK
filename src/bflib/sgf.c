/* sgf.c (sparse Gaussian factorizer) */

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
#include "sgf.h"

/***********************************************************************
*  sgf_choose_pivot - choose pivot element v[p,q]
*
*  This routine chooses pivot element v[p,q], k <= p, q <= n, in the
*  active submatrix of matrix V = P * U * Q, where k is the number of
*  current elimination step, 1 <= k <= n.
*
*  It is assumed that on entry to the routine matrix U = P'* V * Q' has
*  the following partially triangularized form:
*
*        1       k       n
*     1  x x x x x x x x x
*        . x x x x x x x x
*        . . x x x x x x x
*        . . . x x x x x x
*     k  . . . . * * * * *
*        . . . . * * * * *
*        . . . . * * * * *
*        . . . . * * * * *
*     n  . . . . * * * * *
*
*  where rows and columns k, k+1, ..., n belong to the active submatrix
*  (its elements are marked by '*').
*
*  Since the matrix U is not stored, the routine works with the matrix
*  V = P * U * Q. It is assumed that the row-wise representation
*  corresponds to the matrix V, but the column-wise representation
*  corresponds to the active submatrix of the matrix V, i.e. elements,
*  which are not in the active submatrix, are not included in column
*  vectors. It is also assumed that each active row of the matrix V is
*  in the set R[len], where len is the number of non-zeros in the row,
*  and each active column of the matrix V is in the set C[len], where
*  len is the number of non-zeros in the column (in the latter case
*  only elements of the active submatrix are counted; such elements are
*  marked by '*' on the figure above).
*
*  For the reason of numerical stability the routine applies so called
*  threshold pivoting proposed by J.Reid. It is assumed that an element
*  v[i,j] can be selected as a pivot candidate if it is not very small
*  (in magnitude) among other elements in the same row, i.e. if it
*  satisfies to the stability condition |v[i,j]| >= tol * max|v[i,*]|,
*  where 0 < tol < 1 is a given tolerance.
*
*  In order to keep sparsity of the matrix V the routine uses Markowitz
*  strategy, trying to choose such element v[p,q], which satisfies to
*  the stability condition (see above) and has smallest Markowitz cost
*  (nr[p]-1) * (nc[q]-1), where nr[p] and nc[q] are, resp., numbers of
*  non-zeros in p-th row and q-th column of the active submatrix.
*
*  In order to reduce the search, i.e. not to walk through all elements
*  of the active submatrix, the routine uses a technique proposed by
*  I.Duff. This technique is based on using the sets R[len] and C[len]
*  of active rows and columns.
*
*  If the pivot element v[p,q] has been chosen, the routine stores its
*  indices to locations *p and *q and returns zero. Otherwise, non-zero
*  is returned. */

int sgf_choose_pivot(SGF *sgf, int *p_, int *q_)
{     LUF *luf = sgf->luf;
      int n = luf->n;
      SVA *sva = luf->sva;
      int *sv_ind = sva->ind;
      double *sv_val = sva->val;
      int vr_ref = luf->vr_ref;
      int *vr_ptr = &sva->ptr[vr_ref-1];
      int *vr_len = &sva->len[vr_ref-1];
      int vc_ref = luf->vc_ref;
      int *vc_ptr = &sva->ptr[vc_ref-1];
      int *vc_len = &sva->len[vc_ref-1];
      int *rs_head = sgf->rs_head;
      int *rs_next = sgf->rs_next;
      int *cs_head = sgf->cs_head;
      int *cs_prev = sgf->cs_prev;
      int *cs_next = sgf->cs_next;
      double *vr_max = sgf->vr_max;
      double piv_tol = sgf->piv_tol;
      int piv_lim = sgf->piv_lim;
      int suhl = sgf->suhl;
      int i, i_ptr, i_end, j, j_ptr, j_end, len, min_i, min_j, min_len,
         ncand, next_j, p, q;
      double best, big, cost, temp;
      /* no pivot candidate has been chosen so far */
      p = q = 0, best = DBL_MAX, ncand = 0;
      /* if the active submatrix contains a column having the only
       * non-zero element (column singleton), choose it as the pivot */
      j = cs_head[1];
      if (j != 0)
      {  xassert(vc_len[j] == 1);
         p = sv_ind[vc_ptr[j]], q = j;
         goto done;
      }
      /* if the active submatrix contains a row having the only
       * non-zero element (row singleton), choose it as the pivot */
      i = rs_head[1];
      if (i != 0)
      {  xassert(vr_len[i] == 1);
         p = i, q = sv_ind[vr_ptr[i]];
         goto done;
      }
      /* the active submatrix contains no singletons; walk thru its
       * other non-empty rows and columns */
      for (len = 2; len <= n; len++)
      {  /* consider active columns containing len non-zeros */
         for (j = cs_head[len]; j != 0; j = next_j)
         {  /* save the number of next column of the same length */
            next_j = cs_next[j];
            /* find an element in j-th column, which is placed in the
             * row with minimal number of non-zeros and satisfies to
             * the stability condition (such element may not exist) */
            min_i = min_j = 0, min_len = INT_MAX;
            for (j_end = (j_ptr = vc_ptr[j]) + vc_len[j];
               j_ptr < j_end; j_ptr++)
            {  /* get row index of v[i,j] */
               i = sv_ind[j_ptr];
               /* if i-th row is not shorter, skip v[i,j] */
               if (vr_len[i] >= min_len)
                  continue;
               /* big := max|v[i,*]| */
               if ((big = vr_max[i]) < 0.0)
               {  /* largest magnitude is unknown; compute it */
                  for (i_end = (i_ptr = vr_ptr[i]) + vr_len[i];
                     i_ptr < i_end; i_ptr++)
                  {  if ((temp = sv_val[i_ptr]) < 0.0)
                        temp = -temp;
                     if (big < temp)
                        big = temp;
                  }
                  xassert(big > 0.0);
                  vr_max[i] = big;
               }
               /* find v[i,j] in i-th row */
               for (i_end = (i_ptr = vr_ptr[i]) + vr_len[i];
                  sv_ind[i_ptr] != j; i_ptr++)
                  /* nop */;
               xassert(i_ptr < i_end);
               /* if |v[i,j]| < piv_tol * max|v[i,*]|, skip v[i,j] */
               if ((temp = sv_val[i_ptr]) < 0.0)
                  temp = -temp;
               if (temp < piv_tol * big)
                  continue;
               /* v[i,j] is a better candidate */
               min_i = i, min_j = j, min_len = vr_len[i];
               /* if Markowitz cost of v[i,j] is not greater than
                * (len-1)**2, v[i,j] can be chosen as the pivot right
                * now; this heuristic reduces the search and works well
                * in many cases */
               if (min_len <= len)
               {  p = min_i, q = min_j;
                  goto done;
               }
            }
            /* j-th column has been scanned */
            if (min_i != 0)
            {  /* element v[min_i,min_j] is a next pivot candidate */
               ncand++;
               /* compute its Markowitz cost */
               cost = (double)(min_len - 1) * (double)(len - 1);
               /* if this element is better, choose it as the pivot */
               if (cost < best)
                  p = min_i, q = min_j, best = cost;
               /* if piv_lim candidates were considered, terminate
                * the search, because it is doubtful that a much better
                * candidate will be found */
               if (ncand == piv_lim)
                  goto done;
            }
            else if (suhl)
            {  /* j-th column has no eligible elements that satisfy to
                * the stability criterion; Uwe Suhl suggests to exclude
                * such column from further considerations until it
                * becomes a column singleton; in hard cases this may
                * significantly reduce the time needed to choose the
                * pivot element */
               sgf_deactivate_col(j);
               cs_prev[j] = cs_next[j] = j;
            }
         }
         /* consider active rows containing len non-zeros */
         for (i = rs_head[len]; i != 0; i = rs_next[i])
         {  /* big := max|v[i,*]| */
            if ((big = vr_max[i]) < 0.0)
            {  /* largest magnitude is unknown; compute it */
               for (i_end = (i_ptr = vr_ptr[i]) + vr_len[i];
                  i_ptr < i_end; i_ptr++)
               {  if ((temp = sv_val[i_ptr]) < 0.0)
                     temp = -temp;
                  if (big < temp)
                     big = temp;
               }
               xassert(big > 0.0);
               vr_max[i] = big;
            }
            /* find an element in i-th row, which is placed in the
             * column with minimal number of non-zeros and satisfies to
             * the stability condition (such element always exists) */
            min_i = min_j = 0, min_len = INT_MAX;
            for (i_end = (i_ptr = vr_ptr[i]) + vr_len[i];
               i_ptr < i_end; i_ptr++)
            {  /* get column index of v[i,j] */
               j = sv_ind[i_ptr];
               /* if j-th column is not shorter, skip v[i,j] */
               if (vc_len[j] >= min_len)
                  continue;
               /* if |v[i,j]| < piv_tol * max|v[i,*]|, skip v[i,j] */
               if ((temp = sv_val[i_ptr]) < 0.0)
                  temp = -temp;
               if (temp < piv_tol * big)
                  continue;
               /* v[i,j] is a better candidate */
               min_i = i, min_j = j, min_len = vc_len[j];
               /* if Markowitz cost of v[i,j] is not greater than
                * (len-1)**2, v[i,j] can be chosen as the pivot right
                * now; this heuristic reduces the search and works well
                * in many cases */
               if (min_len <= len)
               {  p = min_i, q = min_j;
                  goto done;
               }
            }
            /* i-th row has been scanned */
            if (min_i != 0)
            {  /* element v[min_i,min_j] is a next pivot candidate */
               ncand++;
               /* compute its Markowitz cost */
               cost = (double)(len - 1) * (double)(min_len - 1);
               /* if this element is better, choose it as the pivot */
               if (cost < best)
                  p = min_i, q = min_j, best = cost;
               /* if piv_lim candidates were considered, terminate
                * the search, because it is doubtful that a much better
                * candidate will be found */
               if (ncand == piv_lim)
                  goto done;
            }
            else
            {  /* this can never be */
               xassert(min_i != min_i);
            }
         }
      }
done: /* report the pivot to the factorization routine */
      *p_ = p, *q_ = q;
      return (p == 0);
}

/***********************************************************************
*  sgf_eliminate - perform gaussian elimination
*
*  This routine performs elementary gaussian transformations in order
*  to eliminate subdiagonal elements in k-th column of matrix
*  U = P'* V * Q' using pivot element u[k,k], where k is the number of
*  current elimination step, 1 <= k <= n.
*
*  The parameters p and q specify, resp., row and column indices of the
*  pivot element v[p,q] = u[k,k].
*
*  On entry the routine assumes that partially triangularized matrices
*  L = P'* F * P and U = P'* V * Q' have the following structure:
*
*        1       k       n       1       k       n
*     1  1 . . . . . . . .    1  x x x x x x x x x
*        x 1 . . . . . . .       . x x x x x x x x
*        x x 1 . . . . . .       . . x x x x x x x
*        x x x 1 . . . . .       . . . x x x x x x
*     k  x x x x 1 . . . .    k  . . . . * * * * *
*        x x x x _ 1 . . .       . . . . # * * * *
*        x x x x _ . 1 . .       . . . . # * * * *
*        x x x x _ . . 1 .       . . . . # * * * *
*     n  x x x x _ . . . 1    n  . . . . # * * * *
*
*            matrix L                matrix U
*
*  where rows and columns k, k+1, ..., n of matrix U constitute the
*  active submatrix. Elements to be eliminated are marked by '#', and
*  other elements of the active submatrix are marked by '*'. May note
*  that each eliminated non-zero element u[i,k] of matrix U gives
*  corresponding non-zero element l[i,k] of matrix L (marked by '_').
*
*  Actually all operations are performed on matrix V. It is assumed
*  that the row-wise representation corresponds to matrix V, but the
*  column-wise representation corresponds to the active submatrix of
*  matrix V (or, more precisely, to its pattern, because only row
*  indices for columns of the active submatrix are used on this stage).
*
*  Let u[k,k] = v[p,q] be the pivot. In order to eliminate subdiagonal
*  elements u[i',k] = v[i,q], i'= k+1, k+2, ..., n, the routine applies
*  the following elementary gaussian transformations:
*
*     (i-th row of V) := (i-th row of V) - f[i,p] * (p-th row of V),
*
*  where f[i,p] = v[i,q] / v[p,q] is a gaussian multiplier stored to
*  p-th column of matrix F to keep the main equality A = F * V
*  (corresponding elements l[i',k] of matrix L are marked by '_' on the
*  figure above).
*
*  NOTE: On entry to the routine the working arrays flag and work
*        should contain zeros. This status is retained by the routine
*        on exit. */

int sgf_eliminate(SGF *sgf, int p, int q)
{     LUF *luf = sgf->luf;
      int n = luf->n;
      SVA *sva = luf->sva;
      int *sv_ind = sva->ind;
      double *sv_val = sva->val;
      int fc_ref = luf->fc_ref;
      int *fc_ptr = &sva->ptr[fc_ref-1];
      int *fc_len = &sva->len[fc_ref-1];
      int vr_ref = luf->vr_ref;
      int *vr_ptr = &sva->ptr[vr_ref-1];
      int *vr_len = &sva->len[vr_ref-1];
      int *vr_cap = &sva->cap[vr_ref-1];
      double *vr_piv = luf->vr_piv;
      int vc_ref = luf->vc_ref;
      int *vc_ptr = &sva->ptr[vc_ref-1];
      int *vc_len = &sva->len[vc_ref-1];
      int *vc_cap = &sva->cap[vc_ref-1];
      int *rs_head = sgf->rs_head;
      int *rs_prev = sgf->rs_prev;
      int *rs_next = sgf->rs_next;
      int *cs_head = sgf->cs_head;
      int *cs_prev = sgf->cs_prev;
      int *cs_next = sgf->cs_next;
      double *vr_max = sgf->vr_max;
      char *flag = sgf->flag;
      double *work = sgf->work;
      double eps_tol = sgf->eps_tol;
      int nnz_diff = 0;
      int fill, i, i_ptr, i_end, j, j_ptr, j_end, ptr, len, loc, loc1;
      double vpq, fip, vij;
      xassert(1 <= p && p <= n);
      xassert(1 <= q && q <= n);
      /* remove p-th row from the active set; this row will never
       * return there */
      sgf_deactivate_row(p);
      /* process p-th (pivot) row */
      ptr = 0;
      for (i_end = (i_ptr = vr_ptr[p]) + vr_len[p];
         i_ptr < i_end; i_ptr++)
      {  /* get column index of v[p,j] */
         j = sv_ind[i_ptr];
         if (j == q)
         {  /* save pointer to pivot v[p,q] */
            ptr = i_ptr;
         }
         else
         {  /* store v[p,j], j != q, to working array */
            flag[j] = 1;
            work[j] = sv_val[i_ptr];
         }
         /* remove j-th column from the active set; q-th column will
          * never return there while other columns will return to the
          * active set with new length */
         if (cs_next[j] == j)
         {  /* j-th column was marked by the pivoting routine according
             * to Uwe Suhl's suggestion and is already inactive */
            xassert(cs_prev[j] == j);
         }
         else
            sgf_deactivate_col(j);
         nnz_diff -= vc_len[j];
         /* find and remove v[p,j] from j-th column */
         for (j_end = (j_ptr = vc_ptr[j]) + vc_len[j];
            sv_ind[j_ptr] != p; j_ptr++)
            /* nop */;
         xassert(j_ptr < j_end);
         sv_ind[j_ptr] = sv_ind[j_end-1];
         vc_len[j]--;
      }
      /* save pivot v[p,q] and remove it from p-th row */
      xassert(ptr > 0);
      vpq = vr_piv[p] = sv_val[ptr];
      sv_ind[ptr] = sv_ind[i_end-1];
      sv_val[ptr] = sv_val[i_end-1];
      vr_len[p]--;
      /* if it is not planned to update matrix V, relocate p-th row to
       * the right (static) part of SVA */
      if (!sgf->updat)
      {  len = vr_len[p];
         if (sva->r_ptr - sva->m_ptr < len)
         {  sva_more_space(sva, len);
            sv_ind = sva->ind;
            sv_val = sva->val;
         }
         sva_make_static(sva, vr_ref-1+p);
      }
      /* copy the pattern (row indices) of q-th column of the active
       * submatrix (from which v[p,q] has been just removed) to p-th
       * column of matrix F (without unity diagonal element) */
      len = vc_len[q];
      if (len > 0)
      {  if (sva->r_ptr - sva->m_ptr < len)
         {  sva_more_space(sva, len);
            sv_ind = sva->ind;
            sv_val = sva->val;
         }
         sva_reserve_cap(sva, fc_ref-1+p, len);
         memcpy(&sv_ind[fc_ptr[p]], &sv_ind[vc_ptr[q]],
            len * sizeof(int));
         fc_len[p] = len;
      }
      /* make q-th column of the active submatrix empty */
      vc_len[q] = 0;
      /* transform non-pivot rows of the active submatrix */
      for (loc = fc_len[p]-1; loc >= 0; loc--)
      {  /* get row index of v[i,q] = row index of f[i,p] */
         i = sv_ind[fc_ptr[p] + loc];
         xassert(i != p); /* v[p,q] was removed */
         /* remove i-th row from the active set; this row will return
          * there with new length */
         sgf_deactivate_row(i);
         /* find v[i,q] in i-th row */
         for (i_end = (i_ptr = vr_ptr[i]) + vr_len[i];
            sv_ind[i_ptr] != q; i_ptr++)
            /* nop */;
         xassert(i_ptr < i_end);
         /* compute gaussian multiplier f[i,p] = v[i,q] / v[p,q] */
         fip = sv_val[fc_ptr[p] + loc] = sv_val[i_ptr] / vpq;
         /* remove v[i,q] from i-th row */
         sv_ind[i_ptr] = sv_ind[i_end-1];
         sv_val[i_ptr] = sv_val[i_end-1];
         vr_len[i]--;
         /* perform elementary gaussian transformation:
          * (i-th row) := (i-th row) - f[i,p] * (p-th row)
          * note that p-th row of V, which is in the working array,
          * doesn't contain pivot v[p,q], and i-th row of V doesn't
          * contain v[i,q] to be eliminated */
         /* walk thru i-th row and transform existing elements */
         fill = vr_len[p];
         for (i_end = (i_ptr = ptr = vr_ptr[i]) + vr_len[i];
            i_ptr < i_end; i_ptr++)
         {  /* get column index and value of v[i,j] */
            j = sv_ind[i_ptr];
            vij = sv_val[i_ptr];
            if (flag[j])
            {  /* v[p,j] != 0 */
               flag[j] = 0, fill--;
               /* v[i,j] := v[i,j] - f[i,p] * v[p,j] */
               vij -= fip * work[j];
               if (-eps_tol < vij && vij < +eps_tol)
               {  /* new v[i,j] is close to zero; remove it from the
                   * active submatrix, i.e. replace it by exact zero */
                  /* find and remove v[i,j] from j-th column */
                  for (j_end = (j_ptr = vc_ptr[j]) + vc_len[j];
                     sv_ind[j_ptr] != i; j_ptr++)
                     /* nop */;
                  xassert(j_ptr < j_end);
                  sv_ind[j_ptr] = sv_ind[j_end-1];
                  vc_len[j]--;
                  continue;
               }
            }
            /* keep new v[i,j] in i-th row */
            sv_ind[ptr] = j;
            sv_val[ptr] = vij;
            ptr++;
         }
         /* (new length of i-th row may decrease because of numerical
          * cancellation) */
         vr_len[i] = len = ptr - vr_ptr[i];
         /* now flag[*] is the pattern of the set v[p,*] \ v[i,*], and
          * fill is the number of non-zeros in this set */
         if (fill == 0)
         {  /* no fill-in occurs */
            /* walk thru p-th row and restore the column flags */
            for (i_end = (i_ptr = vr_ptr[p]) + vr_len[p];
               i_ptr < i_end; i_ptr++)
               flag[sv_ind[i_ptr]] = 1; /* v[p,j] != 0 */
            goto skip;
         }
         /* up to fill new non-zero elements may appear in i-th row due
          * to fill-in; reserve locations for these elements (note that
          * actual length of i-th row is currently stored in len) */
         if (vr_cap[i] < len + fill)
         {  if (sva->r_ptr - sva->m_ptr < len + fill)
            {  sva_more_space(sva, len + fill);
               sv_ind = sva->ind;
               sv_val = sva->val;
            }
            sva_enlarge_cap(sva, vr_ref-1+i, len + fill, 0);
         }
         vr_len[i] += fill;
         /* walk thru p-th row and add new elements to i-th row */
         for (loc1 = vr_len[p]-1; loc1 >= 0; loc1--)
         {  /* get column index of v[p,j] */
            j = sv_ind[vr_ptr[p] + loc1];
            if (!flag[j])
            {  /* restore j-th column flag */
               flag[j] = 1;
               /* v[i,j] was computed earlier on transforming existing
                * elements of i-th row */
               continue;
            }
            /* v[i,j] := 0 - f[i,p] * v[p,j] */
            vij = - fip * work[j];
            if (-eps_tol < vij && vij < +eps_tol)
            {  /* new v[i,j] is close to zero; do not add it to the
                * active submatrix, i.e. replace it by exact zero */
               continue;
            }
            /* add new v[i,j] to i-th row */
            sv_ind[ptr = vr_ptr[i] + (len++)] = j;
            sv_val[ptr] = vij;
            /* add new v[i,j] to j-th column */
            if (vc_cap[j] == vc_len[j])
            {  /* we reserve extra locations in j-th column to reduce
                * further relocations of that column */
#if 1 /* FIXME */
               /* use control parameter to specify the number of extra
                * locations reserved */
               int need = vc_len[j] + 10;
#endif
               if (sva->r_ptr - sva->m_ptr < need)
               {  sva_more_space(sva, need);
                  sv_ind = sva->ind;
                  sv_val = sva->val;
               }
               sva_enlarge_cap(sva, vc_ref-1+j, need, 1);
            }
            sv_ind[vc_ptr[j] + (vc_len[j]++)] = i;
         }
         /* set final length of i-th row just transformed */
         xassert(len <= vr_len[i]);
         vr_len[i] = len;
skip:    /* return i-th row to the active set with new length */
         sgf_activate_row(i);
         /* since i-th row has been changed, largest magnitude of its
          * elements becomes unknown */
         vr_max[i] = -1.0;
      }
      /* walk thru p-th (pivot) row */
      for (i_end = (i_ptr = vr_ptr[p]) + vr_len[p];
         i_ptr < i_end; i_ptr++)
      {  /* get column index of v[p,j] */
         j = sv_ind[i_ptr];
         xassert(j != q); /* v[p,q] was removed */
         /* return j-th column to the active set with new length */
         if (cs_next[j] == j && vc_len[j] != 1)
         {  /* j-th column was marked by the pivoting routine and it is
             * still not a column singleton, so leave it incative */
            xassert(cs_prev[j] == j);
         }
         else
            sgf_activate_col(j);
         nnz_diff += vc_len[j];
         /* restore zero content of the working arrays */
         flag[j] = 0;
         work[j] = 0.0;
      }
      /* return the difference between the numbers of non-zeros in the
       * active submatrix on entry and on exit, resp. */
      return nnz_diff;
}

/* eof */
