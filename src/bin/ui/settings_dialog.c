#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Elementary.h>

#include "../mess_header.h"
#include "../cfg.h"

const static int PADDING = 5;
const static int BUTTON_HEIGHT = 27;
const static int BUTTON_WIDTH = 60;

Ent_Cfg *ent_cfg;
Evas_Object *fsize, *list, *win;

static void
settings_alpha_cb (void *data,
                   Evas_Object *obj,
                   void *event_info EINA_UNUSED)
{
  Ecrire_Entry *ent = (Ecrire_Entry *)data;
  Evas_Object *bg;

  ent_cfg->alpha = elm_slider_value_get (obj);
  ALPHA (ent->bg, ent_cfg->alpha);
  ALPHA (ent->bx, ent_cfg->alpha);
}

static void
settings_apply_cb (void *data EINA_UNUSED,
                   Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   ecrire_cfg_save();
}

static void
settings_delete_cb (void *data EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   evas_object_del(win);
   win = NULL;
}

static void
settings_default_font_cb(void *data EINA_UNUSED,
                         Evas_Object *obj,
                         void *event_info EINA_UNUSED)
{
   elm_object_disabled_set(list, elm_check_state_get(obj));
   elm_object_disabled_set(fsize, elm_check_state_get(obj));
}

static Eina_List *
settings_font_list_get(const Evas *e)
{
   Eina_List *flist = evas_font_available_list(e);
   Eina_List *itr, *nitr;
   const char *font, *prev_font = NULL;
   flist = eina_list_sort(flist, eina_list_count(flist),
         (Eina_Compare_Cb) strcasecmp);
   EINA_LIST_FOREACH_SAFE(flist, itr, nitr, font)
     {
        Elm_Font_Properties *efp;

        efp = elm_font_properties_get(font);
        /* Remove dups */
        if (prev_font && !strcmp(efp->name, prev_font))
          {
             flist = eina_list_remove_list(flist, itr);
          }
        else
          {
             eina_stringshare_replace(&font, efp->name);
             prev_font = font;
             eina_list_data_set(itr, font);
          }

        elm_font_properties_free(efp);
     }

   return flist;
}

static void
settings_word_wrap_cb (void *data,
                       Evas_Object *obj,
                       void *event_info EINA_UNUSED)
{
  if(elm_check_state_get(obj))
    {
      elm_entry_line_wrap_set ((Evas_Object *)data, ELM_WRAP_WORD);
      ent_cfg->wrap_type = ELM_WRAP_WORD;
    }
  else
    {
      elm_entry_line_wrap_set ((Evas_Object *)data, ELM_WRAP_NONE);
      ent_cfg->wrap_type = ELM_WRAP_NONE;
    }
}

Evas_Object *
ui_settings_dialog_open(Evas_Object *parent,
                        Ecrire_Entry *ent,
                        Ent_Cfg *_ent_cfg)
{
  ent_cfg = _ent_cfg;
  Evas_Object *entry = ent->entry;
  Evas_Object *ic, *obj, *table;
  int row = 0;

   if (win)
     return win;

  win = elm_win_util_dialog_add (parent, _("ecrire"),  _("Settings"));
  elm_win_autodel_set(win, EINA_TRUE);
  evas_object_smart_callback_add(win, "delete,request", settings_delete_cb, NULL);

  table = elm_table_add(win);
  elm_obj_table_padding_set(table, ELM_SCALE_SIZE(PADDING), ELM_SCALE_SIZE(PADDING));
  evas_object_size_hint_padding_set (table, ELM_SCALE_SIZE(PADDING), ELM_SCALE_SIZE(PADDING),
                                     ELM_SCALE_SIZE(PADDING), ELM_SCALE_SIZE(PADDING));
  evas_object_show(table);

  /* Alpha Label */
  obj = elm_label_add (table);
  elm_object_text_set (obj,  _("Alpha"));
  evas_object_size_hint_weight_set (obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set (obj, 1, 0);
  elm_table_pack (table, obj, 0, row, 1, 1);
  evas_object_show (obj);
  
  /* Alpha Slider */
  obj = elm_slider_add (table);
  elm_slider_horizontal_set (obj, EINA_TRUE);
  elm_slider_min_max_set (obj, 127, 255);
  elm_slider_value_set (obj, ent_cfg->alpha);
  evas_object_size_hint_weight_set (obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set (obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_table_pack (table, obj, 1, row, 2, 1);
  evas_object_smart_callback_add (obj, "changed", settings_alpha_cb, ent);
  evas_object_show (obj);
  row++;

  /* Word Wrap Label */
  obj = elm_label_add(table);
  elm_object_text_set(obj, _("Word Wrap"));
  evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(obj, 1, 0);
  elm_table_pack(table, obj, 0, row, 1, 1);
  evas_object_show(obj);

  /* Word Wrap Check box */
  obj = elm_check_add(table);
  if(ent_cfg->wrap_type != ELM_WRAP_NONE)
    elm_check_state_set(obj, EINA_TRUE);
  evas_object_size_hint_align_set(obj, 0, 1);
  elm_table_pack(table, obj, 1, row, 1, 1);
  evas_object_smart_callback_add(obj, "changed", settings_word_wrap_cb, ent->entry);
  evas_object_show(obj);
  row++;

  /* Use default font Label */
  obj = elm_label_add(table);
  elm_object_text_set(obj, _("Use default font"));
  evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(obj, 1, 0);
  elm_table_pack(table, obj, 0, row, 1, 1);
  evas_object_show(obj);

  /* Use default font Check box */
  obj = elm_check_add(table);
  if(!ent_cfg->font.name || !ent_cfg->font.size)
    elm_check_state_set(obj, EINA_TRUE);
  evas_object_size_hint_align_set(obj, 0, 1);
  elm_table_pack(table, obj, 1, row, 1, 1);
  evas_object_smart_callback_add(obj, "changed", settings_default_font_cb, NULL);
  evas_object_show(obj);
  row++;

  /* Font Size Label */
  obj = elm_label_add(table);
  elm_object_text_set(obj, _("Font size"));
  evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(obj, 1, EVAS_HINT_FILL);
  elm_table_pack (table, obj, 0, row, 1, 1);
  evas_object_show(obj);

  /* Font Size Spinner */
  fsize = elm_spinner_add(table);
  if(!ent_cfg->font.name || !ent_cfg->font.size)
    elm_object_disabled_set(fsize, EINA_TRUE);
  elm_spinner_label_format_set(fsize, _("%.0f pts"));
  elm_spinner_step_set(fsize, 1);
  elm_spinner_wrap_set(fsize, EINA_FALSE);
  elm_object_style_set (fsize, "vertical");
  elm_spinner_min_max_set(fsize, 0, 72);
  elm_spinner_value_set(fsize, ent_cfg->font.size);
  evas_object_size_hint_align_set(fsize, 0.0, 0.5);
  elm_table_pack (table, fsize, 1, row, 2, 1);
  evas_object_show(fsize);
  row++;

  /* Fonts box */
   obj = elm_box_add(table);
   elm_box_horizontal_set(obj, EINA_TRUE);
   evas_object_size_hint_min_set(obj, 1, ELM_SCALE_SIZE(300));
   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack (table, obj, 0, row, 3, 1);
   evas_object_show(obj);
   row++;
   
  /* Fonts List */
  list = elm_list_add(table);
  if(!ent_cfg->font.name || !ent_cfg->font.size)
    elm_object_disabled_set(list, EINA_TRUE);
  evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_box_pack_end (obj, list);
  evas_object_show(list);

  /* Populate list */
    {
       Elm_Object_Item *cur_font = NULL;
       const char *font;
       Eina_List *flist, *itr;
        flist = settings_font_list_get(evas_object_evas_get(list));
       EINA_LIST_FOREACH(flist, itr, font)
         {
            Elm_Object_Item *tmp;
            tmp = elm_list_item_append(list, font, NULL, NULL, NULL, NULL);
            if (ent_cfg->font.name && !strcmp(ent_cfg->font.name, font))
              {
                 cur_font = tmp;
              }
         }
        EINA_LIST_FREE(flist, font)
          eina_stringshare_del(font);
        if (cur_font)
         {
            elm_list_item_bring_in(cur_font);
            elm_list_item_selected_set(cur_font, EINA_TRUE);
         }
    }

  /* Ok Button */
  obj = elm_button_add(table);
  elm_object_text_set(obj,_("OK"));
  ic = elm_image_add (win);
  if (elm_icon_standard_set (ic,"dialog-ok")) {
    elm_object_content_set (obj, ic);
    evas_object_show(ic);
  } else
    evas_object_del(ic);
  evas_object_size_hint_align_set(obj, 1, 0);
  elm_table_pack (table, obj, 0, row, 1, 1);
  evas_object_size_hint_min_set(obj, ELM_SCALE_SIZE(BUTTON_WIDTH), ELM_SCALE_SIZE(BUTTON_HEIGHT));
  evas_object_smart_callback_add (obj, "clicked", settings_delete_cb, NULL);
  evas_object_show (obj);
  
  /* Apply Button */
  obj = elm_button_add(table);
  elm_object_text_set(obj,_("Apply"));
  ic = elm_image_add (win);
  if (elm_icon_standard_set (ic,"dialog-apply")) {
    elm_object_content_set (obj, ic);
    evas_object_show(ic);
  } else
    evas_object_del(ic);
  evas_object_size_hint_align_set(obj, 0, 0);
  elm_table_pack (table, obj, 1, row, 1, 1);
  evas_object_size_hint_min_set(obj, ELM_SCALE_SIZE(BUTTON_WIDTH), ELM_SCALE_SIZE(BUTTON_HEIGHT));
  evas_object_smart_callback_add (obj, "clicked", settings_apply_cb, NULL);
  evas_object_show (obj);

  /* Close Button */
  obj = elm_button_add(table);
  elm_object_text_set(obj,_("Close"));
  ic = elm_image_add (win);
  if (elm_icon_standard_set (ic,"dialog-close")) {
    elm_object_content_set (obj, ic);
    evas_object_show(ic);
  } else
    evas_object_del(ic);
  evas_object_size_hint_align_set(obj, 0, 0);
  elm_table_pack (table, obj, 2, row, 1, 1);
  evas_object_size_hint_min_set(obj, ELM_SCALE_SIZE(BUTTON_WIDTH), ELM_SCALE_SIZE(BUTTON_HEIGHT));
  evas_object_smart_callback_add (obj, "clicked", settings_delete_cb, NULL);
  evas_object_show (obj);

  /* Box for padding */
  obj = elm_box_add (win);
  elm_box_pack_end (obj, table);
  evas_object_show (obj);

  /* Forcing it to be the min height. */
  elm_win_resize_object_add (win, obj);
  elm_win_raise (win);
  evas_object_show (win);

  return win;
}
