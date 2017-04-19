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
Evas_Object *win;

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
settings_font_cb (void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
    Ecrire_Entry *ent = (Ecrire_Entry *) data;
    ui_font_dialog_open (win, ent, ent_cfg->font.name, ent_cfg->font.size);
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

  win = elm_win_add(parent, "settings", ELM_WIN_TOOLBAR);
  elm_win_autodel_set(win, EINA_TRUE);
  elm_win_title_set(win, _("Settings"));
  evas_object_smart_callback_add(win, "delete,request", settings_delete_cb, NULL);

  table = elm_table_add(win);
  elm_obj_table_padding_set(table, ELM_SCALE_SIZE(PADDING), ELM_SCALE_SIZE(PADDING));
  evas_object_size_hint_padding_set (table, ELM_SCALE_SIZE(PADDING), ELM_SCALE_SIZE(PADDING),
                                     ELM_SCALE_SIZE(PADDING), ELM_SCALE_SIZE(PADDING));
  evas_object_show(table);
  

  /* Font Settings Label */
  obj = elm_label_add(table);
  elm_object_text_set(obj,"Font");
  evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(obj, 1, EVAS_HINT_FILL);
  elm_table_pack(table, obj, 0, row, 1, 1);
  evas_object_show(obj);

  /* Font Settings Button */
  obj = elm_button_add(table);
  elm_object_text_set(obj,"Edit");
  ic = elm_image_add (win);
  if (elm_icon_standard_set (ic,"format-text-italic")) {
    elm_object_content_set (obj, ic);
    evas_object_show(ic);
  } else
    evas_object_del(ic);
  evas_object_size_hint_align_set(obj, 0, 0);
  elm_table_pack(table, obj, 1, row, 1, 1);
  evas_object_size_hint_min_set(obj, ELM_SCALE_SIZE(BUTTON_WIDTH), ELM_SCALE_SIZE(BUTTON_HEIGHT));
  evas_object_smart_callback_add(obj, "clicked", settings_font_cb, ent);
  evas_object_show(obj);
  row++;

  /* Word Wrap Label */
  obj = elm_label_add(table);
  elm_object_text_set(obj,"Word Wrap");
  evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(obj, 1, 0);
  elm_table_pack(table, obj, 0, row, 1, 1);
  evas_object_show(obj);

  /* Word Wrap Check box */
  obj = elm_check_add(table);
  if(ent_cfg->wrap_type != ELM_WRAP_NONE)
    elm_check_state_set(obj, EINA_TRUE);
  evas_object_size_hint_align_set(obj, 0, 0);
  elm_table_pack(table, obj, 1, row, 1, 1);
  evas_object_smart_callback_add(obj, "changed", settings_word_wrap_cb, ent->entry);
  evas_object_show(obj);
  row++;

  /* Ok Button */
  obj = elm_button_add(table);
  elm_object_text_set(obj,"OK");
  ic = elm_image_add (win);
  if (elm_icon_standard_set (ic,"dialog-ok")) {
    elm_object_content_set (obj, ic);
    evas_object_show(ic);
  } else
    evas_object_del(ic);
  evas_object_size_hint_align_set(obj, 0, 0);
  elm_table_pack (table, obj, 0, row, 1, 1);
  evas_object_size_hint_min_set(obj, ELM_SCALE_SIZE(BUTTON_WIDTH), ELM_SCALE_SIZE(BUTTON_HEIGHT));
  evas_object_smart_callback_add (obj, "clicked", settings_delete_cb, NULL);
  evas_object_show (obj);
  
  /* Apply Button */
  obj = elm_button_add(table);
  elm_object_text_set(obj,"Apply");
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
  elm_object_text_set(obj,"Close");
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
