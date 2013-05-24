/* env.h (GLPK environment) */

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

#ifndef ENV_H
#define ENV_H

#include "glpenv.h"

#undef xmalloc
#undef xcalloc
#undef xrealloc
#undef xfree

#define talloc(n, type) ((type *)glp_calloc(n, sizeof(type)))

#define trealloc(ptr, n, type) ((type *)glp_realloc(ptr, n, \
      sizeof(type)))

#define tfree(ptr) glp_free(ptr)

#endif

/* eof */
