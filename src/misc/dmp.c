/* dmp.c (dynamic memory pool) */

/***********************************************************************
*  This code is part of GLPK (GNU Linear Programming Kit).
*
*  Copyright (C) 2000-2013 Andrew Makhorin, Department for Applied
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
#include "dmp.h"

#define DMP_BLK_SIZE 8192 //4096
/* size of memory blocks, in bytes, allocated for memory pools */

#define DMP_MAX_ATOM_SIZE 256 //1024 //256
/* max size of an atom in bytes, allocated for memory pools */

#define DMP_AVAIL_SIZE 32 //128 //32
/* max size of an atom in bytes, allocated for memory pools */


struct DMP
{     /* dynamic memory pool */
      void *avail[DMP_AVAIL_SIZE];
      /* avail[k], 0 <= k <= 31, is a pointer to first available (free)
       * atom of (k+1)*8 bytes long; at the beginning of each free atom
       * there is a pointer to another free atom of the same size */
      void *block;
      /* pointer to most recently allocated memory block; at the
       * beginning of each allocated memory block there is a pointer to
       * previously allocated memory block */
      int used;
      /* number of bytes used in most recently allocated memory block */
      size_t count;
      /* number of atoms which are currently in use */
};

struct prefix
{     /* atom prefix (for debugging only) */
      DMP *pool;
      /* dynamic memory pool */
      int size;
      /* original atom size, in bytes */
};

#define prefix_size ((sizeof(struct prefix) + 7) & ~7)
/* size of atom prefix rounded up to multiple of 8 bytes */

#if !(defined(NDEBUG) || defined(NO_XASSERT))
int dmp_debug;
/* debug mode flag */
#endif

/***********************************************************************
*  NAME
*
*  dmp_create_pool - create dynamic memory pool
*
*  SYNOPSIS
*
*  #include "dmp.h"
*  DMP *dmp_create_pool(void);
*
*  DESCRIPTION
*
*  The routine dmp_create_pool creates a dynamic memory pool.
*
*  RETURNS
*
*  The routine returns a pointer to the memory pool created. */

DMP *dmp_create_pool(void)
{     DMP *pool;
      int k;
      xassert(sizeof(void *) <= 8);
#if !(defined(NDEBUG) || defined(NO_XASSERT))
      if (dmp_debug)
         xprintf("dmp_create_pool: warning: debug mode is on\n");
#endif
      pool = talloc(1, DMP);
      for (k = 0; k <= 31; k++)
         pool->avail[k] = NULL;
      pool->block = NULL;
      pool->used = DMP_BLK_SIZE;
      pool->count = 0;
      return pool;
}

/***********************************************************************
*  NAME
*
*  dmp_get_atom - get free atom from dynamic memory pool
*
*  SYNOPSIS
*
*  #include "dmp.h"
*  void *dmp_get_atom(DMP *pool, int size);
*
*  DESCRIPTION
*
*  The routine dmp_get_atom obtains a free atom (memory space) from the
*  specified memory pool.
*
*  The parameter size is the atom size, in bytes, 1 <= size <= 256.
*
*  Note that the free atom contains arbitrary data, not binary zeros.
*
*  RETURNS
*
*  The routine returns a pointer to the free atom obtained. */

void *dmp_get_atom_base(DMP *pool, int size)
{     void *atom;
      int k, need;
      xassert(1 <= size && size <= DMP_MAX_ATOM_SIZE);
      /* round up atom size to multiple of 8 bytes */
      need = (size + 7) & ~7;
      /* determine number of corresponding list of free atoms */
      k = (need >> 3) - 1;
      /* obtain free atom */
      if (pool->avail[k] == NULL)
      {  /* corresponding list of free atoms is empty */
         /* if debug mode is on, add atom prefix size */
#if !(defined(NDEBUG) || defined(NO_XASSERT))
         if (dmp_debug)
            need += prefix_size;
#endif
         if (pool->used + need > DMP_BLK_SIZE)
         {  /* allocate new memory block */
            void *block = talloc(DMP_BLK_SIZE, char);
            *(void **)block = pool->block;
            pool->block = block;
            pool->used = sizeof(void*); /* sufficient to store pointer */
         }
         /* allocate new atom in current memory block */
         atom = (char *)pool->block + pool->used;
         pool->used += need;
      }
      else
      {  /* obtain atom from corresponding list of free atoms */
         atom  = pool->avail[k];
         pool->avail[k] = *(void **)atom;
      }
#if !(defined(NDEBUG) || defined(NO_XASSERT))
      /* if debug mode is on, fill atom prefix */
      if (dmp_debug)
      {  ((struct prefix *)atom)->pool = pool;
         ((struct prefix *)atom)->size = size;
         atom = (char *)atom + prefix_size;
      }
#endif
      /* increase number of allocated atoms */
      pool->count++;
      return atom;
}

#ifdef DMP_LOG
void *dmp_get_atom_log(DMP *pool, int size, const char *filename, int lineno)
{
    void *p = dmp_get_atom_base(pool, size);
    fprintf(dmp_log_fp, "ga:%s:%d:%d:%p\n", filename, lineno, size, p);
    return p;
}
#endif
/***********************************************************************
*  NAME
*
*  dmp_free_atom - return atom to dynamic memory pool
*
*  SYNOPSIS
*
*  #include "dmp.h"
*  void dmp_free_atom(DMP *pool, void *atom, int size);
*
*  DESCRIPTION
*
*  The routine dmp_free_atom returns the specified atom (memory space)
*  to the specified memory pool, making the atom free.
*
*  The parameter size is the atom size, in bytes, 1 <= size <= 256.
*
*  Note that the atom can be returned only to the pool, from which it
*  was obtained, and its size must be exactly the same as on obtaining
*  it from the pool. */

void dmp_free_atom(DMP *pool, void *atom, int size)
{     int k;
      xassert(1 <= size && size <= DMP_MAX_ATOM_SIZE);
      /* determine number of corresponding list of free atoms */
      k = ((size + 7) >> 3) - 1;
      /* if debug mode is on, check atom prefix */
#if !(defined(NDEBUG) || defined(NO_XASSERT))
      if (dmp_debug)
      {  atom = (char *)atom - prefix_size;
         xassert(((struct prefix *)atom)->pool == pool);
         xassert(((struct prefix *)atom)->size == size);
      }
#endif
      /* return atom to corresponding list of free atoms */
      *(void **)atom = pool->avail[k];
      pool->avail[k] = atom;
      /* decrease number of allocated atoms */
      xassert(pool->count > 0);
      pool->count--;
      return;
}

/***********************************************************************
*  NAME
*
*  dmp_in_use - determine how many atoms are still in use
*
*  SYNOPSIS
*
*  #include "dmp.h"
*  size_t dmp_in_use(DMP *pool);
*
*  RETURNS
*
*  The routine returns the number of atoms of the specified memory pool
*  which are still in use. */

size_t dmp_in_use(DMP *pool)
{     return
         pool->count;
}

/***********************************************************************
*  NAME
*
*  dmp_delete_pool - delete dynamic memory pool
*
*  SYNOPSIS
*
*  #include "dmp.h"
*  void dmp_delete_pool(DMP *pool);
*
*  DESCRIPTION
*
*  The routine dmp_delete_pool deletes the specified dynamic memory
*  pool freeing all the memory allocated to this object. */

void dmp_delete_pool(DMP *pool)
{     while (pool->block != NULL)
      {  void *block = pool->block;
         pool->block = *(void **)block;
         tfree(block);
      }
      tfree(pool);
      return;
}

/* eof */
