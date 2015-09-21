/* EINA - EFL data type library
 * Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2010
 *                         Carsten Haitzler,
 *                         Jorge Luis Zapata Muga,
 *                         Cedric Bail,
 *                         Gustavo Sverzut Barbieri
 *                         Tom Hacohen
 *                         Brett Nash
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_log.h"
#include "eina_lock.h"
#include "eina_share_common.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_tmpstr.h"

typedef struct _Str Str;

struct _Str
{
   size_t length;
   Str *next;
   char *str;
};

static Eina_Lock _mutex;
static Str *strs = NULL;

Eina_Bool
eina_tmpstr_init(void)
{
   if (!eina_lock_new(&_mutex)) return EINA_FALSE;
   return EINA_TRUE;
}

Eina_Bool
eina_tmpstr_shutdown(void)
{
   eina_lock_free(&_mutex);
   return EINA_TRUE;
}

EAPI Eina_Tmpstr *
eina_tmpstr_add_length(const char *str, size_t length)
{
   Str *s;

   if (!str || !length) return NULL;
   s = malloc(sizeof(Str) + length + 1);
   if (!s) return NULL;
   s->length = length;
   s->str = ((char *)s) + sizeof(Str);
   strncpy(s->str, str, length);
   s->str[length] = '\0';
   eina_lock_take(&_mutex);
   s->next = strs;
   strs = s;
   eina_lock_release(&_mutex);
   return s->str;
}

EAPI Eina_Tmpstr *
eina_tmpstr_add(const char *str)
{
   size_t len;
   
   if (!str) return NULL;
   len = strlen(str);
   return eina_tmpstr_add_length(str, len);
}

EAPI void
eina_tmpstr_del(Eina_Tmpstr *tmpstr)
{
   Str *s, *sp;
   
   if ((!strs) || (!tmpstr)) return;
   eina_lock_take(&_mutex);
   for (sp = NULL, s = strs; s; sp = s, s = s->next)
     {
        if (s->str == tmpstr)
          {
             if (sp) sp->next = s->next;
             else strs = s->next;
             free(s);
             break;
          }
     }
   eina_lock_release(&_mutex);
}

EAPI size_t
eina_tmpstr_strlen(Eina_Tmpstr *tmpstr)
{
   if (!tmpstr) return 0;
   return eina_tmpstr_len(tmpstr) + 1;
}

EAPI size_t
eina_tmpstr_len(Eina_Tmpstr *tmpstr)
{
   Str *s;

   if (!tmpstr) return 0;
   if (!strs) return strlen(tmpstr);
   eina_lock_take(&_mutex);
   for (s = strs; s; s = s->next)
     {
        if (s->str == tmpstr)
          {
             size_t ret = s->length;
             eina_lock_release(&_mutex);
             return ret;
          }
     }
   eina_lock_release(&_mutex);

   return strlen(tmpstr);
}

EAPI Eina_Tmpstr *
eina_tmpstr_strftime(const char *format, const struct tm *tm)
{
   const size_t flen = strlen(format);
   size_t buflen = 16; // An arbitrary starting size
   char *buf = NULL;

   do {
      char *tmp;
      size_t len;

      tmp = realloc(buf, buflen * sizeof(char));
      if (!tmp) goto on_error;
      buf = tmp;

      len = strftime(buf, buflen, format, tm);
      // Check if we have the expected result and return it.
      if ((len > 0 && len < buflen) || (len == 0 && flen == 0))
        {
           Eina_Tmpstr *r;

           r = eina_tmpstr_add_length(buf, len + 1);
           free(buf);
           return r;
        }

      /* Possibly buf overflowed - try again with a bigger buffer */
      buflen <<= 1; // multiply buffer size by 2
   } while (buflen < 128 * flen);

 on_error:
   free(buf);
   return NULL;
}
