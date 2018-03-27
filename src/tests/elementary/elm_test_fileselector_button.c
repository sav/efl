#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST (elm_fileselector_button_legacy_type_check)
{
   Evas_Object *win, *fs_button;
   const char *type;

   win = elm_win_add(NULL, "fileselector_button", ELM_WIN_BASIC);

   fs_button = elm_fileselector_button_add(win);

   type = elm_object_widget_type_get(fs_button);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Fileselector_Button"));

   type = evas_object_type_get(fs_button);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_fileselector_button"));

}
EFL_END_TEST

EFL_START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *fs_button;
   Efl_Access_Role role;

   win = elm_win_add(NULL, "fileselector_button", ELM_WIN_BASIC);

   fs_button = elm_fileselector_button_add(win);
   role = efl_access_role_get(fs_button);

   ck_assert(role == EFL_ACCESS_ROLE_PUSH_BUTTON);

}
EFL_END_TEST

void elm_test_fileselector_button(TCase *tc)
{
   tcase_add_test(tc, elm_fileselector_button_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
