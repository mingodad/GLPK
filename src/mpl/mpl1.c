/* mpl1.c */

/***********************************************************************
*  This code is part of GLPK (GNU Linear Programming Kit).
*
*  Copyright (C) 2003-2016 Andrew Makhorin, Department for Applied
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

#define dmp_get_atomv dmp_get_atom

/**********************************************************************/
/* * *                  PROCESSING MODEL SECTION                  * * */
/**********************************************************************/

/*----------------------------------------------------------------------
-- enter_context - enter current token into context queue.
--
-- This routine enters the current token into the context queue. */

void enter_context(MPL *mpl)
{     char *image, *s;
      if (mpl->scan_input->token == T_EOF)
         image = "_|_";
      else if (mpl->scan_input->token == T_STRING)
         image = "'...'";
      else
         image = mpl->scan_input->image;
      xassert(0 <= mpl->scan_input->c_ptr && mpl->scan_input->c_ptr < CONTEXT_SIZE);
      mpl->scan_input->context[mpl->scan_input->c_ptr++] = ' ';
      if (mpl->scan_input->c_ptr == CONTEXT_SIZE) mpl->scan_input->c_ptr = 0;
      for (s = image; *s != '\0'; s++)
      {  mpl->scan_input->context[mpl->scan_input->c_ptr++] = *s;
         if (mpl->scan_input->c_ptr == CONTEXT_SIZE) mpl->scan_input->c_ptr = 0;
      }
      return;
}

/*----------------------------------------------------------------------
-- print_context - print current content of context queue.
--
-- This routine prints current content of the context queue. */

void print_context(MPL *mpl)
{     int c;
      while (mpl->scan_input->c_ptr > 0)
      {  mpl->scan_input->c_ptr--;
         c = mpl->scan_input->context[0];
         memmove(mpl->scan_input->context, mpl->scan_input->context+1, CONTEXT_SIZE-1);
         mpl->scan_input->context[CONTEXT_SIZE-1] = (char)c;
      }
      xprintf("Context: %s%.*s\n", mpl->scan_input->context[0] == ' ' ? "" : "...",
         CONTEXT_SIZE, mpl->scan_input->context);
      return;
}

/*----------------------------------------------------------------------
-- get_char - scan next character from input text file.
--
-- This routine scans a next ASCII character from the input text file.
-- In case of end-of-file, the character is assigned EOF. */

void get_char(MPL *mpl)
{     int c;
      if (mpl->scan_input->c == EOF) goto done;
      if (mpl->scan_input->c == '\n') mpl->scan_input->line++;
      c = read_char(mpl);
      if (c == EOF)
      {  if (mpl->scan_input->c == '\n')
            mpl->scan_input->line--;
         else
            warning(mpl, "final NL missing before end of file");
      }
      else if (c == '\n')
         ;
      else if (isspace(c))
         c = ' ';
      else if (iscntrl(c))
      {  enter_context(mpl);
         error(mpl, "control character 0x%02X not allowed", c);
      }
      mpl->scan_input->c = c;
done: return;
}

/*----------------------------------------------------------------------
-- append_char - append character to current token.
--
-- This routine appends the current character to the current token and
-- then scans a next character. */

void append_char(MPL *mpl)
{     xassert(0 <= mpl->scan_input->imlen && mpl->scan_input->imlen <= MAX_LENGTH);
      if (mpl->scan_input->imlen == MAX_LENGTH)
      {  switch (mpl->scan_input->token)
         {  case T_NAME:
               enter_context(mpl);
               error(mpl, "symbolic name %s... too long", mpl->scan_input->image);
            case T_SYMBOL:
               enter_context(mpl);
               error(mpl, "symbol %s... too long", mpl->scan_input->image);
            case T_NUMBER:
               enter_context(mpl);
               error(mpl, "numeric literal %s... too long", mpl->scan_input->image);
            case T_STRING:
               enter_context(mpl);
               error(mpl, "string literal too long");
            default:
               xassert(mpl != mpl);
         }
      }
      mpl->scan_input->image[mpl->scan_input->imlen++] = (char)mpl->scan_input->c;
      mpl->scan_input->image[mpl->scan_input->imlen] = '\0';
      get_char(mpl);
      return;
}

/*----------------------------------------------------------------------
-- get_token - scan next token from input text file.
--
-- This routine scans a next token from the input text file using the
-- standard finite automation technique. */

void get_token(MPL *mpl)
{     /* save the current token */
      mpl->scan_input->b_token = mpl->scan_input->token;
      mpl->scan_input->b_imlen = mpl->scan_input->imlen;
      strcpy(mpl->scan_input->b_image, mpl->scan_input->image);
      mpl->scan_input->b_value = mpl->scan_input->value;
      /* if the next token is already scanned, make it current */
      if (mpl->scan_input->f_scan)
      {  mpl->scan_input->f_scan = 0;
         mpl->scan_input->token = mpl->scan_input->f_token;
         mpl->scan_input->imlen = mpl->scan_input->f_imlen;
         strcpy(mpl->scan_input->image, mpl->scan_input->f_image);
         mpl->scan_input->value = mpl->scan_input->f_value;
         goto done;
      }
loop: /* nothing has been scanned so far */
      mpl->scan_input->token = 0;
      mpl->scan_input->imlen = 0;
      mpl->scan_input->image[0] = '\0';
      mpl->scan_input->value = 0.0;
      /* skip any uninteresting characters */
      while (mpl->scan_input->c == ' ' || mpl->scan_input->c == '\n') get_char(mpl);
      /* recognize and construct the token */
      if (mpl->scan_input->c == EOF)
      {  /* end-of-file reached */
         mpl->scan_input->token = T_EOF;
      }
      else if (mpl->scan_input->c == '#')
      {  /* comment; skip anything until end-of-line */
         while (mpl->scan_input->c != '\n' && mpl->scan_input->c != EOF) get_char(mpl);
         goto loop;
      }
      else if (!mpl->flag_d && (isalpha(mpl->scan_input->c) || mpl->scan_input->c == '_'))
      {  /* symbolic name or reserved keyword */
         mpl->scan_input->token = T_NAME;
         while (isalnum(mpl->scan_input->c) || mpl->scan_input->c == '_') append_char(mpl);
         if (strcmp(mpl->scan_input->image, "and") == 0)
            mpl->scan_input->token = T_AND;
         else if (strcmp(mpl->scan_input->image, "by") == 0)
            mpl->scan_input->token = T_BY;
         else if (strcmp(mpl->scan_input->image, "cross") == 0)
            mpl->scan_input->token = T_CROSS;
         else if (strcmp(mpl->scan_input->image, "diff") == 0)
            mpl->scan_input->token = T_DIFF;
         else if (strcmp(mpl->scan_input->image, "div") == 0)
            mpl->scan_input->token = T_DIV;
         else if (strcmp(mpl->scan_input->image, "else") == 0)
            mpl->scan_input->token = T_ELSE;
         else if (strcmp(mpl->scan_input->image, "if") == 0)
            mpl->scan_input->token = T_IF;
         else if (strcmp(mpl->scan_input->image, "in") == 0)
            mpl->scan_input->token = T_IN;
#if 1 /* 21/VII-2006 */
         else if (strcmp(mpl->scan_input->image, "Infinity") == 0)
            mpl->scan_input->token = T_INFINITY;
#endif
         else if (strcmp(mpl->scan_input->image, "inter") == 0)
            mpl->scan_input->token = T_INTER;
         else if (strcmp(mpl->scan_input->image, "less") == 0)
            mpl->scan_input->token = T_LESS;
         else if (strcmp(mpl->scan_input->image, "mod") == 0)
            mpl->scan_input->token = T_MOD;
         else if (strcmp(mpl->scan_input->image, "not") == 0)
            mpl->scan_input->token = T_NOT;
         else if (strcmp(mpl->scan_input->image, "or") == 0)
            mpl->scan_input->token = T_OR;
         else if (strcmp(mpl->scan_input->image, "s") == 0 && mpl->scan_input->c == '.')
         {  mpl->scan_input->token = T_SPTP;
            append_char(mpl);
            if (mpl->scan_input->c != 't')
sptp:       {  enter_context(mpl);
               error(mpl, "keyword s.t. incomplete");
            }
            append_char(mpl);
            if (mpl->scan_input->c != '.') goto sptp;
            append_char(mpl);
         }
         else if (strcmp(mpl->scan_input->image, "symdiff") == 0)
            mpl->scan_input->token = T_SYMDIFF;
         else if (strcmp(mpl->scan_input->image, "then") == 0)
            mpl->scan_input->token = T_THEN;
         else if (strcmp(mpl->scan_input->image, "union") == 0)
            mpl->scan_input->token = T_UNION;
         else if (strcmp(mpl->scan_input->image, "within") == 0)
            mpl->scan_input->token = T_WITHIN;
      }
      else if (!mpl->flag_d && isdigit(mpl->scan_input->c))
      {  /* numeric literal */
         mpl->scan_input->token = T_NUMBER;
         /* scan integer part */
         while (isdigit(mpl->scan_input->c)) append_char(mpl);
         /* scan optional fractional part */
         if (mpl->scan_input->c == '.')
         {  append_char(mpl);
            if (mpl->scan_input->c == '.')
            {  /* hmm, it is not the fractional part, it is dots that
                  follow the integer part */
               mpl->scan_input->imlen--;
               mpl->scan_input->image[mpl->scan_input->imlen] = '\0';
               mpl->scan_input->f_dots = 1;
               goto conv;
            }
frac:       while (isdigit(mpl->scan_input->c)) append_char(mpl);
         }
         /* scan optional decimal exponent */
         if (mpl->scan_input->c == 'e' || mpl->scan_input->c == 'E')
         {  append_char(mpl);
            if (mpl->scan_input->c == '+' || mpl->scan_input->c == '-') append_char(mpl);
            if (!isdigit(mpl->scan_input->c))
            {  enter_context(mpl);
               error(mpl, "numeric literal %s incomplete", mpl->scan_input->image);
            }
            while (isdigit(mpl->scan_input->c)) append_char(mpl);
         }
         /* there must be no letter following the numeric literal */
         if (isalpha(mpl->scan_input->c) || mpl->scan_input->c == '_')
         {  enter_context(mpl);
            error(mpl, "symbol %s%c... should be enclosed in quotes",
               mpl->scan_input->image, mpl->scan_input->c);
         }
conv:    /* convert numeric literal to floating-point */
         if (str2num(mpl->scan_input->image, &mpl->scan_input->value))
err:     {  enter_context(mpl);
            error(mpl, "cannot convert numeric literal %s to floating-p"
               "oint number", mpl->scan_input->image);
         }
      }
      else if (mpl->scan_input->c == '\'' || mpl->scan_input->c == '"')
      {  /* character string */
         int quote = mpl->scan_input->c;
         mpl->scan_input->token = T_STRING;
         get_char(mpl);
         for (;;)
         {  if (mpl->scan_input->c == '\n' || mpl->scan_input->c == EOF)
            {  enter_context(mpl);
               error(mpl, "unexpected end of line; string literal incom"
                  "plete");
            }
            if (mpl->scan_input->c == quote)
            {  get_char(mpl);
               if (mpl->scan_input->c != quote) break;
            }
            append_char(mpl);
         }
      }
      else if (!mpl->flag_d && mpl->scan_input->c == '+')
         mpl->scan_input->token = T_PLUS, append_char(mpl);
      else if (!mpl->flag_d && mpl->scan_input->c == '-')
         mpl->scan_input->token = T_MINUS, append_char(mpl);
      else if (mpl->scan_input->c == '*')
      {  mpl->scan_input->token = T_ASTERISK, append_char(mpl);
         if (mpl->scan_input->c == '*')
            mpl->scan_input->token = T_POWER, append_char(mpl);
      }
      else if (mpl->scan_input->c == '/')
      {  mpl->scan_input->token = T_SLASH, append_char(mpl);
         if (mpl->scan_input->c == '*')
         {  /* comment sequence */
            get_char(mpl);
            for (;;)
            {  if (mpl->scan_input->c == EOF)
               {  /* do not call enter_context at this point */
                  error(mpl, "unexpected end of file; comment sequence "
                     "incomplete");
               }
               else if (mpl->scan_input->c == '*')
               {  get_char(mpl);
                  if (mpl->scan_input->c == '/') break;
               }
               else
                  get_char(mpl);
            }
            get_char(mpl);
            goto loop;
         }
      }
      else if (mpl->scan_input->c == '^')
         mpl->scan_input->token = T_POWER, append_char(mpl);
      else if (mpl->scan_input->c == '<')
      {  mpl->scan_input->token = T_LT, append_char(mpl);
         if (mpl->scan_input->c == '=')
            mpl->scan_input->token = T_LE, append_char(mpl);
         else if (mpl->scan_input->c == '>')
            mpl->scan_input->token = T_NE, append_char(mpl);
#if 1 /* 11/II-2008 */
         else if (mpl->scan_input->c == '-')
            mpl->scan_input->token = T_INPUT, append_char(mpl);
#endif
      }
      else if (mpl->scan_input->c == '=')
      {  mpl->scan_input->token = T_EQ, append_char(mpl);
         if (mpl->scan_input->c == '=') append_char(mpl);
      }
      else if (mpl->scan_input->c == '>')
      {  mpl->scan_input->token = T_GT, append_char(mpl);
         if (mpl->scan_input->c == '=')
            mpl->scan_input->token = T_GE, append_char(mpl);
#if 1 /* 14/VII-2006 */
         else if (mpl->scan_input->c == '>')
            mpl->scan_input->token = T_APPEND, append_char(mpl);
#endif
      }
      else if (mpl->scan_input->c == '!')
      {  mpl->scan_input->token = T_NOT, append_char(mpl);
         if (mpl->scan_input->c == '=')
            mpl->scan_input->token = T_NE, append_char(mpl);
      }
      else if (mpl->scan_input->c == '&')
      {  mpl->scan_input->token = T_CONCAT, append_char(mpl);
         if (mpl->scan_input->c == '&')
            mpl->scan_input->token = T_AND, append_char(mpl);
      }
      else if (mpl->scan_input->c == '|')
      {  mpl->scan_input->token = T_BAR, append_char(mpl);
         if (mpl->scan_input->c == '|')
            mpl->scan_input->token = T_OR, append_char(mpl);
      }
      else if (!mpl->flag_d && mpl->scan_input->c == '.')
      {  mpl->scan_input->token = T_POINT, append_char(mpl);
         if (mpl->scan_input->f_dots)
         {  /* dots; the first dot was read on the previous call to the
               scanner, so the current character is the second dot */
            mpl->scan_input->token = T_DOTS;
            mpl->scan_input->imlen = 2;
            strcpy(mpl->scan_input->image, "..");
            mpl->scan_input->f_dots = 0;
         }
         else if (mpl->scan_input->c == '.')
            mpl->scan_input->token = T_DOTS, append_char(mpl);
         else if (isdigit(mpl->scan_input->c))
         {  /* numeric literal that begins with the decimal point */
            mpl->scan_input->token = T_NUMBER, append_char(mpl);
            goto frac;
         }
      }
      else if (mpl->scan_input->c == ',')
         mpl->scan_input->token = T_COMMA, append_char(mpl);
      else if (mpl->scan_input->c == ':')
      {  mpl->scan_input->token = T_COLON, append_char(mpl);
         if (mpl->scan_input->c == '=')
            mpl->scan_input->token = T_ASSIGN, append_char(mpl);
      }
      else if (mpl->scan_input->c == ';')
         mpl->scan_input->token = T_SEMICOLON, append_char(mpl);
      else if (mpl->scan_input->c == '(')
         mpl->scan_input->token = T_LEFT, append_char(mpl);
      else if (mpl->scan_input->c == ')')
         mpl->scan_input->token = T_RIGHT, append_char(mpl);
      else if (mpl->scan_input->c == '[')
         mpl->scan_input->token = T_LBRACKET, append_char(mpl);
      else if (mpl->scan_input->c == ']')
         mpl->scan_input->token = T_RBRACKET, append_char(mpl);
      else if (mpl->scan_input->c == '{')
         mpl->scan_input->token = T_LBRACE, append_char(mpl);
      else if (mpl->scan_input->c == '}')
         mpl->scan_input->token = T_RBRACE, append_char(mpl);
#if 1 /* 11/II-2008 */
      else if (mpl->scan_input->c == '~')
         mpl->scan_input->token = T_TILDE, append_char(mpl);
#endif
      else if (isalnum(mpl->scan_input->c) || strchr("+-._", mpl->scan_input->c) != NULL)
      {  /* symbol */
         xassert(mpl->flag_d);
         mpl->scan_input->token = T_SYMBOL;
         while (isalnum(mpl->scan_input->c) || strchr("+-._", mpl->scan_input->c) != NULL)
            append_char(mpl);
         switch (str2num(mpl->scan_input->image, &mpl->scan_input->value))
         {  case 0:
               mpl->scan_input->token = T_NUMBER;
               break;
            case 1:
               goto err;
            case 2:
               break;
            default:
               xassert(mpl != mpl);
         }
      }
      else
      {  enter_context(mpl);
         error(mpl, "character %c not allowed", mpl->scan_input->c);
      }
      /* enter the current token into the context queue */
      enter_context(mpl);
      /* reset the flag, which may be set by indexing_expression() and
         is used by expression_list() */
      mpl->flag_x = 0;
done: return;
}

/*----------------------------------------------------------------------
-- expect_token - check current and scan next token from input text file.
--
-- This routine check current and scans a next token from the input text file using the
-- standard finite automation technique. */

void expect_token(MPL *mpl, int token)
{
    if(mpl->scan_input->token != token)
        error(mpl, "token expected %d but got %d", token, mpl->scan_input->token);
    get_token(mpl);
}

/*----------------------------------------------------------------------
-- unget_token - return current token back to input stream.
--
-- This routine returns the current token back to the input stream, so
-- the previously scanned token becomes the current one. */

void unget_token(MPL *mpl)
{     /* save the current token, which becomes the next one */
      xassert(!mpl->scan_input->f_scan);
      mpl->scan_input->f_scan = 1;
      mpl->scan_input->f_token = mpl->scan_input->token;
      mpl->scan_input->f_imlen = mpl->scan_input->imlen;
      strcpy(mpl->scan_input->f_image, mpl->scan_input->image);
      mpl->scan_input->f_value = mpl->scan_input->value;
      /* restore the previous token, which becomes the current one */
      mpl->scan_input->token = mpl->scan_input->b_token;
      mpl->scan_input->imlen = mpl->scan_input->b_imlen;
      strcpy(mpl->scan_input->image, mpl->scan_input->b_image);
      mpl->scan_input->value = mpl->scan_input->b_value;
      return;
}

/*----------------------------------------------------------------------
-- is_keyword - check if current token is given non-reserved keyword.
--
-- If the current token is given (non-reserved) keyword, this routine
-- returns non-zero. Otherwise zero is returned. */

int is_keyword(MPL *mpl, char *keyword)
{     return
         mpl->scan_input->token == T_NAME && strcmp(mpl->scan_input->image, keyword) == 0;
}

/*----------------------------------------------------------------------
-- is_reserved - check if current token is reserved keyword.
--
-- If the current token is a reserved keyword, this routine returns
-- non-zero. Otherwise zero is returned. */

int is_reserved(MPL *mpl)
{     return
         mpl->scan_input->token == T_AND && mpl->scan_input->image[0] == 'a' ||
         mpl->scan_input->token == T_BY ||
         mpl->scan_input->token == T_CROSS ||
         mpl->scan_input->token == T_DIFF ||
         mpl->scan_input->token == T_DIV ||
         mpl->scan_input->token == T_ELSE ||
         mpl->scan_input->token == T_IF ||
         mpl->scan_input->token == T_IN ||
         mpl->scan_input->token == T_INTER ||
         mpl->scan_input->token == T_LESS ||
         mpl->scan_input->token == T_MOD ||
         mpl->scan_input->token == T_NOT && mpl->scan_input->image[0] == 'n' ||
         mpl->scan_input->token == T_OR && mpl->scan_input->image[0] == 'o' ||
         mpl->scan_input->token == T_SYMDIFF ||
         mpl->scan_input->token == T_THEN ||
         mpl->scan_input->token == T_UNION ||
         mpl->scan_input->token == T_WITHIN;
}

/*----------------------------------------------------------------------
-- make_code - generate pseudo-code (basic routine).
--
-- This routine generates specified pseudo-code. It is assumed that all
-- other translator routines use this basic routine. */

CODE *make_code(MPL *mpl, int op, OPERANDS *arg, int type, int dim)
{     CODE *code;
      DOMAIN *domain;
      DOMAIN_BLOCK *block;
      ARG_LIST *e;
      /* generate pseudo-code */
      code = alloc(CODE);
      code->op = op;
      code->vflag = 0; /* is inherited from operand(s) */
      /* copy operands and also make them referring to the pseudo-code
         being generated, because the latter becomes the parent for all
         its operands */
      memset(&code->arg, '?', sizeof(OPERANDS));
      switch (op)
      {  case O_NUMBER:
            code->arg.num = arg->num;
            break;
         case O_STRING:
            code->arg.str = arg->str;
            break;
         case O_INDEX:
            code->arg.index.slot = arg->index.slot;
            code->arg.index.next = arg->index.next;
            break;
         case O_MEMNUM:
         case O_MEMSYM:
            for (e = arg->par.list; e != NULL; e = e->next)
            {  xassert(e->x != NULL);
               xassert(e->x->up == NULL);
               e->x->up = code;
               code->vflag |= e->x->vflag;
            }
            code->arg.par.par = arg->par.par;
            code->arg.par.list = arg->par.list;
            break;
         case O_MEMSET:
            for (e = arg->set.list; e != NULL; e = e->next)
            {  xassert(e->x != NULL);
               xassert(e->x->up == NULL);
               e->x->up = code;
               code->vflag |= e->x->vflag;
            }
            code->arg.set.set = arg->set.set;
            code->arg.set.list = arg->set.list;
            code->arg.set.elemset = arg->set.elemset;
            break;
         case O_MEMVAR:
            for (e = arg->var.list; e != NULL; e = e->next)
            {  xassert(e->x != NULL);
               xassert(e->x->up == NULL);
               e->x->up = code;
               code->vflag |= e->x->vflag;
            }
            code->arg.var.var = arg->var.var;
            code->arg.var.list = arg->var.list;
#if 1 /* 15/V-2010 */
            code->arg.var.suff = arg->var.suff;
#endif
            break;
#if 1 /* 15/V-2010 */
         case O_MEMCON:
            for (e = arg->con.list; e != NULL; e = e->next)
            {  xassert(e->x != NULL);
               xassert(e->x->up == NULL);
               e->x->up = code;
               code->vflag |= e->x->vflag;
            }
            code->arg.con.con = arg->con.con;
            code->arg.con.list = arg->con.list;
            code->arg.con.suff = arg->con.suff;
            break;
#endif
         case O_TUPLE:
         case O_MAKE:
            for (e = arg->list; e != NULL; e = e->next)
            {  xassert(e->x != NULL);
               xassert(e->x->up == NULL);
               e->x->up = code;
               code->vflag |= e->x->vflag;
            }
            code->arg.list = arg->list;
            break;
         case O_SLICE:
            xassert(arg->slice != NULL);
            code->arg.slice = arg->slice;
            break;
         case O_IRAND224:
         case O_UNIFORM01:
         case O_NORMAL01:
         case O_GMTIME:
            code->vflag = 1;
            break;
         case O_CVTNUM:
         case O_CVTSYM:
         case O_CVTLOG:
         case O_CVTTUP:
         case O_CVTLFM:
         case O_PLUS:
         case O_MINUS:
         case O_NOT:
         case O_ABS:
         case O_CEIL:
         case O_FLOOR:
         case O_EXP:
         case O_LOG:
         case O_LOG10:
         case O_SQRT:
         case O_SIN:
         case O_COS:
         case O_TAN:
         case O_ATAN:
         case O_ROUND:
         case O_TRUNC:
         case O_CARD:
         case O_LENGTH:
            /* unary operation */
            xassert(arg->arg.x != NULL);
            xassert(arg->arg.x->up == NULL);
            arg->arg.x->up = code;
            code->vflag |= arg->arg.x->vflag;
            code->arg.arg.x = arg->arg.x;
            break;
         case O_ADD:
         case O_SUB:
         case O_LESS:
         case O_MUL:
         case O_DIV:
         case O_IDIV:
         case O_MOD:
         case O_POWER:
         case O_ATAN2:
         case O_ROUND2:
         case O_TRUNC2:
         case O_UNIFORM:
            if (op == O_UNIFORM) code->vflag = 1;
         case O_NORMAL:
            if (op == O_NORMAL) code->vflag = 1;
         case O_CONCAT:
         case O_LT:
         case O_LE:
         case O_EQ:
         case O_GE:
         case O_GT:
         case O_NE:
         case O_AND:
         case O_OR:
         case O_UNION:
         case O_DIFF:
         case O_SYMDIFF:
         case O_INTER:
         case O_CROSS:
         case O_IN:
         case O_NOTIN:
         case O_WITHIN:
         case O_NOTWITHIN:
         case O_SUBSTR:
         case O_STR2TIME:
         case O_TIME2STR:
            /* binary operation */
            xassert(arg->arg.x != NULL);
            xassert(arg->arg.x->up == NULL);
            arg->arg.x->up = code;
            code->vflag |= arg->arg.x->vflag;
            xassert(arg->arg.y != NULL);
            xassert(arg->arg.y->up == NULL);
            arg->arg.y->up = code;
            code->vflag |= arg->arg.y->vflag;
            code->arg.arg.x = arg->arg.x;
            code->arg.arg.y = arg->arg.y;
            break;
         case O_DOTS:
         case O_FORK:
         case O_SUBSTR3:
            /* ternary operation */
            xassert(arg->arg.x != NULL);
            xassert(arg->arg.x->up == NULL);
            arg->arg.x->up = code;
            code->vflag |= arg->arg.x->vflag;
            xassert(arg->arg.y != NULL);
            xassert(arg->arg.y->up == NULL);
            arg->arg.y->up = code;
            code->vflag |= arg->arg.y->vflag;
            if (arg->arg.z != NULL)
            {  xassert(arg->arg.z->up == NULL);
               arg->arg.z->up = code;
               code->vflag |= arg->arg.z->vflag;
            }
            code->arg.arg.x = arg->arg.x;
            code->arg.arg.y = arg->arg.y;
            code->arg.arg.z = arg->arg.z;
            break;
         case O_MIN:
         case O_MAX:
            /* n-ary operation */
            for (e = arg->list; e != NULL; e = e->next)
            {  xassert(e->x != NULL);
               xassert(e->x->up == NULL);
               e->x->up = code;
               code->vflag |= e->x->vflag;
            }
            code->arg.list = arg->list;
            break;
         case O_SUM:
         case O_PROD:
         case O_MINIMUM:
         case O_MAXIMUM:
         case O_FORALL:
         case O_EXISTS:
         case O_SETOF:
         case O_BUILD:
            /* iterated operation */
            domain = arg->loop.domain;
            xassert(domain != NULL);
            if (domain->code != NULL)
            {  xassert(domain->code->up == NULL);
               domain->code->up = code;
               code->vflag |= domain->code->vflag;
            }
            for (block = domain->list; block != NULL; block =
               block->next)
            {  xassert(block->code != NULL);
               xassert(block->code->up == NULL);
               block->code->up = code;
               code->vflag |= block->code->vflag;
            }
            if (arg->loop.x != NULL)
            {  xassert(arg->loop.x->up == NULL);
               arg->loop.x->up = code;
               code->vflag |= arg->loop.x->vflag;
            }
            code->arg.loop.domain = arg->loop.domain;
            code->arg.loop.x = arg->loop.x;
            break;
         case O_VERSION: /* nothing to do */
             break;
         default:
            xassert(op != op);
      }
      /* set other attributes of the pseudo-code */
      code->type = type;
      code->dim = dim;
      code->up = NULL;
      code->valid = 0;
      memset(&code->value, '?', sizeof(VALUE));
      return code;
}

/*----------------------------------------------------------------------
-- make_unary - generate pseudo-code for unary operation.
--
-- This routine generates pseudo-code for unary operation. */

CODE *make_unary(MPL *mpl, int op, CODE *x, int type, int dim)
{     CODE *code;
      OPERANDS arg;
      xassert(x != NULL);
      arg.arg.x = x;
      code = make_code(mpl, op, &arg, type, dim);
      return code;
}

/*----------------------------------------------------------------------
-- make_binary - generate pseudo-code for binary operation.
--
-- This routine generates pseudo-code for binary operation. */

CODE *make_binary(MPL *mpl, int op, CODE *x, CODE *y, int type,
      int dim)
{     CODE *code;
      OPERANDS arg;
      xassert(x != NULL);
      xassert(y != NULL);
      arg.arg.x = x;
      arg.arg.y = y;
      code = make_code(mpl, op, &arg, type, dim);
      return code;
}

/*----------------------------------------------------------------------
-- make_ternary - generate pseudo-code for ternary operation.
--
-- This routine generates pseudo-code for ternary operation. */

CODE *make_ternary(MPL *mpl, int op, CODE *x, CODE *y, CODE *z,
      int type, int dim)
{     CODE *code;
      OPERANDS arg;
      xassert(x != NULL);
      xassert(y != NULL);
      /* third operand can be NULL */
      arg.arg.x = x;
      arg.arg.y = y;
      arg.arg.z = z;
      code = make_code(mpl, op, &arg, type, dim);
      return code;
}

/*----------------------------------------------------------------------
-- numeric_literal - parse reference to numeric literal.
--
-- This routine parses primary expression using the syntax:
--
-- <primary expression> ::= <numeric literal> */

CODE *numeric_literal(MPL *mpl)
{     CODE *code;
      OPERANDS arg;
      xassert(mpl->scan_input->token == T_NUMBER);
      arg.num = mpl->scan_input->value;
      code = make_code(mpl, O_NUMBER, &arg, A_NUMERIC, 0);
      get_token(mpl /* <numeric literal> */);
      return code;
}

/*----------------------------------------------------------------------
-- string_literal - parse reference to string literal.
--
-- This routine parses primary expression using the syntax:
--
-- <primary expression> ::= <string literal> */

CODE *string_literal(MPL *mpl)
{     CODE *code;
      OPERANDS arg;
      xassert(mpl->scan_input->token == T_STRING);
      arg.str = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
      strcpy(arg.str, mpl->scan_input->image);
      code = make_code(mpl, O_STRING, &arg, A_SYMBOLIC, 0);
      get_token(mpl /* <string literal> */);
      return code;
}

/*----------------------------------------------------------------------
-- create_arg_list - create empty operands list.
--
-- This routine creates operands list, which is initially empty. */

ARG_LIST *create_arg_list(MPL *mpl)
{     ARG_LIST *list;
      xassert(mpl == mpl);
      list = NULL;
      return list;
}

/*----------------------------------------------------------------------
-- expand_arg_list - append operand to operands list.
--
-- This routine appends new operand to specified operands list. */

ARG_LIST *expand_arg_list(MPL *mpl, ARG_LIST *list, CODE *x)
{     ARG_LIST *tail, *temp;
      xassert(x != NULL);
      /* create new operands list entry */
      tail = alloc(ARG_LIST);
      tail->x = x;
      tail->next = NULL;
      /* and append it to the operands list */
      if (list == NULL)
         list = tail;
      else
      {  for (temp = list; temp->next != NULL; temp = temp->next);
         temp->next = tail;
      }
      return list;
}

/*----------------------------------------------------------------------
-- arg_list_len - determine length of operands list.
--
-- This routine returns the number of operands in operands list. */

int arg_list_len(MPL *mpl, ARG_LIST *list)
{     ARG_LIST *temp;
      int len;
      xassert(mpl == mpl);
      len = 0;
      for (temp = list; temp != NULL; temp = temp->next) len++;
      return len;
}

/*----------------------------------------------------------------------
-- subscript_list - parse subscript list.
--
-- This routine parses subscript list using the syntax:
--
-- <subscript list> ::= <subscript>
-- <subscript list> ::= <subscript list> , <subscript>
-- <subscript> ::= <expression 5> */

ARG_LIST *subscript_list(MPL *mpl)
{     ARG_LIST *list;
      CODE *x;
      list = create_arg_list(mpl);
      for (;;)
      {  /* parse subscript expression */
         x = expression_5(mpl);
         /* convert it to symbolic type, if necessary */
         if (x->type == A_NUMERIC)
            x = make_unary(mpl, O_CVTSYM, x, A_SYMBOLIC, 0);
         /* check that now the expression is of symbolic type */
         if (x->type != A_SYMBOLIC)
            error(mpl, "subscript expression has invalid type");
         xassert(x->dim == 0);
         /* and append it to the subscript list */
         list = expand_arg_list(mpl, list, x);
         /* check a token that follows the subscript expression */
         if (mpl->scan_input->token == T_COMMA)
            get_token(mpl /* , */);
         else if (mpl->scan_input->token == T_RBRACKET)
            break;
         else
            error(mpl, "syntax error in subscript list");
      }
      return list;
}

#if 1 /* 15/V-2010 */
/*----------------------------------------------------------------------
-- object_reference - parse reference to named object.
--
-- This routine parses primary expression using the syntax:
--
-- <primary expression> ::= <dummy index>
-- <primary expression> ::= <set name>
-- <primary expression> ::= <set name> [ <subscript list> ]
-- <primary expression> ::= <parameter name>
-- <primary expression> ::= <parameter name> [ <subscript list> ]
-- <primary expression> ::= <variable name> <suffix>
-- <primary expression> ::= <variable name> [ <subscript list> ]
--                          <suffix>
-- <primary expression> ::= <constraint name> <suffix>
-- <primary expression> ::= <constraint name> [ <subscript list> ]
--                          <suffix>
-- <dummy index> ::= <symbolic name>
-- <set name> ::= <symbolic name>
-- <parameter name> ::= <symbolic name>
-- <variable name> ::= <symbolic name>
-- <constraint name> ::= <symbolic name>
-- <suffix> ::= <empty> | .lb | .ub | .status | .val | .dual */

CODE *object_reference(MPL *mpl)
{     AVLNODE *node;
      DOMAIN_SLOT *slot;
      SET *set;
      PARAMETER *par;
      VARIABLE *var;
      CONSTRAINT *con;
      ARG_LIST *list;
      OPERANDS arg;
      CODE *code;
      char *name;
      int dim, suff;
      /* find the object in the symbolic name table */
      xassert(mpl->scan_input->token == T_NAME);
      node = avl_find_node(mpl->tree, mpl->scan_input->image);
      if (node == NULL)
         error(mpl, "%s not defined", mpl->scan_input->image);
      /* check the object type and obtain its dimension */
      switch (avl_get_node_type(node))
      {  case A_INDEX:
            /* dummy index */
            slot = (DOMAIN_SLOT *)avl_get_node_link(node);
            name = slot->name;
            dim = 0;
            break;
         case A_SET:
            /* model set */
            set = (SET *)avl_get_node_link(node);
            name = set->name;
            dim = set->dim;
            /* if a set object is referenced in its own declaration and
               the dimen attribute is not specified yet, use dimen 1 by
               default */
            if (set->dimen == 0) set->dimen = 1;
            break;
         case A_PARAMETER:
            /* model parameter */
            par = (PARAMETER *)avl_get_node_link(node);
            name = par->name;
            dim = par->dim;
            break;
         case A_VARIABLE:
            /* model variable */
            var = (VARIABLE *)avl_get_node_link(node);
            name = var->name;
            dim = var->dim;
            break;
         case A_CONSTRAINT:
            /* model constraint or objective */
            con = (CONSTRAINT *)avl_get_node_link(node);
            name = con->name;
            dim = con->dim;
            break;
         default:
            xassert(node != node);
      }
      get_token(mpl /* <symbolic name> */);
      /* parse optional subscript list */
      if (mpl->scan_input->token == T_LBRACKET)
      {  /* subscript list is specified */
         if (dim == 0)
            error(mpl, "%s cannot be subscripted", name);
         get_token(mpl /* [ */);
         list = subscript_list(mpl);
         if (dim != arg_list_len(mpl, list))
            error(mpl, "%s must have %d subscript%s rather than %d",
               name, dim, dim == 1 ? "" : "s", arg_list_len(mpl, list));
         xassert(mpl->scan_input->token == T_RBRACKET);
         get_token(mpl /* ] */);
      }
      else
      {  /* subscript list is not specified */
         if (dim != 0)
            error(mpl, "%s must be subscripted", name);
         list = create_arg_list(mpl);
      }
      /* parse optional suffix */
      if (!mpl->flag_s && avl_get_node_type(node) == A_VARIABLE)
         suff = DOT_NONE;
      else
         suff = DOT_VAL;
      if (mpl->scan_input->token == T_POINT)
      {  get_token(mpl /* . */);
         if (mpl->scan_input->token != T_NAME)
            error(mpl, "invalid use of period");
         if (!(avl_get_node_type(node) == A_VARIABLE ||
               avl_get_node_type(node) == A_CONSTRAINT))
            error(mpl, "%s cannot have a suffix", name);
         if (strcmp(mpl->scan_input->image, "lb") == 0)
            suff = DOT_LB;
         else if (strcmp(mpl->scan_input->image, "ub") == 0)
            suff = DOT_UB;
         else if (strcmp(mpl->scan_input->image, "status") == 0)
            suff = DOT_STATUS;
         else if (strcmp(mpl->scan_input->image, "val") == 0)
            suff = DOT_VAL;
         else if (strcmp(mpl->scan_input->image, "dual") == 0)
            suff = DOT_DUAL;
         else
            error(mpl, "suffix .%s invalid", mpl->scan_input->image);
         get_token(mpl /* suffix */);
      }
      /* generate pseudo-code to take value of the object */
      switch (avl_get_node_type(node))
      {  case A_INDEX:
            arg.index.slot = slot;
            arg.index.next = slot->list;
            code = make_code(mpl, O_INDEX, &arg, A_SYMBOLIC, 0);
            slot->list = code;
            break;
         case A_SET:
            arg.set.set = set;
            arg.set.list = list;
            arg.set.elemset = NULL;
            code = make_code(mpl, O_MEMSET, &arg, A_ELEMSET,
               set->dimen);
            if(set->scoped)
                code->vflag = 1;
            break;
         case A_PARAMETER:
            arg.par.par = par;
            arg.par.list = list;
            if (par->type == A_SYMBOLIC)
               code = make_code(mpl, O_MEMSYM, &arg, A_SYMBOLIC, 0);
            else
               code = make_code(mpl, O_MEMNUM, &arg, A_NUMERIC, 0);
            if(par->scoped)
                code->vflag = 1;
            break;
         case A_VARIABLE:
            if (!mpl->flag_s && (suff == DOT_STATUS || suff == DOT_VAL
               || suff == DOT_DUAL))
               error(mpl, "invalid reference to status, primal value, o"
                  "r dual value of variable %s above solve statement",
                  var->name);
            arg.var.var = var;
            arg.var.list = list;
            arg.var.suff = suff;
            code = make_code(mpl, O_MEMVAR, &arg, suff == DOT_NONE ?
               A_FORMULA : A_NUMERIC, 0);
            break;
         case A_CONSTRAINT:
            if (!mpl->flag_s && (suff == DOT_STATUS || suff == DOT_VAL
               || suff == DOT_DUAL))
               error(mpl, "invalid reference to status, primal value, o"
                  "r dual value of %s %s above solve statement",
                  con->type == A_CONSTRAINT ? "constraint" : "objective"
                  , con->name);
            arg.con.con = con;
            arg.con.list = list;
            arg.con.suff = suff;
            code = make_code(mpl, O_MEMCON, &arg, A_NUMERIC, 0);
            break;
         default:
            xassert(node != node);
      }
      return code;
}
#endif

/*----------------------------------------------------------------------
-- numeric_argument - parse argument passed to built-in function.
--
-- This routine parses an argument passed to numeric built-in function
-- using the syntax:
--
-- <arg> ::= <expression 5> */

CODE *numeric_argument(MPL *mpl, char *func)
{     CODE *x;
      x = expression_5(mpl);
      /* convert the argument to numeric type, if necessary */
      if (x->type == A_SYMBOLIC)
         x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
      /* check that now the argument is of numeric type */
      if (x->type != A_NUMERIC)
         error(mpl, "argument for %s has invalid type", func);
      xassert(x->dim == 0);
      return x;
}

#if 1 /* 15/VII-2006 */
CODE *symbolic_argument(MPL *mpl, char *func)
{     CODE *x;
      x = expression_5(mpl);
      /* convert the argument to symbolic type, if necessary */
      if (x->type == A_NUMERIC)
         x = make_unary(mpl, O_CVTSYM, x, A_SYMBOLIC, 0);
      /* check that now the argument is of symbolic type */
      if (x->type != A_SYMBOLIC)
         error(mpl, "argument for %s has invalid type", func);
      xassert(x->dim == 0);
      return x;
}
#endif

#if 1 /* 15/VII-2006 */
CODE *elemset_argument(MPL *mpl, char *func)
{     CODE *x;
      x = expression_9(mpl);
      if (x->type != A_ELEMSET)
         error(mpl, "argument for %s has invalid type", func);
      xassert(x->dim > 0);
      return x;
}
#endif

/*----------------------------------------------------------------------
-- function_reference - parse reference to built-in function.
--
-- This routine parses primary expression using the syntax:
--
-- <primary expression> ::= abs ( <arg> )
-- <primary expression> ::= ceil ( <arg> )
-- <primary expression> ::= floor ( <arg> )
-- <primary expression> ::= exp ( <arg> )
-- <primary expression> ::= log ( <arg> )
-- <primary expression> ::= log10 ( <arg> )
-- <primary expression> ::= max ( <arg list> )
-- <primary expression> ::= min ( <arg list> )
-- <primary expression> ::= sqrt ( <arg> )
-- <primary expression> ::= sin ( <arg> )
-- <primary expression> ::= cos ( <arg> )
-- <primary expression> ::= tan ( <arg> )
-- <primary expression> ::= atan ( <arg> )
-- <primary expression> ::= atan2 ( <arg> , <arg> )
-- <primary expression> ::= round ( <arg> )
-- <primary expression> ::= round ( <arg> , <arg> )
-- <primary expression> ::= trunc ( <arg> )
-- <primary expression> ::= trunc ( <arg> , <arg> )
-- <primary expression> ::= Irand224 ( )
-- <primary expression> ::= Uniform01 ( )
-- <primary expression> ::= Uniform ( <arg> , <arg> )
-- <primary expression> ::= Normal01 ( )
-- <primary expression> ::= Normal ( <arg> , <arg> )
-- <primary expression> ::= card ( <arg> )
-- <primary expression> ::= length ( <arg> )
-- <primary expression> ::= substr ( <arg> , <arg> )
-- <primary expression> ::= substr ( <arg> , <arg> , <arg> )
-- <primary expression> ::= str2time ( <arg> , <arg> )
-- <primary expression> ::= time2str ( <arg> , <arg> )
-- <primary expression> ::= gmtime ( )
-- <arg list> ::= <arg>
-- <arg list> ::= <arg list> , <arg> */

CODE *function_reference(MPL *mpl)
{     CODE *code;
      OPERANDS arg;
      int op;
      char func[15+1];
      /* determine operation code */
      xassert(mpl->scan_input->token == T_NAME);
      if (strcmp(mpl->scan_input->image, "abs") == 0)
         op = O_ABS;
      else if (strcmp(mpl->scan_input->image, "ceil") == 0)
         op = O_CEIL;
      else if (strcmp(mpl->scan_input->image, "floor") == 0)
         op = O_FLOOR;
      else if (strcmp(mpl->scan_input->image, "exp") == 0)
         op = O_EXP;
      else if (strcmp(mpl->scan_input->image, "log") == 0)
         op = O_LOG;
      else if (strcmp(mpl->scan_input->image, "log10") == 0)
         op = O_LOG10;
      else if (strcmp(mpl->scan_input->image, "sqrt") == 0)
         op = O_SQRT;
      else if (strcmp(mpl->scan_input->image, "sin") == 0)
         op = O_SIN;
      else if (strcmp(mpl->scan_input->image, "cos") == 0)
         op = O_COS;
      else if (strcmp(mpl->scan_input->image, "tan") == 0)
         op = O_TAN;
      else if (strcmp(mpl->scan_input->image, "atan") == 0)
         op = O_ATAN;
      else if (strcmp(mpl->scan_input->image, "min") == 0)
         op = O_MIN;
      else if (strcmp(mpl->scan_input->image, "max") == 0)
         op = O_MAX;
      else if (strcmp(mpl->scan_input->image, "round") == 0)
         op = O_ROUND;
      else if (strcmp(mpl->scan_input->image, "trunc") == 0)
         op = O_TRUNC;
      else if (strcmp(mpl->scan_input->image, "Irand224") == 0)
         op = O_IRAND224;
      else if (strcmp(mpl->scan_input->image, "Uniform01") == 0)
         op = O_UNIFORM01;
      else if (strcmp(mpl->scan_input->image, "Uniform") == 0)
         op = O_UNIFORM;
      else if (strcmp(mpl->scan_input->image, "Normal01") == 0)
         op = O_NORMAL01;
      else if (strcmp(mpl->scan_input->image, "Normal") == 0)
         op = O_NORMAL;
      else if (strcmp(mpl->scan_input->image, "card") == 0)
         op = O_CARD;
      else if (strcmp(mpl->scan_input->image, "length") == 0)
         op = O_LENGTH;
      else if (strcmp(mpl->scan_input->image, "substr") == 0)
         op = O_SUBSTR;
      else if (strcmp(mpl->scan_input->image, "str2time") == 0)
         op = O_STR2TIME;
      else if (strcmp(mpl->scan_input->image, "time2str") == 0)
         op = O_TIME2STR;
      else if (strcmp(mpl->scan_input->image, "gmtime") == 0)
         op = O_GMTIME;
      else if (strcmp(mpl->scan_input->image, "version") == 0)
         op = O_VERSION;
      else
         error(mpl, "function %s unknown", mpl->scan_input->image);
      /* save symbolic name of the function */
      strcpy(func, mpl->scan_input->image);
      xassert(strlen(func) < sizeof(func));
      get_token(mpl /* <symbolic name> */);
      /* check the left parenthesis that follows the function name */
      xassert(mpl->scan_input->token == T_LEFT);
      get_token(mpl /* ( */);
      /* parse argument list */
      if (op == O_MIN || op == O_MAX)
      {  /* min and max allow arbitrary number of arguments */
         arg.list = create_arg_list(mpl);
         /* parse argument list */
         for (;;)
         {  /* parse argument and append it to the operands list */
            arg.list = expand_arg_list(mpl, arg.list,
               numeric_argument(mpl, func));
            /* check a token that follows the argument */
            if (mpl->scan_input->token == T_COMMA)
               get_token(mpl /* , */);
            else if (mpl->scan_input->token == T_RIGHT)
               break;
            else
               error(mpl, "syntax error in argument list for %s", func);
         }
      }
      else if (op == O_IRAND224 || op == O_UNIFORM01 || op ==
         O_NORMAL01 || op == O_GMTIME || op == O_VERSION)
      {  /* Irand224, Uniform01, Normal01, gmtime need no arguments */
         if (mpl->scan_input->token != T_RIGHT)
            error(mpl, "%s needs no arguments", func);
      }
      else if (op == O_UNIFORM || op == O_NORMAL)
      {  /* Uniform and Normal need two arguments */
         /* parse the first argument */
         arg.arg.x = numeric_argument(mpl, func);
         /* check a token that follows the first argument */
         if (mpl->scan_input->token == T_COMMA)
            ;
         else if (mpl->scan_input->token == T_RIGHT)
            error(mpl, "%s needs two arguments", func);
         else
            error(mpl, "syntax error in argument for %s", func);
         get_token(mpl /* , */);
         /* parse the second argument */
         arg.arg.y = numeric_argument(mpl, func);
         /* check a token that follows the second argument */
         if (mpl->scan_input->token == T_COMMA)
            error(mpl, "%s needs two argument", func);
         else if (mpl->scan_input->token == T_RIGHT)
            ;
         else
            error(mpl, "syntax error in argument for %s", func);
      }
      else if (op == O_ATAN || op == O_ROUND || op == O_TRUNC)
      {  /* atan, round, and trunc need one or two arguments */
         /* parse the first argument */
         arg.arg.x = numeric_argument(mpl, func);
         /* parse the second argument, if specified */
         if (mpl->scan_input->token == T_COMMA)
         {  switch (op)
            {  case O_ATAN:  op = O_ATAN2;  break;
               case O_ROUND: op = O_ROUND2; break;
               case O_TRUNC: op = O_TRUNC2; break;
               default: xassert(op != op);
            }
            get_token(mpl /* , */);
            arg.arg.y = numeric_argument(mpl, func);
         }
         /* check a token that follows the last argument */
         if (mpl->scan_input->token == T_COMMA)
            error(mpl, "%s needs one or two arguments", func);
         else if (mpl->scan_input->token == T_RIGHT)
            ;
         else
            error(mpl, "syntax error in argument for %s", func);
      }
      else if (op == O_SUBSTR)
      {  /* substr needs two or three arguments */
         /* parse the first argument */
         arg.arg.x = symbolic_argument(mpl, func);
         /* check a token that follows the first argument */
         if (mpl->scan_input->token == T_COMMA)
            ;
         else if (mpl->scan_input->token == T_RIGHT)
            error(mpl, "%s needs two or three arguments", func);
         else
            error(mpl, "syntax error in argument for %s", func);
         get_token(mpl /* , */);
         /* parse the second argument */
         arg.arg.y = numeric_argument(mpl, func);
         /* parse the third argument, if specified */
         if (mpl->scan_input->token == T_COMMA)
         {  op = O_SUBSTR3;
            get_token(mpl /* , */);
            arg.arg.z = numeric_argument(mpl, func);
         }
         /* check a token that follows the last argument */
         if (mpl->scan_input->token == T_COMMA)
            error(mpl, "%s needs two or three arguments", func);
         else if (mpl->scan_input->token == T_RIGHT)
            ;
         else
            error(mpl, "syntax error in argument for %s", func);
      }
      else if (op == O_STR2TIME)
      {  /* str2time needs two arguments, both symbolic */
         /* parse the first argument */
         arg.arg.x = symbolic_argument(mpl, func);
         /* check a token that follows the first argument */
         if (mpl->scan_input->token == T_COMMA)
            ;
         else if (mpl->scan_input->token == T_RIGHT)
            error(mpl, "%s needs two arguments", func);
         else
            error(mpl, "syntax error in argument for %s", func);
         get_token(mpl /* , */);
         /* parse the second argument */
         arg.arg.y = symbolic_argument(mpl, func);
         /* check a token that follows the second argument */
         if (mpl->scan_input->token == T_COMMA)
            error(mpl, "%s needs two argument", func);
         else if (mpl->scan_input->token == T_RIGHT)
            ;
         else
            error(mpl, "syntax error in argument for %s", func);
      }
      else if (op == O_TIME2STR)
      {  /* time2str needs two arguments, numeric and symbolic */
         /* parse the first argument */
         arg.arg.x = numeric_argument(mpl, func);
         /* check a token that follows the first argument */
         if (mpl->scan_input->token == T_COMMA)
            ;
         else if (mpl->scan_input->token == T_RIGHT)
            error(mpl, "%s needs two arguments", func);
         else
            error(mpl, "syntax error in argument for %s", func);
         get_token(mpl /* , */);
         /* parse the second argument */
         arg.arg.y = symbolic_argument(mpl, func);
         /* check a token that follows the second argument */
         if (mpl->scan_input->token == T_COMMA)
            error(mpl, "%s needs two argument", func);
         else if (mpl->scan_input->token == T_RIGHT)
            ;
         else
            error(mpl, "syntax error in argument for %s", func);
      }
      else
      {  /* other functions need one argument */
         if (op == O_CARD)
            arg.arg.x = elemset_argument(mpl, func);
         else if (op == O_LENGTH)
            arg.arg.x = symbolic_argument(mpl, func);
         else
            arg.arg.x = numeric_argument(mpl, func);
         /* check a token that follows the argument */
         if (mpl->scan_input->token == T_COMMA)
            error(mpl, "%s needs one argument", func);
         else if (mpl->scan_input->token == T_RIGHT)
            ;
         else
            error(mpl, "syntax error in argument for %s", func);
      }
      /* make pseudo-code to call the built-in function */
      if (op == O_SUBSTR || op == O_SUBSTR3 || op == O_TIME2STR || op == O_VERSION)
         code = make_code(mpl, op, &arg, A_SYMBOLIC, 0);
      else
         code = make_code(mpl, op, &arg, A_NUMERIC, 0);
      /* the reference ends with the right parenthesis */
      xassert(mpl->scan_input->token == T_RIGHT);
      get_token(mpl /* ) */);
      return code;
}

/*----------------------------------------------------------------------
-- create_domain - create empty domain.
--
-- This routine creates empty domain, which is initially empty, i.e.
-- has no domain blocks. */

DOMAIN *create_domain(MPL *mpl)
{     DOMAIN *domain;
      domain = alloc(DOMAIN);
      domain->list = NULL;
      domain->code = NULL;
      return domain;
}

/*----------------------------------------------------------------------
-- create_block - create empty domain block.
--
-- This routine creates empty domain block, which is initially empty,
-- i.e. has no domain slots. */

DOMAIN_BLOCK *create_block(MPL *mpl)
{     DOMAIN_BLOCK *block;
      block = alloc(DOMAIN_BLOCK);
      block->list = NULL;
      block->code = NULL;
      block->backup = NULL;
      block->next = NULL;
      return block;
}

/*----------------------------------------------------------------------
-- append_block - append domain block to specified domain.
--
-- This routine adds given domain block to the end of the block list of
-- specified domain. */

void append_block(MPL *mpl, DOMAIN *domain, DOMAIN_BLOCK *block)
{     DOMAIN_BLOCK *temp;
      xassert(mpl == mpl);
      xassert(domain != NULL);
      xassert(block != NULL);
      xassert(block->next == NULL);
      if (domain->list == NULL)
         domain->list = block;
      else
      {  for (temp = domain->list; temp->next != NULL; temp =
            temp->next);
         temp->next = block;
      }
      return;
}

/*----------------------------------------------------------------------
-- append_slot - create and append new slot to domain block.
--
-- This routine creates new domain slot and adds it to the end of slot
-- list of specified domain block.
--
-- The parameter name is symbolic name of the dummy index associated
-- with the slot (the character string must be allocated). NULL means
-- the dummy index is not explicitly specified.
--
-- The parameter code is pseudo-code for computing symbolic value, at
-- which the dummy index is bounded. NULL means the dummy index is free
-- in the domain scope. */

DOMAIN_SLOT *append_slot(MPL *mpl, DOMAIN_BLOCK *block, char *name,
      CODE *code)
{     DOMAIN_SLOT *slot, *temp;
      xassert(block != NULL);
      slot = alloc(DOMAIN_SLOT);
      slot->name = name;
      slot->code = code;
      slot->value.sym = nanbox_null();
      slot->list = NULL;
      slot->next = NULL;
      if (block->list == NULL)
         block->list = slot;
      else
      {  for (temp = block->list; temp->next != NULL; temp =
            temp->next);
         temp->next = slot;
      }
      return slot;
}

/*----------------------------------------------------------------------
-- expression_list - parse expression list.
--
-- This routine parses a list of one or more expressions enclosed into
-- the parentheses using the syntax:
--
-- <primary expression> ::= ( <expression list> )
-- <expression list> ::= <expression 13>
-- <expression list> ::= <expression 13> , <expression list>
--
-- Note that this construction may have three different meanings:
--
-- 1. If <expression list> consists of only one expression, <primary
--    expression> is a parenthesized expression, which may be of any
--    valid type (not necessarily 1-tuple).
--
-- 2. If <expression list> consists of several expressions separated by
--    commae, where no expression is undeclared symbolic name, <primary
--    expression> is a n-tuple.
--
-- 3. If <expression list> consists of several expressions separated by
--    commae, where at least one expression is undeclared symbolic name
--    (that denotes a dummy index), <primary expression> is a slice and
--    can be only used as constituent of indexing expression. */

#define max_dim 20
/* maximal number of components allowed within parentheses */

CODE *expression_list(MPL *mpl)
{     CODE *code;
      OPERANDS arg;
      struct { char *name; CODE *code; } list[1+max_dim];
      int flag_x, next_token, dim, j, slice = 0;
      xassert(mpl->scan_input->token == T_LEFT);
      /* the flag, which allows recognizing undeclared symbolic names
         as dummy indices, will be automatically reset by get_token(),
         so save it before scanning the next token */
      flag_x = mpl->flag_x;
      get_token(mpl /* ( */);
      /* parse <expression list> */
      for (dim = 1; ; dim++)
      {  if (dim > max_dim)
            error(mpl, "too many components within parentheses");
         /* current component of <expression list> can be either dummy
            index or expression */
         if (mpl->scan_input->token == T_NAME)
         {  /* symbolic name is recognized as dummy index only if:
               the flag, which allows that, is set, and
               the name is followed by comma or right parenthesis, and
               the name is undeclared */
            get_token(mpl /* <symbolic name> */);
            next_token = mpl->scan_input->token;
            unget_token(mpl);
            if (!(flag_x &&
                  (next_token == T_COMMA || next_token == T_RIGHT) &&
                  avl_find_node(mpl->tree, mpl->scan_input->image) == NULL))
            {  /* this is not dummy index */
               goto expr;
            }
            /* all dummy indices within the same slice must have unique
               symbolic names */
            for (j = 1; j < dim; j++)
            {  if (list[j].name != NULL && strcmp(list[j].name,
                  mpl->scan_input->image) == 0)
                  error(mpl, "duplicate dummy index %s not allowed",
                     mpl->scan_input->image);
            }
            /* current component of <expression list> is dummy index */
            list[dim].name
               = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
            strcpy(list[dim].name, mpl->scan_input->image);
            list[dim].code = NULL;
            get_token(mpl /* <symbolic name> */);
            /* <expression list> is a slice, because at least one dummy
               index has appeared */
            slice = 1;
            /* note that the context ( <dummy index> ) is not allowed,
               i.e. in this case <primary expression> is considered as
               a parenthesized expression */
            if (dim == 1 && mpl->scan_input->token == T_RIGHT)
               error(mpl, "%s not defined", list[dim].name);
         }
         else
expr:    {  /* current component of <expression list> is expression */
            code = expression_13(mpl);
            /* if the current expression is followed by comma or it is
               not the very first expression, entire <expression list>
               is n-tuple or slice, in which case the current expression
               should be converted to symbolic type, if necessary */
            if (mpl->scan_input->token == T_COMMA || dim > 1)
            {  if (code->type == A_NUMERIC)
                  code = make_unary(mpl, O_CVTSYM, code, A_SYMBOLIC, 0);
               /* now the expression must be of symbolic type */
               if (code->type != A_SYMBOLIC)
                  error(mpl, "component expression has invalid type");
               xassert(code->dim == 0);
            }
            list[dim].name = NULL;
            list[dim].code = code;
         }
         /* check a token that follows the current component */
         if (mpl->scan_input->token == T_COMMA)
            get_token(mpl /* , */);
         else if (mpl->scan_input->token == T_RIGHT)
            break;
         else
            error(mpl, "right parenthesis missing where expected");
      }
      /* generate pseudo-code for <primary expression> */
      if (dim == 1 && !slice)
      {  /* <primary expression> is a parenthesized expression */
         code = list[1].code;
      }
      else if (!slice)
      {  /* <primary expression> is a n-tuple */
         arg.list = create_arg_list(mpl);
         for (j = 1; j <= dim; j++)
            arg.list = expand_arg_list(mpl, arg.list, list[j].code);
         code = make_code(mpl, O_TUPLE, &arg, A_TUPLE, dim);
      }
      else
      {  /* <primary expression> is a slice */
         arg.slice = create_block(mpl);
         for (j = 1; j <= dim; j++)
            append_slot(mpl, arg.slice, list[j].name, list[j].code);
         /* note that actually pseudo-codes with op = O_SLICE are never
            evaluated */
         code = make_code(mpl, O_SLICE, &arg, A_TUPLE, dim);
      }
      get_token(mpl /* ) */);
      /* if <primary expression> is a slice, there must be the keyword
         'in', which follows the right parenthesis */
      if (slice && mpl->scan_input->token != T_IN)
         error(mpl, "keyword in missing where expected");
      /* if the slice flag is set and there is the keyword 'in', which
         follows <primary expression>, the latter must be a slice */
      if (flag_x && mpl->scan_input->token == T_IN && !slice)
      {  if (dim == 1)
            error(mpl, "syntax error in indexing expression");
         else
            error(mpl, "0-ary slice not allowed");
      }
      return code;
}

/*----------------------------------------------------------------------
-- literal set - parse literal set.
--
-- This routine parses literal set using the syntax:
--
-- <literal set> ::= { <member list> }
-- <member list> ::= <member expression>
-- <member list> ::= <member list> , <member expression>
-- <member expression> ::= <expression 5>
--
-- It is assumed that the left curly brace and the very first member
-- expression that follows it are already parsed. The right curly brace
-- remains unscanned on exit. */

CODE *literal_set(MPL *mpl, CODE *code)
{     OPERANDS arg;
      int j;
      xassert(code != NULL);
      arg.list = create_arg_list(mpl);
      /* parse <member list> */
      for (j = 1; ; j++)
      {  /* all member expressions must be n-tuples; so, if the current
            expression is not n-tuple, convert it to 1-tuple */
         if (code->type == A_NUMERIC)
            code = make_unary(mpl, O_CVTSYM, code, A_SYMBOLIC, 0);
         if (code->type == A_SYMBOLIC)
            code = make_unary(mpl, O_CVTTUP, code, A_TUPLE, 1);
         /* now the expression must be n-tuple */
         if (code->type != A_TUPLE)
            error(mpl, "member expression has invalid type");
         /* all member expressions must have identical dimension */
         if (arg.list != NULL && arg.list->x->dim != code->dim)
            error(mpl, "member %d has %d component%s while member %d ha"
               "s %d component%s",
               j-1, arg.list->x->dim, arg.list->x->dim == 1 ? "" : "s",
               j, code->dim, code->dim == 1 ? "" : "s");
         /* append the current expression to the member list */
         arg.list = expand_arg_list(mpl, arg.list, code);
         /* check a token that follows the current expression */
         if (mpl->scan_input->token == T_COMMA)
            get_token(mpl /* , */);
         else if (mpl->scan_input->token == T_RBRACE)
            break;
         else
            error(mpl, "syntax error in literal set");
         /* parse the next expression that follows the comma */
         code = expression_5(mpl);
      }
      /* generate pseudo-code for <literal set> */
      code = make_code(mpl, O_MAKE, &arg, A_ELEMSET, arg.list->x->dim);
      return code;
}

/*----------------------------------------------------------------------
-- indexing_expression - parse indexing expression.
--
-- This routine parses indexing expression using the syntax:
--
-- <indexing expression> ::= <literal set>
-- <indexing expression> ::= { <indexing list> }
-- <indexing expression> ::= { <indexing list> : <logical expression> }
-- <indexing list> ::= <indexing element>
-- <indexing list> ::= <indexing list> , <indexing element>
-- <indexing element> ::= <basic expression>
-- <indexing element> ::= <dummy index> in <basic expression>
-- <indexing element> ::= <slice> in <basic expression>
-- <dummy index> ::= <symbolic name>
-- <slice> ::= ( <expression list> )
-- <basic expression> ::= <expression 9>
-- <logical expression> ::= <expression 13>
--
-- This routine creates domain for <indexing expression>, where each
-- domain block corresponds to <indexing element>, and each domain slot
-- corresponds to individual indexing position. */

DOMAIN *indexing_expression(MPL *mpl)
{     DOMAIN *domain;
      DOMAIN_BLOCK *block;
      DOMAIN_SLOT *slot;
      CODE *code;
      xassert(mpl->scan_input->token == T_LBRACE);
      get_token(mpl /* { */);
      if (mpl->scan_input->token == T_RBRACE)
         error(mpl, "empty indexing expression not allowed");
      /* create domain to be constructed */
      domain = create_domain(mpl);
      /* parse either <member list> or <indexing list> that follows the
         left brace */
      for (;;)
      {  /* domain block for <indexing element> is not created yet */
         block = NULL;
         /* pseudo-code for <basic expression> is not generated yet */
         code = NULL;
         /* check a token, which <indexing element> begins with */
         if (mpl->scan_input->token == T_NAME)
         {  /* it is a symbolic name */
            int next_token;
            char *name;
            /* symbolic name is recognized as dummy index only if it is
               followed by the keyword 'in' and not declared */
            get_token(mpl /* <symbolic name> */);
            next_token = mpl->scan_input->token;
            unget_token(mpl);
            if (!(next_token == T_IN &&
                  avl_find_node(mpl->tree, mpl->scan_input->image) == NULL))
            {  /* this is not dummy index; the symbolic name begins an
                  expression, which is either <basic expression> or the
                  very first <member expression> in <literal set> */
               goto expr;
            }
            /* create domain block with one slot, which is assigned the
               dummy index */
            block = create_block(mpl);
            name = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
            strcpy(name, mpl->scan_input->image);
            append_slot(mpl, block, name, NULL);
            get_token(mpl /* <symbolic name> */);
            /* the keyword 'in' is already checked above */
            xassert(mpl->scan_input->token == T_IN);
            get_token(mpl /* in */);
            /* <basic expression> that follows the keyword 'in' will be
               parsed below */
         }
         else if (mpl->scan_input->token == T_LEFT)
         {  /* it is the left parenthesis; parse expression that begins
               with this parenthesis (the flag is set in order to allow
               recognizing slices; see the routine expression_list) */
            mpl->flag_x = 1;
            code = expression_9(mpl);
            if (code->op != O_SLICE)
            {  /* this is either <basic expression> or the very first
                  <member expression> in <literal set> */
               goto expr;
            }
            /* this is a slice; besides the corresponding domain block
               is already created by expression_list() */
            block = code->arg.slice;
            code = NULL; /* <basic expression> is not parsed yet */
            /* the keyword 'in' following the slice is already checked
               by expression_list() */
            xassert(mpl->scan_input->token == T_IN);
            get_token(mpl /* in */);
            /* <basic expression> that follows the keyword 'in' will be
               parsed below */
         }
expr:    /* parse expression that follows either the keyword 'in' (in
            which case it can be <basic expression) or the left brace
            (in which case it can be <basic expression> as well as the
            very first <member expression> in <literal set>); note that
            this expression can be already parsed above */
         if (code == NULL) code = expression_9(mpl);
         /* check the type of the expression just parsed */
         if (code->type != A_ELEMSET)
         {  /* it is not <basic expression> and therefore it can only
               be the very first <member expression> in <literal set>;
               however, then there must be no dummy index neither slice
               between the left brace and this expression */
            if (block != NULL)
               error(mpl, "domain expression has invalid type");
            /* parse the rest part of <literal set> and make this set
               be <basic expression>, i.e. the construction {a, b, c}
               is parsed as it were written as {A}, where A = {a, b, c}
               is a temporary elemental set */
            code = literal_set(mpl, code);
         }
         /* now pseudo-code for <basic set> has been built */
         xassert(code != NULL);
         xassert(code->type == A_ELEMSET);
         xassert(code->dim > 0);
         /* if domain block for the current <indexing element> is still
            not created, create it for fake slice of the same dimension
            as <basic set> */
         if (block == NULL)
         {  int j;
            block = create_block(mpl);
            for (j = 1; j <= code->dim; j++)
               append_slot(mpl, block, NULL, NULL);
         }
         /* number of indexing positions in <indexing element> must be
            the same as dimension of n-tuples in basic set */
         {  int dim = 0;
            for (slot = block->list; slot != NULL; slot = slot->next)
               dim++;
            if (dim != code->dim)
               error(mpl,"%d %s specified for set of dimension %d",
                  dim, dim == 1 ? "index" : "indices", code->dim);
         }
         /* store pseudo-code for <basic set> in the domain block */
         xassert(block->code == NULL);
         block->code = code;
         /* and append the domain block to the domain */
         append_block(mpl, domain, block);
         /* the current <indexing element> has been completely parsed;
            include all its dummy indices into the symbolic name table
            to make them available for referencing from expressions;
            implicit declarations of dummy indices remain valid while
            the corresponding domain scope is valid */
         for (slot = block->list; slot != NULL; slot = slot->next)
         if (slot->name != NULL)
         {  AVLNODE *node;
            xassert(avl_find_node(mpl->tree, slot->name) == NULL);
            node = avl_insert_node(mpl->tree, slot->name);
            avl_set_node_type(node, A_INDEX);
            avl_set_node_link(node, (void *)slot);
         }
         /* check a token that follows <indexing element> */
         if (mpl->scan_input->token == T_COMMA)
            get_token(mpl /* , */);
         else if (mpl->scan_input->token == T_COLON || mpl->scan_input->token == T_RBRACE)
            break;
         else
            error(mpl, "syntax error in indexing expression");
      }
      /* parse <logical expression> that follows the colon */
      if (mpl->scan_input->token == T_COLON)
      {  get_token(mpl /* : */);
         code = expression_13(mpl);
         /* convert the expression to logical type, if necessary */
         if (code->type == A_SYMBOLIC)
            code = make_unary(mpl, O_CVTNUM, code, A_NUMERIC, 0);
         if (code->type == A_NUMERIC)
            code = make_unary(mpl, O_CVTLOG, code, A_LOGICAL, 0);
         /* now the expression must be of logical type */
         if (code->type != A_LOGICAL)
            error(mpl, "expression following colon has invalid type");
         xassert(code->dim == 0);
         domain->code = code;
         /* the right brace must follow the logical expression */
         if (mpl->scan_input->token != T_RBRACE)
            error(mpl, "syntax error in indexing expression");
      }
      get_token(mpl /* } */);
      return domain;
}

/*----------------------------------------------------------------------
-- close_scope - close scope of indexing expression.
--
-- The routine closes the scope of indexing expression specified by its
-- domain and thereby makes all dummy indices introduced in the indexing
-- expression no longer available for referencing. */

void close_scope(MPL *mpl, DOMAIN *domain)
{     DOMAIN_BLOCK *block;
      DOMAIN_SLOT *slot;
      AVLNODE *node;
      xassert(domain != NULL);
      /* remove all dummy indices from the symbolic names table */
      for (block = domain->list; block != NULL; block = block->next)
      {  for (slot = block->list; slot != NULL; slot = slot->next)
         {  if (slot->name != NULL)
            {  node = avl_find_node(mpl->tree, slot->name);
               xassert(node != NULL);
               xassert(avl_get_node_type(node) == A_INDEX);
               avl_delete_node(mpl->tree, node);
            }
         }
      }
      return;
}

/*----------------------------------------------------------------------
-- iterated_expression - parse iterated expression.
--
-- This routine parses primary expression using the syntax:
--
-- <primary expression> ::= <iterated expression>
-- <iterated expression> ::= sum <indexing expression> <expression 3>
-- <iterated expression> ::= prod <indexing expression> <expression 3>
-- <iterated expression> ::= min <indexing expression> <expression 3>
-- <iterated expression> ::= max <indexing expression> <expression 3>
-- <iterated expression> ::= exists <indexing expression>
--                           <expression 12>
-- <iterated expression> ::= forall <indexing expression>
--                           <expression 12>
-- <iterated expression> ::= setof <indexing expression> <expression 5>
--
-- Note that parsing "integrand" depends on the iterated operator. */

#if 1 /* 07/IX-2008 */
static void link_up(CODE *code)
{     /* if we have something like sum{(i+1,j,k-1) in E} x[i,j,k],
         where i and k are dummy indices defined out of the iterated
         expression, we should link up pseudo-code for computing i+1
         and k-1 to pseudo-code for computing the iterated expression;
         this is needed to invalidate current value of the iterated
         expression once i or k have been changed */
      DOMAIN_BLOCK *block;
      DOMAIN_SLOT *slot;
      for (block = code->arg.loop.domain->list; block != NULL;
         block = block->next)
      {  for (slot = block->list; slot != NULL; slot = slot->next)
         {  if (slot->code != NULL)
            {  xassert(slot->code->up == NULL);
               slot->code->up = code;
            }
         }
      }
      return;
}
#endif

CODE *iterated_expression(MPL *mpl)
{     CODE *code;
      OPERANDS arg;
      int op;
      char opstr[8];
      /* determine operation code */
      xassert(mpl->scan_input->token == T_NAME);
      if (strcmp(mpl->scan_input->image, "sum") == 0)
         op = O_SUM;
      else if (strcmp(mpl->scan_input->image, "prod") == 0)
         op = O_PROD;
      else if (strcmp(mpl->scan_input->image, "min") == 0)
         op = O_MINIMUM;
      else if (strcmp(mpl->scan_input->image, "max") == 0)
         op = O_MAXIMUM;
      else if (strcmp(mpl->scan_input->image, "forall") == 0)
         op = O_FORALL;
      else if (strcmp(mpl->scan_input->image, "exists") == 0)
         op = O_EXISTS;
      else if (strcmp(mpl->scan_input->image, "setof") == 0)
         op = O_SETOF;
      else
         error(mpl, "operator %s unknown", mpl->scan_input->image);
      strcpy(opstr, mpl->scan_input->image);
      xassert(strlen(opstr) < sizeof(opstr));
      get_token(mpl /* <symbolic name> */);
      /* check the left brace that follows the operator name */
      xassert(mpl->scan_input->token == T_LBRACE);
      /* parse indexing expression that controls iterating */
      arg.loop.domain = indexing_expression(mpl);
      /* parse "integrand" expression and generate pseudo-code */
      switch (op)
      {  case O_SUM:
         case O_PROD:
         case O_MINIMUM:
         case O_MAXIMUM:
            arg.loop.x = expression_3(mpl);
            /* convert the integrand to numeric type, if necessary */
            if (arg.loop.x->type == A_SYMBOLIC)
               arg.loop.x = make_unary(mpl, O_CVTNUM, arg.loop.x,
                  A_NUMERIC, 0);
            /* now the integrand must be of numeric type or linear form
               (the latter is only allowed for the sum operator) */
            if (!(arg.loop.x->type == A_NUMERIC ||
                  op == O_SUM && arg.loop.x->type == A_FORMULA))
err:           error(mpl, "integrand following %s{...} has invalid type"
                  , opstr);
            xassert(arg.loop.x->dim == 0);
            /* generate pseudo-code */
            code = make_code(mpl, op, &arg, arg.loop.x->type, 0);
            break;
         case O_FORALL:
         case O_EXISTS:
            arg.loop.x = expression_12(mpl);
            /* convert the integrand to logical type, if necessary */
            if (arg.loop.x->type == A_SYMBOLIC)
               arg.loop.x = make_unary(mpl, O_CVTNUM, arg.loop.x,
                  A_NUMERIC, 0);
            if (arg.loop.x->type == A_NUMERIC)
               arg.loop.x = make_unary(mpl, O_CVTLOG, arg.loop.x,
                  A_LOGICAL, 0);
            /* now the integrand must be of logical type */
            if (arg.loop.x->type != A_LOGICAL) goto err;
            xassert(arg.loop.x->dim == 0);
            /* generate pseudo-code */
            code = make_code(mpl, op, &arg, A_LOGICAL, 0);
            break;
         case O_SETOF:
            arg.loop.x = expression_5(mpl);
            /* convert the integrand to 1-tuple, if necessary */
            if (arg.loop.x->type == A_NUMERIC)
               arg.loop.x = make_unary(mpl, O_CVTSYM, arg.loop.x,
                  A_SYMBOLIC, 0);
            if (arg.loop.x->type == A_SYMBOLIC)
               arg.loop.x = make_unary(mpl, O_CVTTUP, arg.loop.x,
                  A_TUPLE, 1);
            /* now the integrand must be n-tuple */
            if (arg.loop.x->type != A_TUPLE) goto err;
            xassert(arg.loop.x->dim > 0);
            /* generate pseudo-code */
            code = make_code(mpl, op, &arg, A_ELEMSET, arg.loop.x->dim);
            break;
         default:
            xassert(op != op);
      }
      /* close the scope of the indexing expression */
      close_scope(mpl, arg.loop.domain);
#if 1 /* 07/IX-2008 */
      link_up(code);
#endif
      return code;
}

/*----------------------------------------------------------------------
-- domain_arity - determine arity of domain.
--
-- This routine returns arity of specified domain, which is number of
-- its free dummy indices. */

int domain_arity(MPL *mpl, DOMAIN *domain)
{     DOMAIN_BLOCK *block;
      DOMAIN_SLOT *slot;
      int arity;
      xassert(mpl == mpl);
      arity = 0;
      for (block = domain->list; block != NULL; block = block->next)
         for (slot = block->list; slot != NULL; slot = slot->next)
            if (slot->code == NULL) arity++;
      return arity;
}

/*----------------------------------------------------------------------
-- set_expression - parse set expression.
--
-- This routine parses primary expression using the syntax:
--
-- <primary expression> ::= { }
-- <primary expression> ::= <indexing expression> */

CODE *set_expression(MPL *mpl)
{     CODE *code;
      OPERANDS arg;
      xassert(mpl->scan_input->token == T_LBRACE);
      get_token(mpl /* { */);
      /* check a token that follows the left brace */
      if (mpl->scan_input->token == T_RBRACE)
      {  /* it is the right brace, so the resultant is an empty set of
            dimension 1 */
         arg.list = NULL;
         /* generate pseudo-code to build the resultant set */
         code = make_code(mpl, O_MAKE, &arg, A_ELEMSET, 1);
         get_token(mpl /* } */);
      }
      else
      {  /* the next token begins an indexing expression */
         unget_token(mpl);
         arg.loop.domain = indexing_expression(mpl);
         arg.loop.x = NULL; /* integrand is not used */
         /* close the scope of the indexing expression */
         close_scope(mpl, arg.loop.domain);
         /* generate pseudo-code to build the resultant set */
         code = make_code(mpl, O_BUILD, &arg, A_ELEMSET,
            domain_arity(mpl, arg.loop.domain));
#if 1 /* 07/IX-2008 */
         link_up(code);
#endif
      }
      return code;
}

/*----------------------------------------------------------------------
-- branched_expression - parse conditional expression.
--
-- This routine parses primary expression using the syntax:
--
-- <primary expression> ::= <branched expression>
-- <branched expression> ::= if <logical expression> then <expression 9>
-- <branched expression> ::= if <logical expression> then <expression 9>
--                           else <expression 9>
-- <logical expression> ::= <expression 13> */

CODE *branched_expression(MPL *mpl)
{     CODE *code, *x, *y, *z;
      xassert(mpl->scan_input->token == T_IF);
      get_token(mpl /* if */);
      /* parse <logical expression> that follows 'if' */
      x = expression_13(mpl);
      /* convert the expression to logical type, if necessary */
      if (x->type == A_SYMBOLIC)
         x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
      if (x->type == A_NUMERIC)
         x = make_unary(mpl, O_CVTLOG, x, A_LOGICAL, 0);
      /* now the expression must be of logical type */
      if (x->type != A_LOGICAL)
         error(mpl, "expression following if has invalid type");
      xassert(x->dim == 0);
      /* the keyword 'then' must follow the logical expression */
      if (mpl->scan_input->token != T_THEN)
         error(mpl, "keyword then missing where expected");
      get_token(mpl /* then */);
      /* parse <expression> that follows 'then' and check its type */
      y = expression_9(mpl);
      if (!(y->type == A_NUMERIC || y->type == A_SYMBOLIC ||
            y->type == A_ELEMSET || y->type == A_FORMULA))
         error(mpl, "expression following then has invalid type");
      /* if the expression that follows the keyword 'then' is elemental
         set, the keyword 'else' cannot be omitted; otherwise else-part
         is optional */
      if (mpl->scan_input->token != T_ELSE)
      {  if (y->type == A_ELEMSET)
            error(mpl, "keyword else missing where expected");
         z = NULL;
         goto skip;
      }
      get_token(mpl /* else */);
      /* parse <expression> that follow 'else' and check its type */
      z = expression_9(mpl);
      if (!(z->type == A_NUMERIC || z->type == A_SYMBOLIC ||
            z->type == A_ELEMSET || z->type == A_FORMULA))
         error(mpl, "expression following else has invalid type");
      /* convert to identical types, if necessary */
      if (y->type == A_FORMULA || z->type == A_FORMULA)
      {  if (y->type == A_SYMBOLIC)
            y = make_unary(mpl, O_CVTNUM, y, A_NUMERIC, 0);
         if (y->type == A_NUMERIC)
            y = make_unary(mpl, O_CVTLFM, y, A_FORMULA, 0);
         if (z->type == A_SYMBOLIC)
            z = make_unary(mpl, O_CVTNUM, z, A_NUMERIC, 0);
         if (z->type == A_NUMERIC)
            z = make_unary(mpl, O_CVTLFM, z, A_FORMULA, 0);
      }
      if (y->type == A_SYMBOLIC || z->type == A_SYMBOLIC)
      {  if (y->type == A_NUMERIC)
            y = make_unary(mpl, O_CVTSYM, y, A_SYMBOLIC, 0);
         if (z->type == A_NUMERIC)
            z = make_unary(mpl, O_CVTSYM, z, A_SYMBOLIC, 0);
      }
      /* now both expressions must have identical types */
      if (y->type != z->type)
         error(mpl, "expressions following then and else have incompati"
            "ble types");
      /* and identical dimensions */
      if (y->dim != z->dim)
         error(mpl, "expressions following then and else have different"
            " dimensions %d and %d, respectively", y->dim, z->dim);
skip: /* generate pseudo-code to perform branching */
      code = make_ternary(mpl, O_FORK, x, y, z, y->type, y->dim);
      return code;
}

/*----------------------------------------------------------------------
-- primary_expression - parse primary expression.
--
-- This routine parses primary expression using the syntax:
--
-- <primary expression> ::= <numeric literal>
-- <primary expression> ::= Infinity
-- <primary expression> ::= <string literal>
-- <primary expression> ::= <dummy index>
-- <primary expression> ::= <set name>
-- <primary expression> ::= <set name> [ <subscript list> ]
-- <primary expression> ::= <parameter name>
-- <primary expression> ::= <parameter name> [ <subscript list> ]
-- <primary expression> ::= <variable name>
-- <primary expression> ::= <variable name> [ <subscript list> ]
-- <primary expression> ::= <built-in function> ( <argument list> )
-- <primary expression> ::= ( <expression list> )
-- <primary expression> ::= <iterated expression>
-- <primary expression> ::= { }
-- <primary expression> ::= <indexing expression>
-- <primary expression> ::= <branched expression>
--
-- For complete list of syntactic rules for <primary expression> see
-- comments to the corresponding parsing routines. */

CODE *primary_expression(MPL *mpl)
{     CODE *code;
      if (mpl->scan_input->token == T_NUMBER)
      {  /* parse numeric literal */
         code = numeric_literal(mpl);
      }
#if 1 /* 21/VII-2006 */
      else if (mpl->scan_input->token == T_INFINITY)
      {  /* parse "infinity" */
         OPERANDS arg;
         arg.num = DBL_MAX;
         code = make_code(mpl, O_NUMBER, &arg, A_NUMERIC, 0);
         get_token(mpl /* Infinity */);
      }
#endif
      else if (mpl->scan_input->token == T_STRING)
      {  /* parse string literal */
         code = string_literal(mpl);
      }
      else if (mpl->scan_input->token == T_NAME)
      {  int next_token;
         get_token(mpl /* <symbolic name> */);
         next_token = mpl->scan_input->token;
         unget_token(mpl);
         /* check a token that follows <symbolic name> */
         switch (next_token)
         {  case T_LBRACKET:
               /* parse reference to subscripted object */
               code = object_reference(mpl);
               break;
            case T_LEFT:
               /* parse reference to built-in function */
               code = function_reference(mpl);
               break;
            case T_LBRACE:
               /* parse iterated expression */
               code = iterated_expression(mpl);
               break;
            default:
               /* parse reference to unsubscripted object */
               code = object_reference(mpl);
               break;
         }
      }
      else if (mpl->scan_input->token == T_LEFT)
      {  /* parse parenthesized expression */
         code = expression_list(mpl);
      }
      else if (mpl->scan_input->token == T_LBRACE)
      {  /* parse set expression */
         code = set_expression(mpl);
      }
      else if (mpl->scan_input->token == T_IF)
      {  /* parse conditional expression */
         code = branched_expression(mpl);
      }
      else if (is_reserved(mpl))
      {  /* other reserved keywords cannot be used here */
         error(mpl, "invalid use of reserved keyword %s", mpl->scan_input->image);
      }
      else
         error(mpl, "syntax error in expression");
      return code;
}

/*----------------------------------------------------------------------
-- error_preceding - raise error if preceding operand has wrong type.
--
-- This routine is called to raise error if operand that precedes some
-- infix operator has invalid type. */

void error_preceding(MPL *mpl, char *opstr)
{     error(mpl, "operand preceding %s has invalid type", opstr);
      /* no return */
}

/*----------------------------------------------------------------------
-- error_following - raise error if following operand has wrong type.
--
-- This routine is called to raise error if operand that follows some
-- infix operator has invalid type. */

void error_following(MPL *mpl, char *opstr)
{     error(mpl, "operand following %s has invalid type", opstr);
      /* no return */
}

/*----------------------------------------------------------------------
-- error_dimension - raise error if operands have different dimension.
--
-- This routine is called to raise error if two operands of some infix
-- operator have different dimension. */

void error_dimension(MPL *mpl, char *opstr, int dim1, int dim2)
{     error(mpl, "operands preceding and following %s have different di"
         "mensions %d and %d, respectively", opstr, dim1, dim2);
      /* no return */
}

/*----------------------------------------------------------------------
-- expression_0 - parse expression of level 0.
--
-- This routine parses expression of level 0 using the syntax:
--
-- <expression 0> ::= <primary expression> */

CODE *expression_0(MPL *mpl)
{     CODE *code;
      code = primary_expression(mpl);
      return code;
}

/*----------------------------------------------------------------------
-- expression_1 - parse expression of level 1.
--
-- This routine parses expression of level 1 using the syntax:
--
-- <expression 1> ::= <expression 0>
-- <expression 1> ::= <expression 0> <power> <expression 1>
-- <expression 1> ::= <expression 0> <power> <expression 2>
-- <power> ::= ^ | ** */

CODE *expression_1(MPL *mpl)
{     CODE *x, *y;
      char opstr[8];
      x = expression_0(mpl);
      if (mpl->scan_input->token == T_POWER)
      {  strcpy(opstr, mpl->scan_input->image);
         xassert(strlen(opstr) < sizeof(opstr));
         if (x->type == A_SYMBOLIC)
            x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
         if (x->type != A_NUMERIC)
            error_preceding(mpl, opstr);
         get_token(mpl /* ^ | ** */);
         if (mpl->scan_input->token == T_PLUS || mpl->scan_input->token == T_MINUS)
            y = expression_2(mpl);
         else
            y = expression_1(mpl);
         if (y->type == A_SYMBOLIC)
            y = make_unary(mpl, O_CVTNUM, y, A_NUMERIC, 0);
         if (y->type != A_NUMERIC)
            error_following(mpl, opstr);
         x = make_binary(mpl, O_POWER, x, y, A_NUMERIC, 0);
      }
      return x;
}

/*----------------------------------------------------------------------
-- expression_2 - parse expression of level 2.
--
-- This routine parses expression of level 2 using the syntax:
--
-- <expression 2> ::= <expression 1>
-- <expression 2> ::= + <expression 1>
-- <expression 2> ::= - <expression 1> */

CODE *expression_2(MPL *mpl)
{     CODE *x;
      if (mpl->scan_input->token == T_PLUS)
      {  get_token(mpl /* + */);
         x = expression_1(mpl);
         if (x->type == A_SYMBOLIC)
            x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
         if (!(x->type == A_NUMERIC || x->type == A_FORMULA))
            error_following(mpl, "+");
         x = make_unary(mpl, O_PLUS, x, x->type, 0);
      }
      else if (mpl->scan_input->token == T_MINUS)
      {  get_token(mpl /* - */);
         x = expression_1(mpl);
         if (x->type == A_SYMBOLIC)
            x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
         if (!(x->type == A_NUMERIC || x->type == A_FORMULA))
            error_following(mpl, "-");
         x = make_unary(mpl, O_MINUS, x, x->type, 0);
      }
      else
         x = expression_1(mpl);
      return x;
}

/*----------------------------------------------------------------------
-- expression_3 - parse expression of level 3.
--
-- This routine parses expression of level 3 using the syntax:
--
-- <expression 3> ::= <expression 2>
-- <expression 3> ::= <expression 3> * <expression 2>
-- <expression 3> ::= <expression 3> / <expression 2>
-- <expression 3> ::= <expression 3> div <expression 2>
-- <expression 3> ::= <expression 3> mod <expression 2> */

CODE *expression_3(MPL *mpl)
{     CODE *x, *y;
      x = expression_2(mpl);
      for (;;)
      {  if (mpl->scan_input->token == T_ASTERISK)
         {  if (x->type == A_SYMBOLIC)
               x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
            if (!(x->type == A_NUMERIC || x->type == A_FORMULA))
               error_preceding(mpl, "*");
            get_token(mpl /* * */);
            y = expression_2(mpl);
            if (y->type == A_SYMBOLIC)
               y = make_unary(mpl, O_CVTNUM, y, A_NUMERIC, 0);
            if (!(y->type == A_NUMERIC || y->type == A_FORMULA))
               error_following(mpl, "*");
            if (x->type == A_FORMULA && y->type == A_FORMULA)
               error(mpl, "multiplication of linear forms not allowed");
            if (x->type == A_NUMERIC && y->type == A_NUMERIC)
               x = make_binary(mpl, O_MUL, x, y, A_NUMERIC, 0);
            else
               x = make_binary(mpl, O_MUL, x, y, A_FORMULA, 0);
         }
         else if (mpl->scan_input->token == T_SLASH)
         {  if (x->type == A_SYMBOLIC)
               x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
            if (!(x->type == A_NUMERIC || x->type == A_FORMULA))
               error_preceding(mpl, "/");
            get_token(mpl /* / */);
            y = expression_2(mpl);
            if (y->type == A_SYMBOLIC)
               y = make_unary(mpl, O_CVTNUM, y, A_NUMERIC, 0);
            if (y->type != A_NUMERIC)
               error_following(mpl, "/");
            if (x->type == A_NUMERIC)
               x = make_binary(mpl, O_DIV, x, y, A_NUMERIC, 0);
            else
               x = make_binary(mpl, O_DIV, x, y, A_FORMULA, 0);
         }
         else if (mpl->scan_input->token == T_DIV)
         {  if (x->type == A_SYMBOLIC)
               x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
            if (x->type != A_NUMERIC)
               error_preceding(mpl, "div");
            get_token(mpl /* div */);
            y = expression_2(mpl);
            if (y->type == A_SYMBOLIC)
               y = make_unary(mpl, O_CVTNUM, y, A_NUMERIC, 0);
            if (y->type != A_NUMERIC)
               error_following(mpl, "div");
            x = make_binary(mpl, O_IDIV, x, y, A_NUMERIC, 0);
         }
         else if (mpl->scan_input->token == T_MOD)
         {  if (x->type == A_SYMBOLIC)
               x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
            if (x->type != A_NUMERIC)
               error_preceding(mpl, "mod");
            get_token(mpl /* mod */);
            y = expression_2(mpl);
            if (y->type == A_SYMBOLIC)
               y = make_unary(mpl, O_CVTNUM, y, A_NUMERIC, 0);
            if (y->type != A_NUMERIC)
               error_following(mpl, "mod");
            x = make_binary(mpl, O_MOD, x, y, A_NUMERIC, 0);
         }
         else
            break;
      }
      return x;
}

/*----------------------------------------------------------------------
-- expression_4 - parse expression of level 4.
--
-- This routine parses expression of level 4 using the syntax:
--
-- <expression 4> ::= <expression 3>
-- <expression 4> ::= <expression 4> + <expression 3>
-- <expression 4> ::= <expression 4> - <expression 3>
-- <expression 4> ::= <expression 4> less <expression 3> */

CODE *expression_4(MPL *mpl)
{     CODE *x, *y;
      x = expression_3(mpl);
      for (;;)
      {  if (mpl->scan_input->token == T_PLUS)
         {  if (x->type == A_SYMBOLIC)
               x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
            if (!(x->type == A_NUMERIC || x->type == A_FORMULA))
               error_preceding(mpl, "+");
            get_token(mpl /* + */);
            y = expression_3(mpl);
            if (y->type == A_SYMBOLIC)
               y = make_unary(mpl, O_CVTNUM, y, A_NUMERIC, 0);
            if (!(y->type == A_NUMERIC || y->type == A_FORMULA))
               error_following(mpl, "+");
            if (x->type == A_NUMERIC && y->type == A_FORMULA)
               x = make_unary(mpl, O_CVTLFM, x, A_FORMULA, 0);
            if (x->type == A_FORMULA && y->type == A_NUMERIC)
               y = make_unary(mpl, O_CVTLFM, y, A_FORMULA, 0);
            x = make_binary(mpl, O_ADD, x, y, x->type, 0);
         }
         else if (mpl->scan_input->token == T_MINUS)
         {  if (x->type == A_SYMBOLIC)
               x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
            if (!(x->type == A_NUMERIC || x->type == A_FORMULA))
               error_preceding(mpl, "-");
            get_token(mpl /* - */);
            y = expression_3(mpl);
            if (y->type == A_SYMBOLIC)
               y = make_unary(mpl, O_CVTNUM, y, A_NUMERIC, 0);
            if (!(y->type == A_NUMERIC || y->type == A_FORMULA))
               error_following(mpl, "-");
            if (x->type == A_NUMERIC && y->type == A_FORMULA)
               x = make_unary(mpl, O_CVTLFM, x, A_FORMULA, 0);
            if (x->type == A_FORMULA && y->type == A_NUMERIC)
               y = make_unary(mpl, O_CVTLFM, y, A_FORMULA, 0);
            x = make_binary(mpl, O_SUB, x, y, x->type, 0);
         }
         else if (mpl->scan_input->token == T_LESS)
         {  if (x->type == A_SYMBOLIC)
               x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
            if (x->type != A_NUMERIC)
               error_preceding(mpl, "less");
            get_token(mpl /* less */);
            y = expression_3(mpl);
            if (y->type == A_SYMBOLIC)
               y = make_unary(mpl, O_CVTNUM, y, A_NUMERIC, 0);
            if (y->type != A_NUMERIC)
               error_following(mpl, "less");
            x = make_binary(mpl, O_LESS, x, y, A_NUMERIC, 0);
         }
         else
            break;
      }
      return x;
}

/*----------------------------------------------------------------------
-- expression_5 - parse expression of level 5.
--
-- This routine parses expression of level 5 using the syntax:
--
-- <expression 5> ::= <expression 4>
-- <expression 5> ::= <expression 5> & <expression 4> */

CODE *expression_5(MPL *mpl)
{     CODE *x, *y;
      x = expression_4(mpl);
      for (;;)
      {  if (mpl->scan_input->token == T_CONCAT)
         {  if (x->type == A_NUMERIC)
               x = make_unary(mpl, O_CVTSYM, x, A_SYMBOLIC, 0);
            if (x->type != A_SYMBOLIC)
               error_preceding(mpl, "&");
            get_token(mpl /* & */);
            y = expression_4(mpl);
            if (y->type == A_NUMERIC)
               y = make_unary(mpl, O_CVTSYM, y, A_SYMBOLIC, 0);
            if (y->type != A_SYMBOLIC)
               error_following(mpl, "&");
            x = make_binary(mpl, O_CONCAT, x, y, A_SYMBOLIC, 0);
         }
         else
            break;
      }
      return x;
}

/*----------------------------------------------------------------------
-- expression_6 - parse expression of level 6.
--
-- This routine parses expression of level 6 using the syntax:
--
-- <expression 6> ::= <expression 5>
-- <expression 6> ::= <expression 5> .. <expression 5>
-- <expression 6> ::= <expression 5> .. <expression 5> by
--                    <expression 5> */

CODE *expression_6(MPL *mpl)
{     CODE *x, *y, *z;
      x = expression_5(mpl);
      if (mpl->scan_input->token == T_DOTS)
      {  if (x->type == A_SYMBOLIC)
            x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
         if (x->type != A_NUMERIC)
            error_preceding(mpl, "..");
         get_token(mpl /* .. */);
         y = expression_5(mpl);
         if (y->type == A_SYMBOLIC)
            y = make_unary(mpl, O_CVTNUM, y, A_NUMERIC, 0);
         if (y->type != A_NUMERIC)
            error_following(mpl, "..");
         if (mpl->scan_input->token == T_BY)
         {  get_token(mpl /* by */);
            z = expression_5(mpl);
            if (z->type == A_SYMBOLIC)
               z = make_unary(mpl, O_CVTNUM, z, A_NUMERIC, 0);
            if (z->type != A_NUMERIC)
               error_following(mpl, "by");
         }
         else
            z = NULL;
         x = make_ternary(mpl, O_DOTS, x, y, z, A_ELEMSET, 1);
      }
      return x;
}

/*----------------------------------------------------------------------
-- expression_7 - parse expression of level 7.
--
-- This routine parses expression of level 7 using the syntax:
--
-- <expression 7> ::= <expression 6>
-- <expression 7> ::= <expression 7> cross <expression 6> */

CODE *expression_7(MPL *mpl)
{     CODE *x, *y;
      x = expression_6(mpl);
      for (;;)
      {  if (mpl->scan_input->token == T_CROSS)
         {  if (x->type != A_ELEMSET)
               error_preceding(mpl, "cross");
            get_token(mpl /* cross */);
            y = expression_6(mpl);
            if (y->type != A_ELEMSET)
               error_following(mpl, "cross");
            x = make_binary(mpl, O_CROSS, x, y, A_ELEMSET,
               x->dim + y->dim);
         }
         else
            break;
      }
      return x;
}

/*----------------------------------------------------------------------
-- expression_8 - parse expression of level 8.
--
-- This routine parses expression of level 8 using the syntax:
--
-- <expression 8> ::= <expression 7>
-- <expression 8> ::= <expression 8> inter <expression 7> */

CODE *expression_8(MPL *mpl)
{     CODE *x, *y;
      x = expression_7(mpl);
      for (;;)
      {  if (mpl->scan_input->token == T_INTER)
         {  if (x->type != A_ELEMSET)
               error_preceding(mpl, "inter");
            get_token(mpl /* inter */);
            y = expression_7(mpl);
            if (y->type != A_ELEMSET)
               error_following(mpl, "inter");
            if (x->dim != y->dim)
               error_dimension(mpl, "inter", x->dim, y->dim);
            x = make_binary(mpl, O_INTER, x, y, A_ELEMSET, x->dim);
         }
         else
            break;
      }
      return x;
}

/*----------------------------------------------------------------------
-- expression_9 - parse expression of level 9.
--
-- This routine parses expression of level 9 using the syntax:
--
-- <expression 9> ::= <expression 8>
-- <expression 9> ::= <expression 9> union <expression 8>
-- <expression 9> ::= <expression 9> diff <expression 8>
-- <expression 9> ::= <expression 9> symdiff <expression 8> */

CODE *expression_9(MPL *mpl)
{     CODE *x, *y;
      x = expression_8(mpl);
      for (;;)
      {  if (mpl->scan_input->token == T_UNION)
         {  if (x->type != A_ELEMSET)
               error_preceding(mpl, "union");
            get_token(mpl /* union */);
            y = expression_8(mpl);
            if (y->type != A_ELEMSET)
               error_following(mpl, "union");
            if (x->dim != y->dim)
               error_dimension(mpl, "union", x->dim, y->dim);
            x = make_binary(mpl, O_UNION, x, y, A_ELEMSET, x->dim);
         }
         else if (mpl->scan_input->token == T_DIFF)
         {  if (x->type != A_ELEMSET)
               error_preceding(mpl, "diff");
            get_token(mpl /* diff */);
            y = expression_8(mpl);
            if (y->type != A_ELEMSET)
               error_following(mpl, "diff");
            if (x->dim != y->dim)
               error_dimension(mpl, "diff", x->dim, y->dim);
            x = make_binary(mpl, O_DIFF, x, y, A_ELEMSET, x->dim);
         }
         else if (mpl->scan_input->token == T_SYMDIFF)
         {  if (x->type != A_ELEMSET)
               error_preceding(mpl, "symdiff");
            get_token(mpl /* symdiff */);
            y = expression_8(mpl);
            if (y->type != A_ELEMSET)
               error_following(mpl, "symdiff");
            if (x->dim != y->dim)
               error_dimension(mpl, "symdiff", x->dim, y->dim);
            x = make_binary(mpl, O_SYMDIFF, x, y, A_ELEMSET, x->dim);
         }
         else
            break;
      }
      return x;
}

/*----------------------------------------------------------------------
-- expression_10 - parse expression of level 10.
--
-- This routine parses expression of level 10 using the syntax:
--
-- <expression 10> ::= <expression 9>
-- <expression 10> ::= <expression 9> <rho> <expression 9>
-- <rho> ::= < | <= | = | == | >= | > | <> | != | in | not in | ! in |
--           within | not within | ! within */

CODE *expression_10(MPL *mpl)
{     CODE *x, *y;
      int op = -1;
      char opstr[16];
      x = expression_9(mpl);
      strcpy(opstr, "");
      switch (mpl->scan_input->token)
      {  case T_LT:
            op = O_LT; break;
         case T_LE:
            op = O_LE; break;
         case T_EQ:
            op = O_EQ; break;
         case T_GE:
            op = O_GE; break;
         case T_GT:
            op = O_GT; break;
         case T_NE:
            op = O_NE; break;
         case T_IN:
            op = O_IN; break;
         case T_WITHIN:
            op = O_WITHIN; break;
         case T_NOT:
            strcpy(opstr, mpl->scan_input->image);
            get_token(mpl /* not | ! */);
            if (mpl->scan_input->token == T_IN)
               op = O_NOTIN;
            else if (mpl->scan_input->token == T_WITHIN)
               op = O_NOTWITHIN;
            else
               error(mpl, "invalid use of %s", opstr);
            strcat(opstr, " ");
            break;
         default:
            goto done;
      }
      strcat(opstr, mpl->scan_input->image);
      xassert(strlen(opstr) < sizeof(opstr));
      switch (op)
      {  case O_EQ:
         case O_NE:
#if 1 /* 02/VIII-2008 */
         case O_LT:
         case O_LE:
         case O_GT:
         case O_GE:
#endif
            if (!(x->type == A_NUMERIC || x->type == A_SYMBOLIC))
               error_preceding(mpl, opstr);
            get_token(mpl /* <rho> */);
            y = expression_9(mpl);
            if (!(y->type == A_NUMERIC || y->type == A_SYMBOLIC))
               error_following(mpl, opstr);
            if (x->type == A_NUMERIC && y->type == A_SYMBOLIC)
               x = make_unary(mpl, O_CVTSYM, x, A_SYMBOLIC, 0);
            if (x->type == A_SYMBOLIC && y->type == A_NUMERIC)
               y = make_unary(mpl, O_CVTSYM, y, A_SYMBOLIC, 0);
            x = make_binary(mpl, op, x, y, A_LOGICAL, 0);
            break;
#if 0 /* 02/VIII-2008 */
         case O_LT:
         case O_LE:
         case O_GT:
         case O_GE:
            if (x->type == A_SYMBOLIC)
               x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
            if (x->type != A_NUMERIC)
               error_preceding(mpl, opstr);
            get_token(mpl /* <rho> */);
            y = expression_9(mpl);
            if (y->type == A_SYMBOLIC)
               y = make_unary(mpl, O_CVTNUM, y, A_NUMERIC, 0);
            if (y->type != A_NUMERIC)
               error_following(mpl, opstr);
            x = make_binary(mpl, op, x, y, A_LOGICAL, 0);
            break;
#endif
         case O_IN:
         case O_NOTIN:
            if (x->type == A_NUMERIC)
               x = make_unary(mpl, O_CVTSYM, x, A_SYMBOLIC, 0);
            if (x->type == A_SYMBOLIC)
               x = make_unary(mpl, O_CVTTUP, x, A_TUPLE, 1);
            if (x->type != A_TUPLE)
               error_preceding(mpl, opstr);
            get_token(mpl /* <rho> */);
            y = expression_9(mpl);
            if (y->type != A_ELEMSET)
               error_following(mpl, opstr);
            if (x->dim != y->dim)
               error_dimension(mpl, opstr, x->dim, y->dim);
            x = make_binary(mpl, op, x, y, A_LOGICAL, 0);
            break;
         case O_WITHIN:
         case O_NOTWITHIN:
            if (x->type != A_ELEMSET)
               error_preceding(mpl, opstr);
            get_token(mpl /* <rho> */);
            y = expression_9(mpl);
            if (y->type != A_ELEMSET)
               error_following(mpl, opstr);
            if (x->dim != y->dim)
               error_dimension(mpl, opstr, x->dim, y->dim);
            x = make_binary(mpl, op, x, y, A_LOGICAL, 0);
            break;
         default:
            xassert(op != op);
      }
done: return x;
}

/*----------------------------------------------------------------------
-- expression_11 - parse expression of level 11.
--
-- This routine parses expression of level 11 using the syntax:
--
-- <expression 11> ::= <expression 10>
-- <expression 11> ::= not <expression 10>
-- <expression 11> ::= ! <expression 10> */

CODE *expression_11(MPL *mpl)
{     CODE *x;
      char opstr[8];
      if (mpl->scan_input->token == T_NOT)
      {  strcpy(opstr, mpl->scan_input->image);
         xassert(strlen(opstr) < sizeof(opstr));
         get_token(mpl /* not | ! */);
         x = expression_10(mpl);
         if (x->type == A_SYMBOLIC)
            x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
         if (x->type == A_NUMERIC)
            x = make_unary(mpl, O_CVTLOG, x, A_LOGICAL, 0);
         if (x->type != A_LOGICAL)
            error_following(mpl, opstr);
         x = make_unary(mpl, O_NOT, x, A_LOGICAL, 0);
      }
      else
         x = expression_10(mpl);
      return x;
}

/*----------------------------------------------------------------------
-- expression_12 - parse expression of level 12.
--
-- This routine parses expression of level 12 using the syntax:
--
-- <expression 12> ::= <expression 11>
-- <expression 12> ::= <expression 12> and <expression 11>
-- <expression 12> ::= <expression 12> && <expression 11> */

CODE *expression_12(MPL *mpl)
{     CODE *x, *y;
      char opstr[8];
      x = expression_11(mpl);
      for (;;)
      {  if (mpl->scan_input->token == T_AND)
         {  strcpy(opstr, mpl->scan_input->image);
            xassert(strlen(opstr) < sizeof(opstr));
            if (x->type == A_SYMBOLIC)
               x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
            if (x->type == A_NUMERIC)
               x = make_unary(mpl, O_CVTLOG, x, A_LOGICAL, 0);
            if (x->type != A_LOGICAL)
               error_preceding(mpl, opstr);
            get_token(mpl /* and | && */);
            y = expression_11(mpl);
            if (y->type == A_SYMBOLIC)
               y = make_unary(mpl, O_CVTNUM, y, A_NUMERIC, 0);
            if (y->type == A_NUMERIC)
               y = make_unary(mpl, O_CVTLOG, y, A_LOGICAL, 0);
            if (y->type != A_LOGICAL)
               error_following(mpl, opstr);
            x = make_binary(mpl, O_AND, x, y, A_LOGICAL, 0);
         }
         else
            break;
      }
      return x;
}

/*----------------------------------------------------------------------
-- expression_13 - parse expression of level 13.
--
-- This routine parses expression of level 13 using the syntax:
--
-- <expression 13> ::= <expression 12>
-- <expression 13> ::= <expression 13> or <expression 12>
-- <expression 13> ::= <expression 13> || <expression 12> */

CODE *expression_13(MPL *mpl)
{     CODE *x, *y;
      char opstr[8];
      x = expression_12(mpl);
      for (;;)
      {  if (mpl->scan_input->token == T_OR)
         {  strcpy(opstr, mpl->scan_input->image);
            xassert(strlen(opstr) < sizeof(opstr));
            if (x->type == A_SYMBOLIC)
               x = make_unary(mpl, O_CVTNUM, x, A_NUMERIC, 0);
            if (x->type == A_NUMERIC)
               x = make_unary(mpl, O_CVTLOG, x, A_LOGICAL, 0);
            if (x->type != A_LOGICAL)
               error_preceding(mpl, opstr);
            get_token(mpl /* or | || */);
            y = expression_12(mpl);
            if (y->type == A_SYMBOLIC)
               y = make_unary(mpl, O_CVTNUM, y, A_NUMERIC, 0);
            if (y->type == A_NUMERIC)
               y = make_unary(mpl, O_CVTLOG, y, A_LOGICAL, 0);
            if (y->type != A_LOGICAL)
               error_following(mpl, opstr);
            x = make_binary(mpl, O_OR, x, y, A_LOGICAL, 0);
         }
         else
            break;
      }
      return x;
}

/*----------------------------------------------------------------------
-- problem_statement - parse problem statement.
--
-- This routine parses problem statement using the syntax:
--
-- <problem statement> ::= problem <symbolic name> <alias> : <elements>;
-- <alias> ::= <empty>
-- <alias> ::= <string literal>
-- <elements> ::= <empty>
-- <elements> ::= <elements>, <elements>
--
 */

PROBLEM *problem_statement(MPL *mpl)
{     PROBLEM *prob;
      AVLNODE *node;
      xassert(is_keyword(mpl, "problem"));
      get_token(mpl /* problem */);
      /* symbolic name must follow the keyword 'problem' */
      if (mpl->scan_input->token == T_NAME)
         ;
      else if (is_reserved(mpl))
         error(mpl, "invalid use of reserved keyword %s", mpl->scan_input->image);
      else
         error(mpl, "symbolic name missing where expected");
      /* there must be no other object with the same name */
      node = avl_find_node(mpl->tree, mpl->scan_input->image);
      if (node) {
          if(avl_get_node_type(node) == A_PROBLEM) {
              get_token(mpl /* <symbolic name> */);
              if(mpl->scan_input->token != T_SEMICOLON)
                  error(mpl, "problem statement using an existing name do not accept paramenters");
              get_token(mpl /* ; */);
              prob = (PROBLEM*)avl_get_node_link(node);
              mpl->current_problem = prob;
              return prob;
          }
          else error(mpl, "%s already in use", mpl->scan_input->image);
      }
      else if(mpl->nested_scope)
          error(mpl, "problem creation not allowed here");
      /* create model problem */
      prob = alloc(PROBLEM);
      prob->name = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
      strcpy(prob->name, mpl->scan_input->image);
      prob->alias = NULL;
      prob->list = NULL;
      prob->type = 0;
      get_token(mpl /* <symbolic name> */);
      /* parse optional alias */
      if (mpl->scan_input->token == T_STRING)
      {  prob->alias = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
         strcpy(prob->alias, mpl->scan_input->image);
         get_token(mpl /* <string literal> */);
      }
      /* include the problem name in the symbolic names table */
      {  AVLNODE *node;
         node = avl_insert_node(mpl->tree, prob->name);
         avl_set_node_type(node, A_PROBLEM);
         avl_set_node_link(node, (void *)prob);
      }
      mpl->current_problem = prob;
      /* the colon must precede the first element */
      if (mpl->scan_input->token == T_COLON) {
          get_token(mpl /* : */);
          while(mpl->scan_input->token == T_NAME) {
              AVLNODE *node = avl_find_node(mpl->tree, mpl->scan_input->image);
              if (!node) error(mpl, "unknown name '%s'", mpl->scan_input->image);
              STATEMENT *stmt = alloc(STATEMENT);
              stmt->next = NULL;
              stmt->line = 0;
              stmt->type = avl_get_node_type(node);
              stmt->u.var = (VARIABLE*)avl_get_node_link(node);
              add_problem_element(mpl, stmt);
              get_token(mpl /* name */);
              if(mpl->scan_input->token != T_COMMA) break;
              get_token(mpl /* , */);
          }
      }
      /* the problem statement has been completely parsed */
      xassert(mpl->scan_input->token == T_SEMICOLON);
      get_token(mpl /* ; */);
      return prob;
}

/*----------------------------------------------------------------------
-- model_statement - parse model statement.
--
-- This routine parses model statement using the syntax:
--
-- <model statement> ::= model <string file_name>;
-- <string file_name> ::= <empty>
-- <string file_name> ::= <string literal>
--
 */

void model_statement(MPL *mpl)
{
      char *file_name = NULL;
      xassert(is_keyword(mpl, "model"));
      get_token(mpl /* model */);
      if(mpl->nested_scope)
          error(mpl, "model not allowed in nested scopes");
      /* parse optional file name */
      if (mpl->scan_input->token == T_STRING)
      {  file_name = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
         strcpy(file_name, mpl->scan_input->image);
         get_token(mpl /* <string literal> */);
      }
      /* the problem statement has been completely parsed */
      if (mpl->scan_input->token != T_SEMICOLON)
         error(mpl, "semicolon missing where expected");
      get_token(mpl /* ; */);

      mpl->phase = GLP_TRAN_PHASE_MODEL;
      mpl->flag_d = 0;
      if(file_name) {
          glp_input_file_st *saved_scan_input = mpl->scan_input;
          mpl->scan_input = mpl_new_scan_input(mpl);
          open_input(mpl, file_name);
          model_section(mpl);
          close_input(mpl);
          mpl_free_scan_input(mpl, mpl->scan_input);
          mpl->scan_input = saved_scan_input;
      }
}

/*----------------------------------------------------------------------
-- data_statement - parse data statement.
--
-- This routine parses data statement using the syntax:
--
-- <model statement> ::= data <string file_name>;
-- <string file_name> ::= <empty>
-- <string file_name> ::= <string literal>
--
 */

int data_statement(MPL *mpl)
{
      glp_input_file_st *saved_scan_input;
      char *file_name = NULL;
      xassert(is_keyword(mpl, "data"));
      get_token(mpl /* data */);
      if(mpl->nested_scope)
          error(mpl, "data not allowed in nested scopes");
      /* parse optional file name */
      if (mpl->scan_input->token == T_STRING)
      {  file_name = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
         strcpy(file_name, mpl->scan_input->image);
         get_token(mpl /* <string literal> */);
      }
      /* the data statement has been completely parsed */
      if (mpl->scan_input->token != T_SEMICOLON)
        error(mpl, "semicolon missing where expected");
      /* it's important to set mpl->flag_d before the next get_token call */
      mpl->phase = GLP_TRAN_PHASE_DATA;
      mpl->flag_d = 1;
      get_token(mpl /* ; */);

      if(file_name) {
          alloc_content(mpl);
          /* process data file */
          if (mpl->msg_lev >= GLP_MSG_ON)
            xprintf("Reading data section from %s...\n", file_name);
          saved_scan_input = mpl->scan_input;
          mpl->scan_input = mpl_new_scan_input(mpl);
          open_input(mpl, file_name);
          /* in this case the keyword 'data' is optional */
          if (is_literal(mpl, "data"))
          {  get_token(mpl /* data */);
               if (mpl->scan_input->token != T_SEMICOLON)
                  error(mpl, "semicolon missing where expected");
               get_token(mpl /* ; */);
          }
      }
      alloc_content(mpl);
      data_section(mpl, file_name ? 0 : 1);
      if (mpl->msg_lev >= GLP_MSG_ON)
            xprintf("%d line%s were read\n",
              mpl->scan_input->line, mpl->scan_input->line == 1 ? "" : "s");
      if(file_name) {
          close_input(mpl);
          mpl_free_scan_input(mpl, mpl->scan_input);
          mpl->scan_input = saved_scan_input;
      }
      mpl->phase = GLP_TRAN_PHASE_MODEL;
      mpl->flag_d = 0;
      return 1;
}

/*----------------------------------------------------------------------
-- set_statement - parse set statement.
--
-- This routine parses set statement using the syntax:
--
-- <set statement> ::= set <symbolic name> <alias> <domain>
--                     <attributes> ;
-- <alias> ::= <empty>
-- <alias> ::= <string literal>
-- <domain> ::= <empty>
-- <domain> ::= <indexing expression>
-- <attributes> ::= <empty>
-- <attributes> ::= <attributes> , dimen <numeric literal>
-- <attributes> ::= <attributes> , within <expression 9>
-- <attributes> ::= <attributes> , := <expression 9>
-- <attributes> ::= <attributes> , default <expression 9>
--
-- Commae in <attributes> are optional and may be omitted anywhere. */

SET *set_statement(MPL *mpl)
{     SET *set;
      int dimen_used = 0;
      xassert(is_keyword(mpl, "set"));
      get_token(mpl /* set */);
      /* symbolic name must follow the keyword 'set' */
      if (mpl->scan_input->token == T_NAME)
         ;
      else if (is_reserved(mpl))
         error(mpl, "invalid use of reserved keyword %s", mpl->scan_input->image);
      else
         error(mpl, "symbolic name missing where expected");
      /* there must be no other object with the same name */
      if (avl_find_node(mpl->tree, mpl->scan_input->image) != NULL)
         error(mpl, "%s multiply declared", mpl->scan_input->image);
      /* create model set */
      set = alloc(SET);
      set->name = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
      strcpy(set->name, mpl->scan_input->image);
      set->alias = NULL;
      set->dim = 0;
      set->code_valid = mpl->last_code_valid;
      set->domain = NULL;
      set->dimen = 0;
      set->within = NULL;
      set->assign = NULL;
      set->option = NULL;
      set->gadget = NULL;
      set->data = 0;
      set->scoped = mpl->current_for_loop ? 1 : 0;
      set->array = NULL;
      get_token(mpl /* <symbolic name> */);
      /* parse optional alias */
      if (mpl->scan_input->token == T_STRING)
      {  set->alias = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
         strcpy(set->alias, mpl->scan_input->image);
         get_token(mpl /* <string literal> */);
      }
      /* parse optional indexing expression */
      if (mpl->scan_input->token == T_LBRACE)
      {  set->domain = indexing_expression(mpl);
         set->dim = domain_arity(mpl, set->domain);
      }
      /* include the set name in the symbolic names table */
      {  AVLNODE *node;
         node = avl_insert_node(mpl->tree, set->name);
         avl_set_node_type(node, A_SET);
         avl_set_node_link(node, (void *)set);
      }
      /* parse the list of optional attributes */
      for (;;)
      {  if (mpl->scan_input->token == T_COMMA)
            get_token(mpl /* , */);
         else if (mpl->scan_input->token == T_SEMICOLON)
            break;
         if (is_keyword(mpl, "dimen"))
         {  /* dimension of set members */
            int dimen;
            get_token(mpl /* dimen */);
            if (!(mpl->scan_input->token == T_NUMBER &&
                  1.0 <= mpl->scan_input->value && mpl->scan_input->value <= MAX_TUPLE_DIM &&
                  floor(mpl->scan_input->value) == mpl->scan_input->value))
               error(mpl, "dimension must be integer between 1 and %d", MAX_TUPLE_DIM);
            dimen = (int)(mpl->scan_input->value + 0.5);
            if (dimen_used)
               error(mpl, "at most one dimension attribute allowed");
            if (set->dimen > 0)
               error(mpl, "dimension %d conflicts with dimension %d alr"
                  "eady determined", dimen, set->dimen);
            set->dimen = dimen;
            dimen_used = 1;
            get_token(mpl /* <numeric literal> */);
         }
         else if (mpl->scan_input->token == T_WITHIN || mpl->scan_input->token == T_IN)
         {  /* restricting superset */
            WITHIN *within, *temp;
            if (mpl->scan_input->token == T_IN && !mpl->as_within)
            {  warning(mpl, "keyword in understood as within");
               mpl->as_within = 1;
            }
            get_token(mpl /* within */);
            /* create new restricting superset list entry and append it
               to the within-list */
            within = alloc(WITHIN);
            within->code = NULL;
            within->next = NULL;
            if (set->within == NULL)
               set->within = within;
            else
            {  for (temp = set->within; temp->next != NULL; temp =
                  temp->next);
               temp->next = within;
            }
            /* parse an expression that follows 'within' */
            within->code = expression_9(mpl);
            if (within->code->type != A_ELEMSET)
               error(mpl, "expression following within has invalid type"
                  );
            xassert(within->code->dim > 0);
            /* check/set dimension of set members */
            if (set->dimen == 0) set->dimen = within->code->dim;
            if (set->dimen != within->code->dim)
               error(mpl, "set expression following within must have di"
                  "mension %d rather than %d",
                  set->dimen, within->code->dim);
         }
         else if (mpl->scan_input->token == T_ASSIGN)
         {  /* assignment expression */
            if (!(set->assign == NULL && set->option == NULL &&
                  set->gadget == NULL))
err:           error(mpl, "at most one := or default/data allowed");
            get_token(mpl /* := */);
            /* parse an expression that follows ':=' */
            set->assign = expression_9(mpl);
            if (set->assign->type != A_ELEMSET)
               error(mpl, "expression following := has invalid type");
            xassert(set->assign->dim > 0);
            /* check/set dimension of set members */
            if (set->dimen == 0) set->dimen = set->assign->dim;
            if (set->dimen != set->assign->dim)
               error(mpl, "set expression following := must have dimens"
                  "ion %d rather than %d",
                  set->dimen, set->assign->dim);
         }
         else if (is_keyword(mpl, "default"))
         {  /* expression for default value */
            if (!(set->assign == NULL && set->option == NULL)) goto err;
            get_token(mpl /* := */);
            /* parse an expression that follows 'default' */
            set->option = expression_9(mpl);
            if (set->option->type != A_ELEMSET)
               error(mpl, "expression following default has invalid typ"
                  "e");
            xassert(set->option->dim > 0);
            /* check/set dimension of set members */
            if (set->dimen == 0) set->dimen = set->option->dim;
            if (set->dimen != set->option->dim)
               error(mpl, "set expression following default must have d"
                  "imension %d rather than %d",
                  set->dimen, set->option->dim);
         }
#if 1 /* 12/XII-2008 */
         else if (is_keyword(mpl, "data"))
         {  /* gadget to initialize the set by data from plain set */
            GADGET *gadget;
            AVLNODE *node;
            int i, k, fff[MAX_TUPLE_DIM];
            if (!(set->assign == NULL && set->gadget == NULL)) goto err;
            get_token(mpl /* data */);
            set->gadget = gadget = alloc(GADGET);
            /* set name must follow the keyword 'data' */
            if (mpl->scan_input->token == T_NAME)
               ;
            else if (is_reserved(mpl))
               error(mpl, "invalid use of reserved keyword %s",
                  mpl->scan_input->image);
            else
               error(mpl, "set name missing where expected");
            /* find the set in the symbolic name table */
            node = avl_find_node(mpl->tree, mpl->scan_input->image);
            if (node == NULL)
               error(mpl, "%s not defined", mpl->scan_input->image);
            if (avl_get_node_type(node) != A_SET)
err1:          error(mpl, "%s not a plain set", mpl->scan_input->image);
            gadget->set = avl_get_node_link(node);
            if (gadget->set->dim != 0) goto err1;
            if (gadget->set == set)
               error(mpl, "set cannot be initialized by itself");
            /* check and set dimensions */
            if (set->dim >= gadget->set->dimen)
err2:          error(mpl, "dimension of %s too small", mpl->scan_input->image);
            if (set->dimen == 0)
               set->dimen = gadget->set->dimen - set->dim;
            if (set->dim + set->dimen > gadget->set->dimen)
               goto err2;
            else if (set->dim + set->dimen < gadget->set->dimen)
               error(mpl, "dimension of %s too big", mpl->scan_input->image);
            get_token(mpl /* set name */);
            /* left parenthesis must follow the set name */
            if (mpl->scan_input->token == T_LEFT)
               get_token(mpl /* ( */);
            else
               error(mpl, "left parenthesis missing where expected");
            /* parse permutation of component numbers */
            for (k = 0; k < gadget->set->dimen; k++) fff[k] = 0;
            k = 0;
            for (;;)
            {  if (mpl->scan_input->token != T_NUMBER)
                  error(mpl, "component number missing where expected");
               if (str2int(mpl->scan_input->image, &i) != 0)
err3:             error(mpl, "component number must be integer between "
                     "1 and %d", gadget->set->dimen);
               if (!(1 <= i && i <= gadget->set->dimen)) goto err3;
               if (fff[i-1] != 0)
                  error(mpl, "component %d multiply specified", i);
               gadget->ind[k++] = i, fff[i-1] = 1;
               xassert(k <= gadget->set->dimen);
               get_token(mpl /* number */);
               if (mpl->scan_input->token == T_COMMA)
                  get_token(mpl /* , */);
               else if (mpl->scan_input->token == T_RIGHT)
                  break;
               else
                  error(mpl, "syntax error in data attribute");
            }
            if (k < gadget->set->dimen)
               error(mpl, "there are must be %d components rather than "
                  "%d", gadget->set->dimen, k);
            get_token(mpl /* ) */);
         }
#endif
         else if (is_keyword(mpl, "ordered")) {
             warning(mpl, "ordered keyword ignored");
             get_token(mpl /* ordered */);
         }
         else
            error(mpl, "syntax error in set statement");
      }
      /* close the domain scope */
      if (set->domain != NULL) close_scope(mpl, set->domain);
      /* if dimension of set members is still unknown, set it to 1 */
      if (set->dimen == 0) set->dimen = 1;
      /* the set statement has been completely parsed */
      xassert(mpl->scan_input->token == T_SEMICOLON);
      get_token(mpl /* ; */);
      return set;
}

/*----------------------------------------------------------------------
-- parameter_statement - parse parameter statement.
--
-- This routine parses parameter statement using the syntax:
--
-- <parameter statement> ::= param <symbolic name> <alias> <domain>
--                           <attributes> ;
-- <alias> ::= <empty>
-- <alias> ::= <string literal>
-- <domain> ::= <empty>
-- <domain> ::= <indexing expression>
-- <attributes> ::= <empty>
-- <attributes> ::= <attributes> , integer
-- <attributes> ::= <attributes> , binary
-- <attributes> ::= <attributes> , symbolic
-- <attributes> ::= <attributes> , <rho> <expression 5>
-- <attributes> ::= <attributes> , in <expression 9>
-- <attributes> ::= <attributes> , := <expression 5>
-- <attributes> ::= <attributes> , default <expression 5>
-- <rho> ::= < | <= | = | == | >= | > | <> | !=
--
-- Commae in <attributes> are optional and may be omitted anywhere. */

PARAMETER *parameter_statement(MPL *mpl)
{     PARAMETER *par;
      int integer_used = 0, binary_used = 0, symbolic_used = 0;
      xassert(is_keyword(mpl, "param"));
      get_token(mpl /* param */);
      /* symbolic name must follow the keyword 'param' */
      if (mpl->scan_input->token == T_NAME)
         ;
      else if (is_reserved(mpl))
         error(mpl, "invalid use of reserved keyword %s", mpl->scan_input->image);
      else
         error(mpl, "symbolic name missing where expected");
      /* there must be no other object with the same name */
      if (avl_find_node(mpl->tree, mpl->scan_input->image) != NULL)
         error(mpl, "%s multiply declared", mpl->scan_input->image);
      /* create model parameter */
      par = alloc(PARAMETER);
      par->name = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
      strcpy(par->name, mpl->scan_input->image);
      par->alias = NULL;
      par->dim = 0;
      par->domain = NULL;
      par->type = A_NUMERIC;
      par->code_valid = mpl->last_code_valid;
      par->cond = NULL;
      par->in = NULL;
      par->assign = NULL;
      par->option = NULL;
      par->scoped = mpl->current_for_loop ? 1 : 0;
      par->data = 0;
      par->defval.sym = nanbox_null();
      par->array = NULL;
      get_token(mpl /* <symbolic name> */);
      /* parse optional alias */
      if (mpl->scan_input->token == T_STRING)
      {  par->alias = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
         strcpy(par->alias, mpl->scan_input->image);
         get_token(mpl /* <string literal> */);
      }
      /* parse optional indexing expression */
      if (mpl->scan_input->token == T_LBRACE)
      {  par->domain = indexing_expression(mpl);
         par->dim = domain_arity(mpl, par->domain);
      }
      /* include the parameter name in the symbolic names table */
      {  AVLNODE *node;
         node = avl_insert_node(mpl->tree, par->name);
         avl_set_node_type(node, A_PARAMETER);
         avl_set_node_link(node, (void *)par);
      }
      /* parse the list of optional attributes */
      for (;;)
      {  if (mpl->scan_input->token == T_COMMA)
            get_token(mpl /* , */);
         else if (mpl->scan_input->token == T_SEMICOLON)
            break;
         if (is_keyword(mpl, "integer"))
         {  if (integer_used)
               error(mpl, "at most one integer allowed");
            if (par->type == A_SYMBOLIC)
               error(mpl, "symbolic parameter cannot be integer");
            if (par->type != A_BINARY) par->type = A_INTEGER;
            integer_used = 1;
            get_token(mpl /* integer */);
         }
         else if (is_keyword(mpl, "binary"))
bin:     {  if (binary_used)
               error(mpl, "at most one binary allowed");
            if (par->type == A_SYMBOLIC)
               error(mpl, "symbolic parameter cannot be binary");
            par->type = A_BINARY;
            binary_used = 1;
            get_token(mpl /* binary */);
         }
         else if (is_keyword(mpl, "logical"))
         {  if (!mpl->as_binary)
            {  warning(mpl, "keyword logical understood as binary");
               mpl->as_binary = 1;
            }
            goto bin;
         }
         else if (is_keyword(mpl, "symbolic"))
         {  if (symbolic_used)
               error(mpl, "at most one symbolic allowed");
            if (par->type != A_NUMERIC)
               error(mpl, "integer or binary parameter cannot be symbol"
                  "ic");
            /* the parameter may be referenced from expressions given
               in the same parameter declaration, so its type must be
               completed before parsing that expressions */
            if (!(par->cond == NULL && par->in == NULL &&
                  par->assign == NULL && par->option == NULL))
               error(mpl, "keyword symbolic must precede any other para"
                  "meter attributes");
            par->type = A_SYMBOLIC;
            symbolic_used = 1;
            get_token(mpl /* symbolic */);
         }
         else if (mpl->scan_input->token == T_LT || mpl->scan_input->token == T_LE ||
                  mpl->scan_input->token == T_EQ || mpl->scan_input->token == T_GE ||
                  mpl->scan_input->token == T_GT || mpl->scan_input->token == T_NE)
         {  /* restricting condition */
            CONDITION *cond, *temp;
            char opstr[8];
            /* create new restricting condition list entry and append
               it to the conditions list */
            cond = alloc(CONDITION);
            switch (mpl->scan_input->token)
            {  case T_LT:
                  cond->rho = O_LT, strcpy(opstr, mpl->scan_input->image); break;
               case T_LE:
                  cond->rho = O_LE, strcpy(opstr, mpl->scan_input->image); break;
               case T_EQ:
                  cond->rho = O_EQ, strcpy(opstr, mpl->scan_input->image); break;
               case T_GE:
                  cond->rho = O_GE, strcpy(opstr, mpl->scan_input->image); break;
               case T_GT:
                  cond->rho = O_GT, strcpy(opstr, mpl->scan_input->image); break;
               case T_NE:
                  cond->rho = O_NE, strcpy(opstr, mpl->scan_input->image); break;
               default:
                  xassert(mpl->scan_input->token != mpl->scan_input->token);
            }
            xassert(strlen(opstr) < sizeof(opstr));
            cond->code = NULL;
            cond->next = NULL;
            if (par->cond == NULL)
               par->cond = cond;
            else
            {  for (temp = par->cond; temp->next != NULL; temp =
                  temp->next);
               temp->next = cond;
            }
#if 0 /* 13/VIII-2008 */
            if (par->type == A_SYMBOLIC &&
               !(cond->rho == O_EQ || cond->rho == O_NE))
               error(mpl, "inequality restriction not allowed");
#endif
            get_token(mpl /* rho */);
            /* parse an expression that follows relational operator */
            cond->code = expression_5(mpl);
            if (!(cond->code->type == A_NUMERIC ||
                  cond->code->type == A_SYMBOLIC))
               error(mpl, "expression following %s has invalid type",
                  opstr);
            xassert(cond->code->dim == 0);
            /* convert to the parameter type, if necessary */
            if (par->type != A_SYMBOLIC && cond->code->type ==
               A_SYMBOLIC)
               cond->code = make_unary(mpl, O_CVTNUM, cond->code,
                  A_NUMERIC, 0);
            if (par->type == A_SYMBOLIC && cond->code->type !=
               A_SYMBOLIC)
               cond->code = make_unary(mpl, O_CVTSYM, cond->code,
                  A_SYMBOLIC, 0);
         }
         else if (mpl->scan_input->token == T_IN || mpl->scan_input->token == T_WITHIN)
         {  /* restricting superset */
            WITHIN *in, *temp;
            if (mpl->scan_input->token == T_WITHIN && !mpl->as_in)
            {  warning(mpl, "keyword within understood as in");
               mpl->as_in = 1;
            }
            get_token(mpl /* in */);
            /* create new restricting superset list entry and append it
               to the in-list */
            in = alloc(WITHIN);
            in->code = NULL;
            in->next = NULL;
            if (par->in == NULL)
               par->in = in;
            else
            {  for (temp = par->in; temp->next != NULL; temp =
                  temp->next);
               temp->next = in;
            }
            /* parse an expression that follows 'in' */
            in->code = expression_9(mpl);
            if (in->code->type != A_ELEMSET)
               error(mpl, "expression following in has invalid type");
            xassert(in->code->dim > 0);
            if (in->code->dim != 1)
               error(mpl, "set expression following in must have dimens"
                  "ion 1 rather than %d", in->code->dim);
         }
         else if (mpl->scan_input->token == T_ASSIGN)
         {  /* assignment expression */
            if (!(par->assign == NULL && par->option == NULL))
err:           error(mpl, "at most one := or default allowed");
            get_token(mpl /* := */);
            /* parse an expression that follows ':=' */
            par->assign = expression_5(mpl);
            /* the expression must be of numeric/symbolic type */
            if (!(par->assign->type == A_NUMERIC ||
                  par->assign->type == A_SYMBOLIC))
               error(mpl, "expression following := has invalid type");
            xassert(par->assign->dim == 0);
            /* convert to the parameter type, if necessary */
            if (par->type != A_SYMBOLIC && par->assign->type ==
               A_SYMBOLIC)
               par->assign = make_unary(mpl, O_CVTNUM, par->assign,
                  A_NUMERIC, 0);
            if (par->type == A_SYMBOLIC && par->assign->type !=
               A_SYMBOLIC)
               par->assign = make_unary(mpl, O_CVTSYM, par->assign,
                  A_SYMBOLIC, 0);
         }
         else if (is_keyword(mpl, "default"))
         {  /* expression for default value */
            if (!(par->assign == NULL && par->option == NULL)) goto err;
            get_token(mpl /* default */);
            /* parse an expression that follows 'default' */
            par->option = expression_5(mpl);
            if (!(par->option->type == A_NUMERIC ||
                  par->option->type == A_SYMBOLIC))
               error(mpl, "expression following default has invalid typ"
                  "e");
            xassert(par->option->dim == 0);
            /* convert to the parameter type, if necessary */
            if (par->type != A_SYMBOLIC && par->option->type ==
               A_SYMBOLIC)
               par->option = make_unary(mpl, O_CVTNUM, par->option,
                  A_NUMERIC, 0);
            if (par->type == A_SYMBOLIC && par->option->type !=
               A_SYMBOLIC)
               par->option = make_unary(mpl, O_CVTSYM, par->option,
                  A_SYMBOLIC, 0);
         }
         else
            error(mpl, "syntax error in parameter statement");
      }
      /* close the domain scope */
      if (par->domain != NULL) close_scope(mpl, par->domain);
      /* the parameter statement has been completely parsed */
      xassert(mpl->scan_input->token == T_SEMICOLON);
      get_token(mpl /* ; */);
      return par;
}

/*----------------------------------------------------------------------
-- variable_statement - parse variable statement.
--
-- This routine parses variable statement using the syntax:
--
-- <variable statement> ::= var <symbolic name> <alias> <domain>
--                          <attributes> ;
-- <alias> ::= <empty>
-- <alias> ::= <string literal>
-- <domain> ::= <empty>
-- <domain> ::= <indexing expression>
-- <attributes> ::= <empty>
-- <attributes> ::= <attributes> , integer
-- <attributes> ::= <attributes> , binary
-- <attributes> ::= <attributes> , <rho> <expression 5>
-- <rho> ::= >= | <= | = | ==
--
-- Commae in <attributes> are optional and may be omitted anywhere. */

VARIABLE *variable_statement(MPL *mpl)
{     VARIABLE *var;
      int integer_used = 0, binary_used = 0;
      xassert(is_keyword(mpl, "var"));
      if (mpl->flag_s)
         error(mpl, "variable statement must precede solve statement");
      get_token(mpl /* var */);
      /* symbolic name must follow the keyword 'var' */
      if (mpl->scan_input->token == T_NAME)
         ;
      else if (is_reserved(mpl))
         error(mpl, "invalid use of reserved keyword %s", mpl->scan_input->image);
      else
         error(mpl, "symbolic name missing where expected");
      /* there must be no other object with the same name */
      if (avl_find_node(mpl->tree, mpl->scan_input->image) != NULL)
         error(mpl, "%s multiply declared", mpl->scan_input->image);
      /* create model variable */
      var = alloc(VARIABLE);
      var->name = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
      strcpy(var->name, mpl->scan_input->image);
      var->alias = NULL;
      var->dim = 0;
      var->domain = NULL;
      var->type = A_NUMERIC;
      var->code_valid = mpl->last_code_valid;
      var->lbnd = NULL;
      var->ubnd = NULL;
      var->array = NULL;
      get_token(mpl /* <symbolic name> */);
      /* parse optional alias */
      if (mpl->scan_input->token == T_STRING)
      {  var->alias = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
         strcpy(var->alias, mpl->scan_input->image);
         get_token(mpl /* <string literal> */);
      }
      /* parse optional indexing expression */
      if (mpl->scan_input->token == T_LBRACE)
      {  var->domain = indexing_expression(mpl);
         var->dim = domain_arity(mpl, var->domain);
      }
      /* include the variable name in the symbolic names table */
      {  AVLNODE *node;
         node = avl_insert_node(mpl->tree, var->name);
         avl_set_node_type(node, A_VARIABLE);
         avl_set_node_link(node, (void *)var);
      }
      /* parse the list of optional attributes */
      for (;;)
      {  if (mpl->scan_input->token == T_COMMA)
            get_token(mpl /* , */);
         else if (mpl->scan_input->token == T_SEMICOLON)
            break;
         if (is_keyword(mpl, "integer"))
         {  if (integer_used)
               error(mpl, "at most one integer allowed");
            if (var->type != A_BINARY) var->type = A_INTEGER;
            integer_used = 1;
            get_token(mpl /* integer */);
         }
         else if (is_keyword(mpl, "binary"))
bin:     {  if (binary_used)
               error(mpl, "at most one binary allowed");
            var->type = A_BINARY;
            binary_used = 1;
            get_token(mpl /* binary */);
         }
         else if (is_keyword(mpl, "logical"))
         {  if (!mpl->as_binary)
            {  warning(mpl, "keyword logical understood as binary");
               mpl->as_binary = 1;
            }
            goto bin;
         }
         else if (is_keyword(mpl, "symbolic"))
            error(mpl, "variable cannot be symbolic");
         else if (mpl->scan_input->token == T_GE)
         {  /* lower bound */
            if (var->lbnd != NULL)
            {  if (var->lbnd == var->ubnd)
                  error(mpl, "both fixed value and lower bound not allo"
                     "wed");
               else
                  error(mpl, "at most one lower bound allowed");
            }
            get_token(mpl /* >= */);
            /* parse an expression that specifies the lower bound */
            var->lbnd = expression_5(mpl);
            if (var->lbnd->type == A_SYMBOLIC)
               var->lbnd = make_unary(mpl, O_CVTNUM, var->lbnd,
                  A_NUMERIC, 0);
            if (var->lbnd->type != A_NUMERIC)
               error(mpl, "expression following >= has invalid type");
            xassert(var->lbnd->dim == 0);
         }
         else if (mpl->scan_input->token == T_LE)
         {  /* upper bound */
            if (var->ubnd != NULL)
            {  if (var->ubnd == var->lbnd)
                  error(mpl, "both fixed value and upper bound not allo"
                     "wed");
               else
                  error(mpl, "at most one upper bound allowed");
            }
            get_token(mpl /* <= */);
            /* parse an expression that specifies the upper bound */
            var->ubnd = expression_5(mpl);
            if (var->ubnd->type == A_SYMBOLIC)
               var->ubnd = make_unary(mpl, O_CVTNUM, var->ubnd,
                  A_NUMERIC, 0);
            if (var->ubnd->type != A_NUMERIC)
               error(mpl, "expression following <= has invalid type");
            xassert(var->ubnd->dim == 0);
         }
         else if (mpl->scan_input->token == T_EQ)
         {  /* fixed value */
            char opstr[8];
            if (!(var->lbnd == NULL && var->ubnd == NULL))
            {  if (var->lbnd == var->ubnd)
                  error(mpl, "at most one fixed value allowed");
               else if (var->lbnd != NULL)
                  error(mpl, "both lower bound and fixed value not allo"
                     "wed");
               else
                  error(mpl, "both upper bound and fixed value not allo"
                     "wed");
            }
            strcpy(opstr, mpl->scan_input->image);
            xassert(strlen(opstr) < sizeof(opstr));
            get_token(mpl /* = | == */);
            /* parse an expression that specifies the fixed value */
            var->lbnd = expression_5(mpl);
            if (var->lbnd->type == A_SYMBOLIC)
               var->lbnd = make_unary(mpl, O_CVTNUM, var->lbnd,
                  A_NUMERIC, 0);
            if (var->lbnd->type != A_NUMERIC)
               error(mpl, "expression following %s has invalid type",
                  opstr);
            xassert(var->lbnd->dim == 0);
            /* indicate that the variable is fixed, not bounded */
            var->ubnd = var->lbnd;
         }
         else if (mpl->scan_input->token == T_LT || mpl->scan_input->token == T_GT ||
                  mpl->scan_input->token == T_NE)
            error(mpl, "strict bound not allowed");
         else
            error(mpl, "syntax error in variable statement");
      }
      /* close the domain scope */
      if (var->domain != NULL) close_scope(mpl, var->domain);
      /* the variable statement has been completely parsed */
      xassert(mpl->scan_input->token == T_SEMICOLON);
      get_token(mpl /* ; */);
      return var;
}

/*----------------------------------------------------------------------
-- constraint_statement - parse constraint statement.
--
-- This routine parses constraint statement using the syntax:
--
-- <constraint statement> ::= <subject to> <symbolic name> <alias>
--                            <domain> : <constraint> ;
-- <subject to> ::= <empty>
-- <subject to> ::= subject to
-- <subject to> ::= subj to
-- <subject to> ::= s.t.
-- <alias> ::= <empty>
-- <alias> ::= <string literal>
-- <domain> ::= <empty>
-- <domain> ::= <indexing expression>
-- <constraint> ::= <formula> , >= <formula>
-- <constraint> ::= <formula> , <= <formula>
-- <constraint> ::= <formula> , = <formula>
-- <constraint> ::= <formula> , <= <formula> , <= <formula>
-- <constraint> ::= <formula> , >= <formula> , >= <formula>
-- <formula> ::= <expression 5>
--
-- Commae in <constraint> are optional and may be omitted anywhere. */

CONSTRAINT *constraint_statement(MPL *mpl)
{     CONSTRAINT *con;
      CODE *first, *second, *third;
      int rho;
      char opstr[8];
      if (mpl->flag_s)
         error(mpl, "constraint statement must precede solve statement")
            ;
      if (is_keyword(mpl, "subject"))
      {  get_token(mpl /* subject */);
         if (!is_keyword(mpl, "to"))
            error(mpl, "keyword subject to incomplete");
         get_token(mpl /* to */);
      }
      else if (is_keyword(mpl, "subj"))
      {  get_token(mpl /* subj */);
         if (!is_keyword(mpl, "to"))
            error(mpl, "keyword subj to incomplete");
         get_token(mpl /* to */);
      }
      else if (mpl->scan_input->token == T_SPTP)
         get_token(mpl /* s.t. */);
      /* the current token must be symbolic name of constraint */
      if (mpl->scan_input->token == T_NAME)
         ;
      else if (is_reserved(mpl))
         error(mpl, "invalid use of reserved keyword %s", mpl->scan_input->image);
      else
         error(mpl, "symbolic name missing where expected");
      /* there must be no other object with the same name */
      if (avl_find_node(mpl->tree, mpl->scan_input->image) != NULL)
         error(mpl, "%s multiply declared", mpl->scan_input->image);
      /* create model constraint */
      con = alloc(CONSTRAINT);
      con->name = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
      strcpy(con->name, mpl->scan_input->image);
      con->alias = NULL;
      con->dim = 0;
      con->domain = NULL;
      con->type = A_CONSTRAINT;
      con->code_valid = mpl->last_code_valid;
      con->code = NULL;
      con->lbnd = NULL;
      con->ubnd = NULL;
      con->array = NULL;
      get_token(mpl /* <symbolic name> */);
      /* parse optional alias */
      if (mpl->scan_input->token == T_STRING)
      {  con->alias = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
         strcpy(con->alias, mpl->scan_input->image);
         get_token(mpl /* <string literal> */);
      }
      /* parse optional indexing expression */
      if (mpl->scan_input->token == T_LBRACE)
      {  con->domain = indexing_expression(mpl);
         con->dim = domain_arity(mpl, con->domain);
      }
      /* include the constraint name in the symbolic names table */
      {  AVLNODE *node;
         node = avl_insert_node(mpl->tree, con->name);
         avl_set_node_type(node, A_CONSTRAINT);
         avl_set_node_link(node, (void *)con);
      }
      /* the colon must precede the first expression */
      if (mpl->scan_input->token != T_COLON)
         error(mpl, "colon missing where expected");
      get_token(mpl /* : */);
      /* parse the first expression */
      first = expression_5(mpl);
      if (first->type == A_SYMBOLIC)
         first = make_unary(mpl, O_CVTNUM, first, A_NUMERIC, 0);
      if (!(first->type == A_NUMERIC || first->type == A_FORMULA))
         error(mpl, "expression following colon has invalid type");
      xassert(first->dim == 0);
      /* relational operator must follow the first expression */
      if (mpl->scan_input->token == T_COMMA) get_token(mpl /* , */);
      switch (mpl->scan_input->token)
      {  case T_LE:
         case T_GE:
         case T_EQ:
            break;
         case T_LT:
         case T_GT:
         case T_NE:
            error(mpl, "strict inequality not allowed");
         case T_SEMICOLON:
            error(mpl, "constraint must be equality or inequality");
         default:
            goto err;
      }
      rho = mpl->scan_input->token;
      strcpy(opstr, mpl->scan_input->image);
      xassert(strlen(opstr) < sizeof(opstr));
      get_token(mpl /* rho */);
      /* parse the second expression */
      second = expression_5(mpl);
      if (second->type == A_SYMBOLIC)
         second = make_unary(mpl, O_CVTNUM, second, A_NUMERIC, 0);
      if (!(second->type == A_NUMERIC || second->type == A_FORMULA))
         error(mpl, "expression following %s has invalid type", opstr);
      xassert(second->dim == 0);
      /* check a token that follow the second expression */
      if (mpl->scan_input->token == T_COMMA)
      {  get_token(mpl /* , */);
         if (mpl->scan_input->token == T_SEMICOLON) goto err;
      }
      if (mpl->scan_input->token == T_LT || mpl->scan_input->token == T_LE ||
          mpl->scan_input->token == T_EQ || mpl->scan_input->token == T_GE ||
          mpl->scan_input->token == T_GT || mpl->scan_input->token == T_NE)
      {  /* it is another relational operator, therefore the constraint
            is double inequality */
         if (rho == T_EQ || mpl->scan_input->token != rho)
            error(mpl, "double inequality must be ... <= ... <= ... or "
               "... >= ... >= ...");
         /* the first expression cannot be linear form */
         if (first->type == A_FORMULA)
            error(mpl, "leftmost expression in double inequality cannot"
               " be linear form");
         get_token(mpl /* rho */);
         /* parse the third expression */
         third = expression_5(mpl);
         if (third->type == A_SYMBOLIC)
            third = make_unary(mpl, O_CVTNUM, second, A_NUMERIC, 0);
         if (!(third->type == A_NUMERIC || third->type == A_FORMULA))
            error(mpl, "rightmost expression in double inequality const"
               "raint has invalid type");
         xassert(third->dim == 0);
         /* the third expression also cannot be linear form */
         if (third->type == A_FORMULA)
            error(mpl, "rightmost expression in double inequality canno"
               "t be linear form");
      }
      else
      {  /* the constraint is equality or single inequality */
         third = NULL;
      }
      /* close the domain scope */
      if (con->domain != NULL) close_scope(mpl, con->domain);
      /* convert all expressions to linear form, if necessary */
      if (first->type != A_FORMULA)
         first = make_unary(mpl, O_CVTLFM, first, A_FORMULA, 0);
      if (second->type != A_FORMULA)
         second = make_unary(mpl, O_CVTLFM, second, A_FORMULA, 0);
      if (third != NULL)
         third = make_unary(mpl, O_CVTLFM, third, A_FORMULA, 0);
      /* arrange expressions in the constraint */
      if (third == NULL)
      {  /* the constraint is equality or single inequality */
         switch (rho)
         {  case T_LE:
               /* first <= second */
               con->code = first;
               con->lbnd = NULL;
               con->ubnd = second;
               break;
            case T_GE:
               /* first >= second */
               con->code = first;
               con->lbnd = second;
               con->ubnd = NULL;
               break;
            case T_EQ:
               /* first = second */
               con->code = first;
               con->lbnd = second;
               con->ubnd = second;
               break;
            default:
               xassert(rho != rho);
         }
      }
      else
      {  /* the constraint is double inequality */
         switch (rho)
         {  case T_LE:
               /* first <= second <= third */
               con->code = second;
               con->lbnd = first;
               con->ubnd = third;
               break;
            case T_GE:
               /* first >= second >= third */
               con->code = second;
               con->lbnd = third;
               con->ubnd = first;
               break;
            default:
               xassert(rho != rho);
         }
      }
      /* the constraint statement has been completely parsed */
      if (mpl->scan_input->token != T_SEMICOLON)
err:     error(mpl, "syntax error in constraint statement");
      get_token(mpl /* ; */);
      return con;
}

/*----------------------------------------------------------------------
-- objective_statement - parse objective statement.
--
-- This routine parses objective statement using the syntax:
--
-- <objective statement> ::= <verb> <symbolic name> <alias> <domain> :
--                           <formula> ;
-- <verb> ::= minimize
-- <verb> ::= maximize
-- <alias> ::= <empty>
-- <alias> ::= <string literal>
-- <domain> ::= <empty>
-- <domain> ::= <indexing expression>
-- <formula> ::= <expression 5> */

CONSTRAINT *objective_statement(MPL *mpl)
{     CONSTRAINT *obj;
      int type;
      if (is_keyword(mpl, "minimize"))
         type = A_MINIMIZE;
      else if (is_keyword(mpl, "maximize"))
         type = A_MAXIMIZE;
      else
         xassert(mpl != mpl);
      if (mpl->flag_s)
         error(mpl, "objective statement must precede solve statement");
      get_token(mpl /* minimize | maximize */);
      /* symbolic name must follow the verb 'minimize' or 'maximize' */
      if (mpl->scan_input->token == T_NAME)
         ;
      else if (is_reserved(mpl))
         error(mpl, "invalid use of reserved keyword %s", mpl->scan_input->image);
      else
         error(mpl, "symbolic name missing where expected");
      /* there must be no other object with the same name */
      if (avl_find_node(mpl->tree, mpl->scan_input->image) != NULL)
         error(mpl, "%s multiply declared", mpl->scan_input->image);
      /* create model objective */
      obj = alloc(CONSTRAINT);
      obj->name = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
      strcpy(obj->name, mpl->scan_input->image);
      obj->alias = NULL;
      obj->dim = 0;
      obj->domain = NULL;
      obj->type = type;
      obj->code_valid = mpl->last_code_valid;
      obj->code = NULL;
      obj->lbnd = NULL;
      obj->ubnd = NULL;
      obj->array = NULL;
      get_token(mpl /* <symbolic name> */);
      /* parse optional alias */
      if (mpl->scan_input->token == T_STRING)
      {  obj->alias = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
         strcpy(obj->alias, mpl->scan_input->image);
         get_token(mpl /* <string literal> */);
      }
      /* parse optional indexing expression */
      if (mpl->scan_input->token == T_LBRACE)
      {  obj->domain = indexing_expression(mpl);
         obj->dim = domain_arity(mpl, obj->domain);
      }
      /* include the constraint name in the symbolic names table */
      {  AVLNODE *node;
         node = avl_insert_node(mpl->tree, obj->name);
         avl_set_node_type(node, A_CONSTRAINT);
         avl_set_node_link(node, (void *)obj);
      }
      /* the colon must precede the objective expression */
      if (mpl->scan_input->token != T_COLON)
         error(mpl, "colon missing where expected");
      get_token(mpl /* : */);
      /* parse the objective expression */
      obj->code = expression_5(mpl);
      if (obj->code->type == A_SYMBOLIC)
         obj->code = make_unary(mpl, O_CVTNUM, obj->code, A_NUMERIC, 0);
      if (obj->code->type == A_NUMERIC)
         obj->code = make_unary(mpl, O_CVTLFM, obj->code, A_FORMULA, 0);
      if (obj->code->type != A_FORMULA)
         error(mpl, "expression following colon has invalid type");
      xassert(obj->code->dim == 0);
      /* close the domain scope */
      if (obj->domain != NULL) close_scope(mpl, obj->domain);
      /* the objective statement has been completely parsed */
      if (mpl->scan_input->token != T_SEMICOLON)
         error(mpl, "syntax error in objective statement");
      get_token(mpl /* ; */);
      return obj;
}

#if 1 /* 11/II-2008 */
/***********************************************************************
*  table_statement - parse table statement
*
*  This routine parses table statement using the syntax:
*
*  <table statement> ::= <input table statement>
*  <table statement> ::= <output table statement>
*
*  <input table statement> ::=
*        table <table name> <alias> IN <argument list> :
*        <input set> [ <field list> ] , <input list> ;
*  <alias> ::= <empty>
*  <alias> ::= <string literal>
*  <argument list> ::= <expression 5>
*  <argument list> ::= <argument list> <expression 5>
*  <argument list> ::= <argument list> , <expression 5>
*  <input set> ::= <empty>
*  <input set> ::= <set name> <-
*  <field list> ::= <field name>
*  <field list> ::= <field list> , <field name>
*  <input list> ::= <input item>
*  <input list> ::= <input list> , <input item>
*  <input item> ::= <parameter name>
*  <input item> ::= <parameter name> ~ <field name>
*
*  <output table statement> ::=
*        table <table name> <alias> <domain> OUT <argument list> :
*        <output list> ;
*  <domain> ::= <indexing expression>
*  <output list> ::= <output item>
*  <output list> ::= <output list> , <output item>
*  <output item> ::= <expression 5>
*  <output item> ::= <expression 5> ~ <field name> */

TABLE *table_statement(MPL *mpl)
{     TABLE *tab;
      TABARG *last_arg, *arg;
      TABFLD *last_fld, *fld;
      TABIN *last_in, *in;
      TABOUT *last_out, *out;
      AVLNODE *node;
      int nflds;
      char name[MAX_LENGTH+1];
      xassert(is_keyword(mpl, "table"));
      get_token(mpl /* solve */);
      /* symbolic name must follow the keyword table */
      if (mpl->scan_input->token == T_NAME)
         ;
      else if (is_reserved(mpl))
         error(mpl, "invalid use of reserved keyword %s", mpl->scan_input->image);
      else
         error(mpl, "symbolic name missing where expected");
      /* there must be no other object with the same name */
      if (avl_find_node(mpl->tree, mpl->scan_input->image) != NULL)
         error(mpl, "%s multiply declared", mpl->scan_input->image);
      /* create data table */
      tab = alloc(TABLE);
      tab->name = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
      strcpy(tab->name, mpl->scan_input->image);
      get_token(mpl /* <symbolic name> */);
      /* parse optional alias */
      if (mpl->scan_input->token == T_STRING)
      {  tab->alias = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
         strcpy(tab->alias, mpl->scan_input->image);
         get_token(mpl /* <string literal> */);
      }
      else
         tab->alias = NULL;
      /* parse optional indexing expression */
      if (mpl->scan_input->token == T_LBRACE)
      {  /* this is output table */
         tab->type = A_OUTPUT;
         tab->u.out.domain = indexing_expression(mpl);
         if (!is_keyword(mpl, "OUT"))
            error(mpl, "keyword OUT missing where expected");
         get_token(mpl /* OUT */);
      }
      else
      {  /* this is input table */
         tab->type = A_INPUT;
         if (!is_keyword(mpl, "IN"))
            error(mpl, "keyword IN missing where expected");
         get_token(mpl /* IN */);
      }
      /* parse argument list */
      tab->arg = last_arg = NULL;
      for (;;)
      {  /* create argument list entry */
         arg = alloc(TABARG);
         /* parse argument expression */
         if (mpl->scan_input->token == T_COMMA || mpl->scan_input->token == T_COLON ||
             mpl->scan_input->token == T_SEMICOLON)
            error(mpl, "argument expression missing where expected");
         arg->code = expression_5(mpl);
         /* convert the result to symbolic type, if necessary */
         if (arg->code->type == A_NUMERIC)
            arg->code =
               make_unary(mpl, O_CVTSYM, arg->code, A_SYMBOLIC, 0);
         /* check that now the result is of symbolic type */
         if (arg->code->type != A_SYMBOLIC)
            error(mpl, "argument expression has invalid type");
         /* add the entry to the end of the list */
         arg->next = NULL;
         if (last_arg == NULL)
            tab->arg = arg;
         else
            last_arg->next = arg;
         last_arg = arg;
         /* argument expression has been parsed */
         if (mpl->scan_input->token == T_COMMA)
            get_token(mpl /* , */);
         else if (mpl->scan_input->token == T_COLON || mpl->scan_input->token == T_SEMICOLON)
            break;
      }
      xassert(tab->arg != NULL);
      /* argument list must end with colon */
      if (mpl->scan_input->token == T_COLON)
         get_token(mpl /* : */);
      else
         error(mpl, "colon missing where expected");
      /* parse specific part of the table statement */
      switch (tab->type)
      {  case A_INPUT:  goto input_table;
         case A_OUTPUT: goto output_table;
         default:       xassert(tab != tab);
      }
input_table:
      /* parse optional set name */
      if (mpl->scan_input->token == T_NAME)
      {  node = avl_find_node(mpl->tree, mpl->scan_input->image);
         if (node == NULL)
            error(mpl, "%s not defined", mpl->scan_input->image);
         if (avl_get_node_type(node) != A_SET)
            error(mpl, "%s not a set", mpl->scan_input->image);
         tab->u.in.set = (SET *)avl_get_node_link(node);
         if (tab->u.in.set->assign != NULL)
            error(mpl, "%s needs no data", mpl->scan_input->image);
         if (tab->u.in.set->dim != 0)
            error(mpl, "%s must be a simple set", mpl->scan_input->image);
         get_token(mpl /* <symbolic name> */);
         if (mpl->scan_input->token == T_INPUT)
            get_token(mpl /* <- */);
         else
            error(mpl, "delimiter <- missing where expected");
      }
      else if (is_reserved(mpl))
         error(mpl, "invalid use of reserved keyword %s", mpl->scan_input->image);
      else
         tab->u.in.set = NULL;
      /* parse field list */
      tab->u.in.fld = last_fld = NULL;
      nflds = 0;
      if (mpl->scan_input->token == T_LBRACKET)
         get_token(mpl /* [ */);
      else
         error(mpl, "field list missing where expected");
      for (;;)
      {  /* create field list entry */
         fld = alloc(TABFLD);
         /* parse field name */
         if (mpl->scan_input->token == T_NAME)
            ;
         else if (is_reserved(mpl))
            error(mpl,
               "invalid use of reserved keyword %s", mpl->scan_input->image);
         else
            error(mpl, "field name missing where expected");
         fld->name = dmp_get_atomv(mpl->pool, strlen(mpl->scan_input->image)+1);
         strcpy(fld->name, mpl->scan_input->image);
         get_token(mpl /* <symbolic name> */);
         /* add the entry to the end of the list */
         fld->next = NULL;
         if (last_fld == NULL)
            tab->u.in.fld = fld;
         else
            last_fld->next = fld;
         last_fld = fld;
         nflds++;
         /* field name has been parsed */
         if (mpl->scan_input->token == T_COMMA)
            get_token(mpl /* , */);
         else if (mpl->scan_input->token == T_RBRACKET)
            break;
         else
            error(mpl, "syntax error in field list");
      }
      /* check that the set dimen is equal to the number of fields */
      if (tab->u.in.set != NULL && tab->u.in.set->dimen != nflds)
         error(mpl, "there must be %d field%s rather than %d",
            tab->u.in.set->dimen, tab->u.in.set->dimen == 1 ? "" : "s",
            nflds);
      get_token(mpl /* ] */);
      /* parse optional input list */
      tab->u.in.list = last_in = NULL;
      while (mpl->scan_input->token == T_COMMA)
      {  get_token(mpl /* , */);
         /* create input list entry */
         in = alloc(TABIN);
         /* parse parameter name */
         if (mpl->scan_input->token == T_NAME)
            ;
         else if (is_reserved(mpl))
            error(mpl,
               "invalid use of reserved keyword %s", mpl->scan_input->image);
         else
            error(mpl, "parameter name missing where expected");
         node = avl_find_node(mpl->tree, mpl->scan_input->image);
         if (node == NULL)
            error(mpl, "%s not defined", mpl->scan_input->image);
         if (avl_get_node_type(node) != A_PARAMETER)
            error(mpl, "%s not a parameter", mpl->scan_input->image);
         in->par = (PARAMETER *)avl_get_node_link(node);
         if (in->par->dim != nflds)
            error(mpl, "%s must have %d subscript%s rather than %d",
               mpl->scan_input->image, nflds, nflds == 1 ? "" : "s", in->par->dim);
         if (in->par->assign != NULL)
            error(mpl, "%s needs no data", mpl->scan_input->image);
         get_token(mpl /* <symbolic name> */);
         /* parse optional field name */
         if (mpl->scan_input->token == T_TILDE)
         {  get_token(mpl /* ~ */);
            /* parse field name */
            if (mpl->scan_input->token == T_NAME)
               ;
            else if (is_reserved(mpl))
               error(mpl,
                  "invalid use of reserved keyword %s", mpl->scan_input->image);
            else
               error(mpl, "field name missing where expected");
            xassert(strlen(mpl->scan_input->image) < sizeof(name));
            strcpy(name, mpl->scan_input->image);
            get_token(mpl /* <symbolic name> */);
         }
         else
         {  /* field name is the same as the parameter name */
            xassert(strlen(in->par->name) < sizeof(name));
            strcpy(name, in->par->name);
         }
         /* assign field name */
         in->name = dmp_get_atomv(mpl->pool, strlen(name)+1);
         strcpy(in->name, name);
         /* add the entry to the end of the list */
         in->next = NULL;
         if (last_in == NULL)
            tab->u.in.list = in;
         else
            last_in->next = in;
         last_in = in;
      }
      goto end_of_table;
output_table:
      /* parse output list */
      tab->u.out.list = last_out = NULL;
      for (;;)
      {  /* create output list entry */
         out = alloc(TABOUT);
         /* parse expression */
         if (mpl->scan_input->token == T_COMMA || mpl->scan_input->token == T_SEMICOLON)
            error(mpl, "expression missing where expected");
         if (mpl->scan_input->token == T_NAME)
         {  xassert(strlen(mpl->scan_input->image) < sizeof(name));
            strcpy(name, mpl->scan_input->image);
         }
         else
            name[0] = '\0';
         out->code = expression_5(mpl);
         /* parse optional field name */
         if (mpl->scan_input->token == T_TILDE)
         {  get_token(mpl /* ~ */);
            /* parse field name */
            if (mpl->scan_input->token == T_NAME)
               ;
            else if (is_reserved(mpl))
               error(mpl,
                  "invalid use of reserved keyword %s", mpl->scan_input->image);
            else
               error(mpl, "field name missing where expected");
            xassert(strlen(mpl->scan_input->image) < sizeof(name));
            strcpy(name, mpl->scan_input->image);
            get_token(mpl /* <symbolic name> */);
         }
         /* assign field name */
         if (name[0] == '\0')
            error(mpl, "field name required");
         out->name = dmp_get_atomv(mpl->pool, strlen(name)+1);
         strcpy(out->name, name);
         /* add the entry to the end of the list */
         out->next = NULL;
         if (last_out == NULL)
            tab->u.out.list = out;
         else
            last_out->next = out;
         last_out = out;
         /* output item has been parsed */
         if (mpl->scan_input->token == T_COMMA)
            get_token(mpl /* , */);
         else if (mpl->scan_input->token == T_SEMICOLON)
            break;
         else
            error(mpl, "syntax error in output list");
      }
      /* close the domain scope */
      close_scope(mpl,tab->u.out.domain);
end_of_table:
      /* the table statement must end with semicolon */
      if (mpl->scan_input->token != T_SEMICOLON)
         error(mpl, "syntax error in table statement");
      get_token(mpl /* ; */);
      return tab;
}
#endif

/*----------------------------------------------------------------------
-- solve_statement - parse solve statement.
--
-- This routine parses solve statement using the syntax:
--
-- <solve statement> ::= solve <problem>;
-- <problem> ::= <empty>
--
-- The solve statement can be used at most once. */

SOLVE *solve_statement(MPL *mpl)
{     SOLVE *solve;
      xassert(is_keyword(mpl, "solve"));
      //if (mpl->flag_s)
      //   error(mpl, "at most one solve statement allowed");
      ++mpl->flag_s;
      get_token(mpl /* solve */);
      solve = alloc(SOLVE);
      solve->prob = NULL;
      solve->type = 0;
      /* solve type */
      if (mpl->scan_input->token == T_POINT) {
        get_token(mpl /* . */);
        if (mpl->scan_input->token == T_NAME) {
            if(strcmp(mpl->scan_input->image, "lp") == 0)
                solve->type = A_PROBLEM_LP;
            else if(strcmp(mpl->scan_input->image, "mip") == 0)
                solve->type = A_PROBLEM_MIP;
            else if(strcmp(mpl->scan_input->image, "ip") == 0)
                solve->type = A_PROBLEM_IP;
            else
                error(mpl, "unknown solve option %s, valid options are lp/mip/ip", mpl->scan_input->image);
            get_token(mpl /* name */);
        }
        else
            error(mpl, "solve option expected, valid options are lp/mip/ip");
      }
      /* there is a problem ? */
      if (mpl->scan_input->token == T_NAME) {
          AVLNODE *node = avl_find_node(mpl->tree, mpl->scan_input->image);
          if(!node || avl_get_node_type(node) != A_PROBLEM)
            error(mpl, "%s is not a problem name", mpl->scan_input->image);
          solve->prob = (PROBLEM *)avl_get_node_link(node);
          get_token(mpl /* name */);
      }
      /* semicolon must follow solve statement */
      if (mpl->scan_input->token != T_SEMICOLON)
         error(mpl, "syntax error in solve statement");
      get_token(mpl /* ; */);
      return solve;
}

/*----------------------------------------------------------------------
-- check_statement - parse check statement.
--
-- This routine parses check statement using the syntax:
--
-- <check statement> ::= check <domain> : <expression 13> ;
-- <domain> ::= <empty>
-- <domain> ::= <indexing expression>
--
-- If <domain> is omitted, colon following it may also be omitted. */

CHECK *check_statement(MPL *mpl)
{     CHECK *chk;
      xassert(is_keyword(mpl, "check"));
      /* create check descriptor */
      chk = alloc(CHECK);
      chk->domain = NULL;
      chk->code = NULL;
      get_token(mpl /* check */);
      /* parse optional indexing expression */
      if (mpl->scan_input->token == T_LBRACE)
      {  chk->domain = indexing_expression(mpl);
#if 0
         if (mpl->scan_input->token != T_COLON)
            error(mpl, "colon missing where expected");
#endif
      }
      /* skip optional colon */
      if (mpl->scan_input->token == T_COLON) get_token(mpl /* : */);
      /* parse logical expression */
      chk->code = expression_13(mpl);
      if (chk->code->type != A_LOGICAL)
         error(mpl, "expression has invalid type");
      xassert(chk->code->dim == 0);
      /* close the domain scope */
      if (chk->domain != NULL) close_scope(mpl, chk->domain);
      /* the check statement has been completely parsed */
      if (mpl->scan_input->token != T_SEMICOLON)
         error(mpl, "syntax error in check statement");
      get_token(mpl /* ; */);
      return chk;
}

/*----------------------------------------------------------------------
-- let_statement - parse ley statement.
--
-- This routine parses check statement using the syntax:
--
-- <check statement> ::= check <domain> : <expression 13> ;
-- <domain> ::= <empty>
-- <domain> ::= <indexing expression>
--
-- If <domain> is omitted, colon following it may also be omitted. */

LET_STMT *let_statement(MPL *mpl)
{     LET_STMT *let;
      xassert(is_keyword(mpl, "let"));
      /* create let descriptor */
      let = alloc(LET_STMT);
      let->domain = NULL;
      let->par = NULL;
      let->assign = NULL;
      get_token(mpl /* let */);
      /* parse optional indexing expression */
      if (mpl->scan_input->token == T_LBRACE)
      {  let->domain = indexing_expression(mpl);
#if 0
         if (mpl->scan_input->token != T_COLON)
            error(mpl, "colon missing where expected");
#endif
      }
      /* skip optional colon */
      if (mpl->scan_input->token == T_COLON) get_token(mpl /* : */);
      if (mpl->scan_input->token != T_NAME)
          error(mpl, "existing model element missing");
      let->par = object_reference(mpl);
      switch (let->par->op)
      {  case O_MEMNUM:
         case O_MEMSYM:
            break;
          default:
              error(mpl, "only parameter allowed here");
      }
      if (mpl->scan_input->token != T_ASSIGN)
          error(mpl, ":= assignment token expected");
      get_token(mpl /* := */);
      /* parse an expression that follows ':=' */
      let->assign = expression_5(mpl);
      /* the expression must be of numeric/symbolic type */
      if (!(let->assign->type == A_NUMERIC ||
              let->assign->type == A_SYMBOLIC))
           error(mpl, "expression following := has invalid type");
      xassert(let->assign->dim == 0);
      /* convert to the parameter type, if necessary */
      if (let->par->arg.par.par->type != A_SYMBOLIC && let->assign->type ==
           A_SYMBOLIC)
           let->assign = make_unary(mpl, O_CVTNUM, let->assign,
              A_NUMERIC, 0);
      if (let->par->arg.par.par->type == A_SYMBOLIC && let->assign->type !=
           A_SYMBOLIC)
           let->assign = make_unary(mpl, O_CVTSYM, let->assign,
              A_SYMBOLIC, 0);
      /* close the domain scope */
      if (let->domain != NULL) close_scope(mpl, let->domain);
      /* the check statement has been completely parsed */
      if (mpl->scan_input->token != T_SEMICOLON)
         error(mpl, "syntax error in check statement");
      get_token(mpl /* ; */);
      return let;
}

#if 1 /* 15/V-2010 */
/*----------------------------------------------------------------------
-- display_statement - parse display statement.
--
-- This routine parses display statement using the syntax:
--
-- <display statement> ::= display <domain> : <display list> ;
-- <display statement> ::= display <domain> <display list> ;
-- <domain> ::= <empty>
-- <domain> ::= <indexing expression>
-- <display list> ::= <display entry>
-- <display list> ::= <display list> , <display entry>
-- <display entry> ::= <dummy index>
-- <display entry> ::= <set name>
-- <display entry> ::= <set name> [ <subscript list> ]
-- <display entry> ::= <parameter name>
-- <display entry> ::= <parameter name> [ <subscript list> ]
-- <display entry> ::= <variable name>
-- <display entry> ::= <variable name> [ <subscript list> ]
-- <display entry> ::= <constraint name>
-- <display entry> ::= <constraint name> [ <subscript list> ]
-- <display entry> ::= <problem name>
-- <display entry> ::= <expression 13> */

DISPLAY *display_statement(MPL *mpl)
{     DISPLAY *dpy;
      DISPLAY1 *entry, *last_entry;
      xassert(is_keyword(mpl, "display"));
      /* create display descriptor */
      dpy = alloc(DISPLAY);
      dpy->domain = NULL;
      dpy->list = last_entry = NULL;
      get_token(mpl /* display */);
      /* parse optional indexing expression */
      if (mpl->scan_input->token == T_LBRACE)
         dpy->domain = indexing_expression(mpl);
      /* skip optional colon */
      if (mpl->scan_input->token == T_COLON) get_token(mpl /* : */);
      /* parse display list */
      for (;;)
      {  /* create new display entry */
         entry = alloc(DISPLAY1);
         entry->type = 0;
         entry->next = NULL;
         /* and append it to the display list */
         if (dpy->list == NULL)
            dpy->list = entry;
         else
            last_entry->next = entry;
         last_entry = entry;
         /* parse display entry */
         if (mpl->scan_input->token == T_NAME)
         {  AVLNODE *node;
            int next_token;
            get_token(mpl /* <symbolic name> */);
            next_token = mpl->scan_input->token;
            unget_token(mpl);
            if (!(next_token == T_COMMA || next_token == T_SEMICOLON))
            {  /* symbolic name begins expression */
               goto expr;
            }
            /* display entry is dummy index or model object */
            node = avl_find_node(mpl->tree, mpl->scan_input->image);
            if (node == NULL)
               error(mpl, "%s not defined", mpl->scan_input->image);
            entry->type = avl_get_node_type(node);
            switch (avl_get_node_type(node))
            {  case A_INDEX:
                  entry->u.slot =
                     (DOMAIN_SLOT *)avl_get_node_link(node);
                  break;
               case A_SET:
                  entry->u.set = (SET *)avl_get_node_link(node);
                  break;
               case A_PARAMETER:
                  entry->u.par = (PARAMETER *)avl_get_node_link(node);
                  break;
               case A_VARIABLE:
                  entry->u.var = (VARIABLE *)avl_get_node_link(node);
                  if (!mpl->flag_s)
                     error(mpl, "invalid reference to variable %s above"
                        " solve statement", entry->u.var->name);
                  break;
               case A_CONSTRAINT:
                  entry->u.con = (CONSTRAINT *)avl_get_node_link(node);
                  if (!mpl->flag_s)
                     error(mpl, "invalid reference to %s %s above solve"
                        " statement",
                        entry->u.con->type == A_CONSTRAINT ?
                        "constraint" : "objective", entry->u.con->name);
                  break;
               case A_PROBLEM:
                  entry->u.prob = (PROBLEM *)avl_get_node_link(node);
                  break;
               default:
                  xassert(node != node);
            }
            get_token(mpl /* <symbolic name> */);
         }
         else
expr:    {  /* display entry is expression */
            entry->type = A_EXPRESSION;
            entry->u.code = expression_13(mpl);
         }
         /* check a token that follows the entry parsed */
         if (mpl->scan_input->token == T_COMMA)
            get_token(mpl /* , */);
         else
            break;
      }
      /* close the domain scope */
      if (dpy->domain != NULL) close_scope(mpl, dpy->domain);
      /* the display statement has been completely parsed */
      if (mpl->scan_input->token != T_SEMICOLON)
         error(mpl, "syntax error in display statement");
      get_token(mpl /* ; */);
      return dpy;
}
#endif

/*----------------------------------------------------------------------
-- printf_statement - parse printf statement.
--
-- This routine parses print statement using the syntax:
--
-- <printf statement> ::= <printf clause> ;
-- <printf statement> ::= <printf clause> > <file name> ;
-- <printf statement> ::= <printf clause> >> <file name> ;
-- <printf clause> ::= printf <domain> : <format> <printf list>
-- <printf clause> ::= printf <domain> <format> <printf list>
-- <domain> ::= <empty>
-- <domain> ::= <indexing expression>
-- <format> ::= <expression 5>
-- <printf list> ::= <empty>
-- <printf list> ::= <printf list> , <printf entry>
-- <printf entry> ::= <expression 9>
-- <file name> ::= <expression 5> */

PRINTF *printf_statement(MPL *mpl)
{     PRINTF *prt;
      PRINTF1 *entry, *last_entry;
      xassert(is_keyword(mpl, "printf"));
      /* create printf descriptor */
      prt = alloc(PRINTF);
      prt->domain = NULL;
      prt->fmt = NULL;
      prt->list = last_entry = NULL;
      get_token(mpl /* printf */);
      /* parse optional indexing expression */
      if (mpl->scan_input->token == T_LBRACE)
      {  prt->domain = indexing_expression(mpl);
#if 0
         if (mpl->scan_input->token != T_COLON)
            error(mpl, "colon missing where expected");
#endif
      }
      /* skip optional colon */
      if (mpl->scan_input->token == T_COLON) get_token(mpl /* : */);
      /* parse expression for format string */
      prt->fmt = expression_5(mpl);
      /* convert it to symbolic type, if necessary */
      if (prt->fmt->type == A_NUMERIC)
         prt->fmt = make_unary(mpl, O_CVTSYM, prt->fmt, A_SYMBOLIC, 0);
      /* check that now the expression is of symbolic type */
      if (prt->fmt->type != A_SYMBOLIC)
         error(mpl, "format expression has invalid type");
      /* parse printf list */
      while (mpl->scan_input->token == T_COMMA)
      {  get_token(mpl /* , */);
         /* create new printf entry */
         entry = alloc(PRINTF1);
         entry->code = NULL;
         entry->next = NULL;
         /* and append it to the printf list */
         if (prt->list == NULL)
            prt->list = entry;
         else
            last_entry->next = entry;
         last_entry = entry;
         /* parse printf entry */
         entry->code = expression_9(mpl);
         if (!(entry->code->type == A_NUMERIC ||
               entry->code->type == A_SYMBOLIC ||
               entry->code->type == A_LOGICAL))
            error(mpl, "only numeric, symbolic, or logical expression a"
               "llowed");
      }
      /* close the domain scope */
      if (prt->domain != NULL) close_scope(mpl, prt->domain);
#if 1 /* 14/VII-2006 */
      /* parse optional redirection */
      prt->fname = NULL, prt->app = 0;
      if (mpl->scan_input->token == T_GT || mpl->scan_input->token == T_APPEND)
      {  prt->app = (mpl->scan_input->token == T_APPEND);
         get_token(mpl /* > or >> */);
         /* parse expression for file name string */
         prt->fname = expression_5(mpl);
         /* convert it to symbolic type, if necessary */
         if (prt->fname->type == A_NUMERIC)
            prt->fname = make_unary(mpl, O_CVTSYM, prt->fname,
               A_SYMBOLIC, 0);
         /* check that now the expression is of symbolic type */
         if (prt->fname->type != A_SYMBOLIC)
            error(mpl, "file name expression has invalid type");
      }
#endif
      /* the printf statement has been completely parsed */
      if (mpl->scan_input->token != T_SEMICOLON)
         error(mpl, "syntax error in printf statement");
      get_token(mpl /* ; */);
      return prt;
}

static STATEMENT *parse_compound_statement(MPL *mpl)
{
    STATEMENT *list, *stmt, *last_stmt;
    list = last_stmt = NULL;
    get_token(mpl /* { */);
    ++mpl->nested_scope;
    while (mpl->scan_input->token != T_RBRACE)
    {  /* parse statement */
       stmt = simple_statement(mpl, 1);
       /* and append it to the end of the statement list */
       if (last_stmt == NULL)
          list = stmt;
       else
          last_stmt->next = stmt;
       last_stmt = stmt;
    }
    get_token(mpl /* } */);
    --mpl->nested_scope;
    return list;
}

/* Remove local param/set declarations from symbol table */
static void close_scope_remove_decl(MPL *mpl, STATEMENT *list)
{
    STATEMENT *stmt;
    AVLNODE *node = NULL;
    for(stmt = list; stmt != NULL; stmt = stmt->next)
    {
        if(stmt->type == A_PARAMETER)
            node = avl_find_node(mpl->tree, stmt->u.par->name);
        else if(stmt->type == A_SET)
            node = avl_find_node(mpl->tree, stmt->u.set->name);
        if(node)
        {
            alloc_content_for_stmt(mpl, stmt);
            avl_delete_node(mpl->tree, node);
            node = NULL;
        }
    }
}

/*----------------------------------------------------------------------
-- for_statement - parse for/repeat statement.
--
-- This routine parses for/repeat statement using the syntax:
-- Note the repeat is identical to for but without a domain
--
-- <for statement> ::= for <domain> <statement>
-- <for statement> ::= for <domain> { <statement list> }
-- <domain> ::= <indexing expression>
-- <statement list> ::= <empty>
-- <statement list> ::= <statement list> <statement>
-- <statement> ::= <check statement>
-- <statement> ::= <display statement>
-- <statement> ::= <printf statement>
-- <statement> ::= <if statement>
-- <statement> ::= <for statement> */

FOR *for_statement(MPL *mpl, int is_repeat)
{     FOR *fur, *prev_for_loop;
      xassert(is_keyword(mpl, "for") || is_keyword(mpl, "repeat"));
      /* create for descriptor */
      fur = alloc(FOR);
      fur->domain = NULL;
      fur->do_break = 0;
      fur->do_continue = 0;
      fur->list = NULL;
      prev_for_loop = mpl->current_for_loop;
      mpl->current_for_loop = fur;
      get_token(mpl /* for/repeat */);
      if(!is_repeat) {
        /* parse indexing expression */
        if (mpl->scan_input->token != T_LBRACE)
           error(mpl, "indexing expression missing where expected");
        fur->domain = indexing_expression(mpl);
        /* skip optional colon */
        if (mpl->scan_input->token == T_COLON) get_token(mpl /* : */);
      }
      /* parse for statement body */
      if (mpl->scan_input->token != T_LBRACE)
      {  /* parse simple statement */
         fur->list = simple_statement(mpl, 1);
      }
      else
          fur->list = parse_compound_statement(mpl);
      if(!is_repeat) {
        /* close the domain scope */
        xassert(fur->domain != NULL);
        close_scope(mpl, fur->domain);
      }
      close_scope_remove_decl(mpl, fur->list);
      /* the for statement has been completely parsed */
      mpl->current_for_loop = prev_for_loop;
      return fur;
}

void break_continue_statement(MPL *mpl)
{
    if(!mpl->current_for_loop)
        error(mpl, "break/continue only allowed inside for loops");
    if (!(is_keyword(mpl, "break") || is_keyword(mpl, "continue")))
        error(mpl, "break/continue keywords expected");
    get_token(mpl /* break/continue */);
    /* the break/continue statement has been completely parsed */
    if (mpl->scan_input->token != T_SEMICOLON)
       error(mpl, "syntax error in break statement");
    get_token(mpl /* ; */);
    /* to check if we are in a repeat loop without a break */
    mpl->current_for_loop->do_break = -2;
}

/*----------------------------------------------------------------------
-- if_statement - parse if then else statement.
--
-- This routine parses for statement using the syntax:
--
-- <if statement> ::= if <logical expression> then <statement>
-- <if statement> ::= if <logical expression> then <statement> else <statement>
-- <if statement> ::= if <logical expression> then { <statement list> }
-- <if statement> ::= if <logical expression> then { <statement list> } else { <statement list> }
-- <statement list> ::= <empty>
-- <statement list> ::= <statement list> <statement>
-- <statement> ::= <check statement>
-- <statement> ::= <display statement>
-- <statement> ::= <printf statement>
-- <statement> ::= <if statement>
-- <statement> ::= <for statement> */

IF_STMT *if_statement(MPL *mpl)
{     IF_STMT *if_stmt;
      xassert(mpl->scan_input->token == T_IF);
      /* create for descriptor */
      if_stmt = alloc(IF_STMT);
      if_stmt->true_list = if_stmt->else_list = NULL;
      get_token(mpl /* if */);
      /* parse logical expression */
      if_stmt->code = expression_13(mpl);
      if(mpl->scan_input->token != T_THEN)
          error(mpl, "'then' keyword expected");
      get_token(mpl /* then */);
      /* parse true statement body */
      if (mpl->scan_input->token != T_LBRACE)
      {  /* parse simple statement */
         if_stmt->true_list = simple_statement(mpl, 1);
      }
      else
          if_stmt->true_list = parse_compound_statement(mpl);
      close_scope_remove_decl(mpl, if_stmt->true_list);

      if(mpl->scan_input->token == T_ELSE)
      {
        get_token(mpl /* else */);
        /* parse else statement body */
        if (mpl->scan_input->token != T_LBRACE)
        {  /* parse simple statement */
           if_stmt->else_list = simple_statement(mpl, 1);
        }
        else
            if_stmt->else_list = parse_compound_statement(mpl);
        close_scope_remove_decl(mpl, if_stmt->else_list);
      }
      /* the for statement has been completely parsed */
      return if_stmt;
}

/*----------------------------------------------------------------------
-- end_statement - parse end statement.
--
-- This routine parses end statement using the syntax:
--
-- <end statement> ::= end ; <eof> */

void end_statement(MPL *mpl)
{     if (!mpl->flag_d && is_keyword(mpl, "end") ||
           mpl->flag_d && is_literal(mpl, "end"))
      {  get_token(mpl /* end */);
         if (mpl->scan_input->token == T_SEMICOLON)
            get_token(mpl /* ; */);
         else
            warning(mpl, "no semicolon following end statement; missing"
               " semicolon inserted");
      }
      else
         warning(mpl, "unexpected end of file; missing end statement in"
            "serted");
      if (mpl->scan_input->token != T_EOF)
         warning(mpl, "some text detected beyond end statement; text ig"
            "nored");
      return;
}

/*----------------------------------------------------------------------
-- simple_statement - parse simple statement.
--
-- This routine parses simple statement using the syntax:
--
-- <statement> ::= <set statement>
-- <statement> ::= <parameter statement>
-- <statement> ::= <variable statement>
-- <statement> ::= <constraint statement>
-- <statement> ::= <objective statement>
-- <statement> ::= <solve statement>
-- <statement> ::= <check statement>
-- <statement> ::= <display statement>
-- <statement> ::= <printf statement>
-- <statement> ::= <for statement>
--
-- If the flag spec is set, some statements cannot be used. */

STATEMENT *simple_statement(MPL *mpl, int spec)
{     STATEMENT *stmt;
#if 0
      if (mpl->scan_input->token == T_LBRACE)
      {
          ++mpl->nested_scope;
          get_token(mpl /* { */);
      }
      else if (mpl->scan_input->token == T_RBRACE)
      {
          if(mpl->nested_scope == 0)
              error(mpl, "unmatched scope closing");
          --mpl->nested_scope;
          get_token(mpl /* } */);
      }
#endif
      stmt = alloc(STATEMENT);
      stmt->line = mpl->scan_input->line;
      stmt->next = NULL;
      if (is_keyword(mpl, "set"))
      {  if (spec && !mpl->nested_scope)
            error(mpl, "set statement not allowed here");
         stmt->type = A_SET;
         stmt->u.set = set_statement(mpl);
      }
      else if (is_keyword(mpl, "param"))
      {  if (spec && !mpl->nested_scope)
            error(mpl, "parameter statement not allowed here");
         stmt->type = A_PARAMETER;
         stmt->u.par = parameter_statement(mpl);
      }
      else if (is_keyword(mpl, "var"))
      {  if (spec)
            error(mpl, "variable statement not allowed here");
         stmt->type = A_VARIABLE;
         stmt->u.var = variable_statement(mpl);
         add_problem_element(mpl, stmt);
      }
      else if (is_keyword(mpl, "subject") ||
               is_keyword(mpl, "subj") ||
               mpl->scan_input->token == T_SPTP)
      {  if (spec)
            error(mpl, "constraint statement not allowed here");
         stmt->type = A_CONSTRAINT;
         stmt->u.con = constraint_statement(mpl);
         add_problem_element(mpl, stmt);
      }
      else if (is_keyword(mpl, "minimize") ||
               is_keyword(mpl, "maximize"))
      {  if (spec)
            error(mpl, "objective statement not allowed here");
         stmt->type = A_CONSTRAINT;
         stmt->u.con = objective_statement(mpl);
         add_problem_element(mpl, stmt);
      }
#if 1 /* 11/II-2008 */
      else if (is_keyword(mpl, "table"))
      {  if (spec)
            error(mpl, "table statement not allowed here");
         stmt->type = A_TABLE;
         stmt->u.tab = table_statement(mpl);
      }
#endif
      else if (is_keyword(mpl, "solve"))
      {
         stmt->type = A_SOLVE;
         stmt->u.slv = solve_statement(mpl);
      }
      else if (is_keyword(mpl, "check"))
      {  stmt->type = A_CHECK;
         stmt->u.chk = check_statement(mpl);
      }
      else if (is_keyword(mpl, "display"))
      {  stmt->type = A_DISPLAY;
         stmt->u.dpy = display_statement(mpl);
      }
      else if (is_keyword(mpl, "printf"))
      {  stmt->type = A_PRINTF;
         stmt->u.prt = printf_statement(mpl);
      }
      else if (is_keyword(mpl, "for"))
      {  stmt->type = A_FOR;
         stmt->u.fur = for_statement(mpl, 0);
         /* used to check reapeat */
         stmt->u.fur->do_break = 0;
      }
      else if (is_keyword(mpl, "repeat"))
      {  stmt->type = A_REPEAT;
         stmt->u.fur = for_statement(mpl, 1);
         if(!stmt->u.fur->do_break)
             error(mpl, "'repeat' statement without a break is an infinite loop");
         stmt->u.fur->do_break = 0;
      }
      else if (is_keyword(mpl, "break"))
      {  stmt->type = A_BREAK;
         break_continue_statement(mpl);
      }
      else if (is_keyword(mpl, "continue"))
      {  stmt->type = A_CONTINUE;
         break_continue_statement(mpl);
      }
      else if (mpl->scan_input->token == T_IF)
      {  stmt->type = A_IF;
         stmt->u.if_stmt = if_statement(mpl);
      }
      else if (is_keyword(mpl, "problem"))
      {  stmt->type = A_PROBLEM;
         stmt->u.prob = problem_statement(mpl);
      }
      else if (is_keyword(mpl, "let"))
      {  stmt->type = A_LET;
         stmt->u.let = let_statement(mpl);
      }
      else if (mpl->scan_input->token == T_NAME)
      {  if (spec)
            error(mpl, "constraint statement not allowed here");
         stmt->type = A_CONSTRAINT;
         stmt->u.con = constraint_statement(mpl);
      }
      else if (is_reserved(mpl))
         error(mpl, "invalid use of reserved keyword %s", mpl->scan_input->image);
      else
         error(mpl, "syntax error in model section");
      return stmt;
}

/*----------------------------------------------------------------------
-- model_section - parse model section.
--
-- This routine parses model section using the syntax:
--
-- <model section> ::= <empty>
-- <model section> ::= <model section> <statement>
--
-- Parsing model section is terminated by either the keyword 'data', or
-- the keyword 'end', or the end of file. */

void model_section(MPL *mpl)
{     STATEMENT *stmt, *last_stmt;
      xassert(mpl->model == NULL || mpl->nested_input > 1);
#define SET_STMT_TO_MODEL_LAST(s) \
      for(s = mpl->model; s && s->next; s = s->next)
      SET_STMT_TO_MODEL_LAST(last_stmt);
      while (mpl->scan_input->token != T_EOF)
      {  /* parse statement */
         if(is_keyword(mpl, "end")) break;
         else if(is_keyword(mpl, "data"))
         {
             if(data_statement(mpl)) continue;
             break;
         }
         else if(is_keyword(mpl, "model"))
         {
             model_statement(mpl);
             /* actualize our last_stmt */
             SET_STMT_TO_MODEL_LAST(last_stmt);
             continue;
         }
         stmt = simple_statement(mpl, 0);
         /* and append it to the end of the statement list */
         if (last_stmt == NULL)
            mpl->model = stmt;
         else
            last_stmt->next = stmt;
         last_stmt = stmt;
      }
      if(mpl->nested_scope)
              error(mpl, "unmatched scope open");
      return;
}

/* eof */
