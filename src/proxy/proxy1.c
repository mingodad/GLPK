/* proxy1.c */

/* (reserved for copyright notice) */

#include "glpios.h"
#include "proxy.h"

void ios_proxy_heur(glp_tree *T)
{     glp_prob *prob;
      int j, status;
      double *xstar, zstar;
      /* this heuristic is applied only once on the root level */
      if (!(T->curr->level == 0 && T->curr->solved == 1))
         goto done;
      prob = glp_create_prob();
      glp_copy_prob(prob, T->mip, 0);
      xstar = xcalloc(1+prob->n, sizeof(double));
      for (j = 1; j <= prob->n; j++)
         xstar[j] = 0.0;
      /* time limit = 1 min = 60 sec */
      status = proxy(prob, &zstar, xstar, NULL, 0.0, 60000, 1);
      if (status == 0)
         glp_ios_heur_sol(T, xstar);
      xfree(xstar);
      glp_delete_prob(prob);
done: return;
}

/* eof */
