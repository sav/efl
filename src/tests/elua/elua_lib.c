#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include<Eina.h>
#include "Elua.h"
#include "elua_suite.h"

START_TEST(elua_api)
{
    Elua_State *st;
    lua_State *lst;

    fail_if(!elua_init());

    st = elua_state_new("test");
    fail_if(!st);

    /* test env vars */
    setenv("ELUA_CORE_DIR", "foo", 1);
    setenv("ELUA_MODULES_DIR", "bar", 1);
    setenv("ELUA_APPS_DIR", "baz", 1);
    elua_state_dirs_fill(st, EINA_FALSE);
    fail_if(strcmp(elua_state_core_dir_get(st), "foo"));
    fail_if(strcmp(elua_state_mod_dir_get(st), "bar"));
    fail_if(strcmp(elua_state_apps_dir_get(st), "baz"));

    /* now fill it properly */
    elua_state_dirs_set(st, ELUA_CORE_DIR, ELUA_MODULES_DIR, ELUA_APPS_DIR);
    fail_if(strcmp(elua_state_core_dir_get(st), ELUA_CORE_DIR));
    fail_if(strcmp(elua_state_mod_dir_get(st), ELUA_MODULES_DIR));
    fail_if(strcmp(elua_state_apps_dir_get(st), ELUA_APPS_DIR));

    lst = elua_state_lua_state_get(st);
    fail_if(!lst);

    fail_if(elua_state_from_lua_state_get(lst) != st);

    fail_if(strcmp(elua_state_prog_name_get(st), "test"));

    elua_state_free(st);

    elua_shutdown();
}
END_TEST

void elua_lib_test(TCase *tc)
{
   tcase_add_test(tc, elua_api);
}

