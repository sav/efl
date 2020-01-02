#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "elm_priv.h"


typedef struct {
   Efl_Ui_Spotlight_Container * container;
   Efl_Canvas_Animation_Player *alpha_anim;
   Efl_Gfx_Entity *content[2];
   int ids[2]; //only used when in animation
   Eina_Size2D page_size;
   Eina_Bool animation;
} Efl_Ui_Spotlight_Manager_Stack_Data;

#define MY_CLASS EFL_UI_SPOTLIGHT_MANAGER_STACK_CLASS

static void
_geom_sync(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Manager_Stack_Data *pd)
{
   Eina_Rect group_pos = efl_gfx_entity_geometry_get(pd->container);
   Eina_Rect goal = EINA_RECT_EMPTY();
   goal.size = pd->page_size;
   goal.y = (group_pos.y + group_pos.h/2)-pd->page_size.h/2;
   goal.x = (group_pos.x + group_pos.w/2)-pd->page_size.w/2;
   for (int i = 0; i < 2; ++i)
     {
        if (pd->content[i])
          efl_gfx_entity_geometry_set(pd->content[i], goal);
     }
}

static void
_running_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Ui_Spotlight_Manager_Stack_Data *pd = efl_data_scope_safe_get(data, MY_CLASS);
   double absolut_position;

   EINA_SAFETY_ON_NULL_RETURN(pd);
   //calculate absolut position, multiply pos with 2.0 because duration is only 0.5)
   absolut_position = pd->ids[0] + (pd->ids[1] - pd->ids[0])*(efl_canvas_object_animation_progress_get(ev->object));
   efl_event_callback_call(data, EFL_UI_SPOTLIGHT_MANAGER_EVENT_POS_UPDATE, &absolut_position);
}

static void
_hide_object_cb(void *data, const Efl_Event *ev)
{
   if (!ev->info)
     {
        efl_gfx_entity_visible_set(ev->object, EINA_FALSE);
        efl_event_callback_del(ev->object, ev->desc, _hide_object_cb, data);
        efl_event_callback_del(ev->object, EFL_CANVAS_OBJECT_ANIMATION_EVENT_ANIMATION_PROGRESS_UPDATED, _running_cb, data);
     }
}

EOLIAN static void
_efl_ui_spotlight_manager_stack_efl_ui_spotlight_manager_bind(Eo *obj, Efl_Ui_Spotlight_Manager_Stack_Data *pd, Efl_Ui_Spotlight_Container *spotlight)
{
   if (spotlight)
     {
        pd->container = spotlight;

        pd->alpha_anim = efl_add(EFL_CANVAS_ANIMATION_ALPHA_CLASS, obj);
        efl_animation_alpha_set(pd->alpha_anim, 0.0, 1.0);
        efl_animation_duration_set(pd->alpha_anim, 0.5);

        for (int i = 0; i < efl_content_count(spotlight) ; ++i) {
           Efl_Gfx_Entity *elem = efl_pack_content_get(spotlight, i);
           efl_canvas_group_member_add(pd->container, elem);
           efl_gfx_entity_visible_set(elem, EINA_FALSE);
        }
        if (efl_ui_spotlight_active_element_get(spotlight))
          {
             pd->content[0] = efl_ui_spotlight_active_element_get(spotlight);
             efl_gfx_entity_visible_set(pd->content[0], EINA_TRUE);
             _geom_sync(obj, pd);
          }
     }
}

static void
_update_ids(Eo *obj, Efl_Ui_Spotlight_Manager_Stack_Data *pd, int avoid_index)
{
   for (int i = 0; i < 2; ++i)
     {
        if (pd->ids[i] != -1 && pd->ids[i] != avoid_index)
          pd->ids[i] = efl_pack_index_get(pd->container, pd->content[i]);
     }
   double pos = pd->ids[1];
   efl_event_callback_call(obj, EFL_UI_SPOTLIGHT_MANAGER_EVENT_POS_UPDATE, &pos);
}

EOLIAN static void
_efl_ui_spotlight_manager_stack_efl_ui_spotlight_manager_content_add(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Manager_Stack_Data *pd, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   efl_canvas_group_member_add(pd->container, subobj);
   efl_gfx_entity_visible_set(subobj, EINA_FALSE);
   _update_ids(obj, pd, -1);
}

EOLIAN static void
_efl_ui_spotlight_manager_stack_efl_ui_spotlight_manager_content_del(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Manager_Stack_Data *pd, Efl_Gfx_Entity *subobj, int index)
{
   efl_canvas_group_member_remove(pd->container, subobj);
   for (int i = 0; i < 2; ++i)
     {
        if (pd->content[i] == subobj)
          pd->content[i] = NULL;
     }
   _update_ids(obj, pd, index);
}

static Eina_Bool
is_valid(Eo *obj, int index)
{
   if (index < 0) return EINA_FALSE;
   if (index >= efl_content_count(obj)) return EINA_FALSE;

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_spotlight_manager_stack_efl_ui_spotlight_manager_switch_to(Eo *obj, Efl_Ui_Spotlight_Manager_Stack_Data *pd, int from, int to)
{
   if (efl_pack_content_get(pd->container, to) == pd->content[1])
     return;

   if (is_valid(pd->container, to) && is_valid(pd->container, from))
     {
        int tmp[2] = {from, to};

        for (int i = 0; i < 2; ++i)
          {
             pd->ids[i] = tmp[i];
             pd->content[i] = efl_pack_content_get(pd->container, pd->ids[i]);
             if (pd->animation)
               efl_canvas_object_animation_start(pd->content[i], pd->alpha_anim, -1.0+2.0*i, 0.0);
             efl_gfx_entity_visible_set(pd->content[i], EINA_TRUE);
          }
        if (pd->animation)
          {
             efl_event_callback_add(pd->content[0], EFL_CANVAS_OBJECT_ANIMATION_EVENT_ANIMATION_CHANGED, _hide_object_cb, obj);
             efl_event_callback_add(pd->content[0], EFL_CANVAS_OBJECT_ANIMATION_EVENT_ANIMATION_PROGRESS_UPDATED, _running_cb, obj);
          }
     }
   else
     {
        double pos = to;

        pd->ids[0] = -1;
        pd->content[0] = NULL;
        pd->ids[1] = to;
        pd->content[1] = efl_pack_content_get(pd->container, to);
        efl_gfx_entity_visible_set(pd->content[1], EINA_TRUE);
        efl_event_callback_call(obj, EFL_UI_SPOTLIGHT_MANAGER_EVENT_POS_UPDATE, &pos);
     }

   _geom_sync(obj, pd);
}

EOLIAN static void
_efl_ui_spotlight_manager_stack_efl_ui_spotlight_manager_size_set(Eo *obj, Efl_Ui_Spotlight_Manager_Stack_Data *pd, Eina_Size2D size)
{
   pd->page_size = size;
   _geom_sync(obj, pd);
}

EOLIAN static void
_efl_ui_spotlight_manager_stack_efl_object_invalidate(Eo *obj, Efl_Ui_Spotlight_Manager_Stack_Data *pd EINA_UNUSED)
{
   efl_invalidate(efl_super(obj, MY_CLASS));

   for (int i = 0; i < efl_content_count(pd->container); ++i)
     {
        Efl_Gfx_Stack *elem = efl_pack_content_get(pd->container, i);
        for (int d = 0; d < 4; d++)
          {
             efl_gfx_mapping_color_set(elem, d, 255, 255, 255, 255);
          }
     }
}

EOLIAN static void
_efl_ui_spotlight_manager_stack_efl_ui_spotlight_manager_animated_transition_set(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Manager_Stack_Data *pd, Eina_Bool animation)
{
   for (int i = 0; i < 2; ++i)
     {
        if (pd->content[i])
          efl_canvas_object_animation_stop(pd->content[i]);
     }
   pd->animation = animation;
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_manager_stack_efl_ui_spotlight_manager_animated_transition_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Manager_Stack_Data *pd)
{
   return pd->animation;
}


#include "efl_ui_spotlight_manager_stack.eo.c"
