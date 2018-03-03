#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Elementary.h>

#include "../ecrire.h"
#include "../cfg.h"

const static int PADDING = 5;
const static int BUTTON_HEIGHT = 27;
const static int BUTTON_WIDTH = 60;

Ent_Cfg *ent_cfg;
Evas_Object *fsize;
Evas_Object *list;

static void
disable_font_widgets(Eina_Bool state)
{
  const Eina_List *items;
  const Eina_List *itr;
  void *item;

  elm_object_disabled_set(fsize, state);
  items = elm_list_items_get(list);
  EINA_LIST_FOREACH(items, itr, item)
    elm_object_item_disabled_set((Elm_Object_Item *)item, state);
}

static void
settings_alpha_cb (void *data,
                   Evas_Object *obj,
                   void *event_info EINA_UNUSED)
{
  Ecrire_Doc *doc = data;

  ent_cfg->alpha = elm_slider_value_get (obj);
  ALPHA (doc->bg, ent_cfg->alpha);
  ALPHA (doc->box_main, ent_cfg->alpha);
  ALPHA (doc->box_editor, ent_cfg->alpha);
  ecrire_cfg_save();
}

static void
_settings_animate_open_cb (void *data,
                           Evas_Object *obj,
                           void *event_info EINA_UNUSED)
{
  Eina_Bool state;

  state = elm_check_state_get(obj);
  ent_cfg->anim_open = !state;
  ecrire_cfg_save();
}

static void
settings_apply_font_cb (void *data,
                        Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
  Elm_Object_Item *list_it = elm_list_selected_item_get(list);
  const char *name = elm_object_item_text_get(list_it);
  unsigned int size = elm_spinner_value_get(fsize);
  editor_font_set((Ecrire_Doc *)data, name, size);
  if(name)
    ent_cfg->font.name = name;
  ent_cfg->font.size = size;
  ecrire_cfg_save();
}

static void
settings_delete_cb (void *data,
                    Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   Evas_Object *win;

   win = (Evas_Object *)data;
   evas_object_del(win);
   win = NULL;
}

static void
settings_default_font_cb(void *data,
                         Evas_Object *obj,
                         void *event_info EINA_UNUSED)
{
  Ecrire_Doc *doc = data;
  Eina_Bool state = elm_check_state_get(obj);
  disable_font_widgets(state);
  if(state)
    {
      elm_obj_code_widget_font_set(doc->widget, NULL, 10);
      ent_cfg->font.name = NULL;
      ent_cfg->font.size = 10;
      ecrire_cfg_save();
    }
  else
    settings_apply_font_cb(doc, (Evas_Object *)NULL, (void *)NULL);
}

static void
settings_show_line_width_marker_cb(void *data,
                                   Evas_Object *obj,
                                   void *event_info EINA_UNUSED)
{
  Ecrire_Doc *doc = data;
  Eina_Bool state = elm_check_state_get(obj);
  unsigned int width = 0;
  if(state)
    width = ECRIRE_LINE_WIDTH;
  elm_code_widget_line_width_marker_set(doc->widget, width);
  ent_cfg->line_width_marker = width;
  ecrire_cfg_save();
}

static Eina_List *
settings_font_list_get(const Evas *e)
{
   Eina_List *flist = evas_font_available_list(e);
   Eina_List *itr;
   Eina_List *nitr;
   const char *font;
   const char *prev_font = NULL;
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
_settings_insert_spaces_cb (void *data,
                            Evas_Object *obj,
                            void *event_info EINA_UNUSED)
{
  Eina_Bool state;

  state = elm_check_state_get(obj);
  elm_obj_code_widget_tab_inserts_spaces_set((Elm_Code_Widget *)data, state);
  ent_cfg->insert_spaces = !state;
  ecrire_cfg_save();
}

static void
settings_line_numbers_cb (void *data,
                          Evas_Object *obj,
                          void *event_info EINA_UNUSED)
{
  Eina_Bool state;

  state = elm_check_state_get (obj);
  elm_obj_code_widget_line_numbers_set ((Elm_Code_Widget *)data, state);
  ent_cfg->line_numbers = !state;
  ecrire_cfg_save();
}

static void
settings_menu_cb (void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
  Eina_Bool state;
  Evas_Object *win;

  state = elm_check_state_get (obj);
  win = (Evas_Object *)data;
  if(!state)
    evas_object_del(elm_win_main_menu_get(win));
  ent_cfg->menu = !state;
  ecrire_cfg_save();
}

static void
settings_toolbar_cb (void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
  Eina_Bool state;

  state = elm_check_state_get (obj);
  if(state)
    add_toolbar((Ecrire_Doc *)data);
  else
    evas_object_del(((Ecrire_Doc *)data)->toolbar);
  ent_cfg->toolbar = !state;
  ecrire_cfg_save();
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
  ecrire_cfg_save();
}

Evas_Object *
_settings_dialog_display(Evas_Object *parent,
                         Ecrire_Doc *doc,
                         Ent_Cfg *_ent_cfg)
{
  ent_cfg = _ent_cfg;
  Evas_Object *boxv;
  Evas_Object *obj;
  Evas_Object *table;
  int row = 0;

  boxv = elm_box_add(parent);
  evas_object_size_hint_weight_set(boxv, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(boxv, EVAS_HINT_FILL, EVAS_HINT_FILL);
  evas_object_show(boxv);

  table = elm_table_add(boxv);
  elm_table_padding_set(table,ELM_SCALE_SIZE(PADDING),ELM_SCALE_SIZE(PADDING));
  evas_object_size_hint_padding_set (table, ELM_SCALE_SIZE(PADDING), ELM_SCALE_SIZE(PADDING),
                                     ELM_SCALE_SIZE(PADDING), ELM_SCALE_SIZE(PADDING));
  evas_object_size_hint_weight_set(table, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(table, 0, 0);
  elm_box_pack_end (boxv, table);
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
  elm_table_pack (table, obj, 1, row, 3, 1);
  evas_object_smart_callback_add (obj, "changed", settings_alpha_cb, doc);
  evas_object_show (obj);
  row++;

  /* Animate Open Label */
  obj = elm_label_add(table);
  elm_object_text_set(obj, _("Animate Open"));
  evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(obj, 1, 0);
  elm_table_pack(table, obj, 0, row, 1, 1);
  evas_object_show(obj);

  /* Animate Open Check box */
  obj = elm_check_add(table);
  elm_check_state_set(obj, !_ent_cfg->anim_open);
  evas_object_size_hint_align_set(obj, 0, 1);
  elm_table_pack(table, obj, 1, row, 1, 1);
  evas_object_smart_callback_add(obj, "changed", _settings_animate_open_cb, doc->widget);
  evas_object_show(obj);
  row++;

  /* Insert Spaces Label */
  obj = elm_label_add(table);
  elm_object_text_set(obj, _("Tab inserts spaces"));
  evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(obj, 1, 0);
  elm_table_pack(table, obj, 0, row, 1, 1);
  evas_object_show(obj);

  /* Insert Spaces Check box */
  obj = elm_check_add(table);
  elm_check_state_set(obj, !_ent_cfg->insert_spaces);
  evas_object_size_hint_align_set(obj, 0, 1);
  elm_table_pack(table, obj, 1, row, 1, 1);
  evas_object_smart_callback_add(obj, "changed", _settings_insert_spaces_cb, doc->widget);
  evas_object_show(obj);
  row++;

  /* Line Numbers Label */
  obj = elm_label_add(table);
  elm_object_text_set(obj, _("Line Numbers"));
  evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(obj, 1, 0);
  elm_table_pack(table, obj, 0, row, 1, 1);
  evas_object_show(obj);

  /* Line Numbers Check box */
  obj = elm_check_add(table);
  elm_check_state_set(obj, !_ent_cfg->line_numbers);
  evas_object_size_hint_align_set(obj, 0, 1);
  elm_table_pack(table, obj, 1, row, 1, 1);
  evas_object_smart_callback_add(obj, "changed", settings_line_numbers_cb, doc->widget);
  evas_object_show(obj);
  row++;

  /* Show line width marker label */
  obj = elm_label_add(table);
  elm_object_text_set(obj, _("Line Width Marker"));
  evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(obj, 1, 0);
  elm_table_pack(table, obj, 0, row, 1, 1);
  evas_object_show(obj);

  /* Show line width marker check box */
  obj = elm_check_add(table);
  if(ent_cfg->line_width_marker>0)
    elm_check_state_set(obj, EINA_TRUE);
  evas_object_size_hint_align_set(obj, 0, 1);
  elm_table_pack(table, obj, 1, row, 1, 1);
  evas_object_smart_callback_add(obj, "changed", settings_show_line_width_marker_cb, doc);
  evas_object_show(obj);
  row++;

  /* Show Menu Label */
  obj = elm_label_add(table);
  elm_object_text_set(obj, _("Show Menu"));
  evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(obj, 1, 0);
  elm_table_pack(table, obj, 0, row, 1, 1);
  evas_object_show(obj);

  /* Menu Check box */
  obj = elm_check_add(table);
  elm_check_state_set(obj, !_ent_cfg->menu);
  evas_object_size_hint_align_set(obj, 0, 1);
  elm_table_pack(table, obj, 1, row, 1, 1);
  evas_object_smart_callback_add(obj, "changed", settings_menu_cb, doc->win);
  evas_object_show(obj);
  row++;

  /* Show Toolbar Label */
  obj = elm_label_add(table);
  elm_object_text_set(obj, _("Show Toolbar"));
  evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(obj, 1, 0);
  elm_table_pack(table, obj, 0, row, 1, 1);
  evas_object_show(obj);

  /* Menu Toolbar box */
  obj = elm_check_add(table);
  elm_check_state_set(obj, !_ent_cfg->toolbar);
  evas_object_size_hint_align_set(obj, 0, 1);
  elm_table_pack(table, obj, 1, row, 1, 1);
  evas_object_smart_callback_add(obj, "changed", settings_toolbar_cb, doc);
  evas_object_show(obj);
  row++;

  /* Word Wrap Label */
/* FIXME: Hide till elm_code supports line wrapping
  obj = elm_label_add(table);
  elm_object_text_set(obj, _("Word Wrap"));
  evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(obj, 1, 0);
  elm_table_pack(table, obj, 0, row, 1, 1);
  evas_object_show(obj);
*/

  /* Word Wrap Check box */
/* FIXME: Hide till elm_code supports line wrapping
  obj = elm_check_add(table);
  if(ent_cfg->wrap_type != ELM_WRAP_NONE)
    elm_check_state_set(obj, EINA_TRUE);
  evas_object_size_hint_align_set(obj, 0, 1);
  elm_table_pack(table, obj, 1, row, 1, 1);
  evas_object_smart_callback_add(obj, "changed", settings_word_wrap_cb, doc->widget);
  evas_object_show(obj);
  row++;
*/

  return(boxv);
}

Evas_Object *
_settings_dialog_font(Evas_Object *parent, Ecrire_Doc *doc, Ent_Cfg *_ent_cfg)
{
  ent_cfg = _ent_cfg;
  Evas_Object *boxv;
  Evas_Object *obj;
  Evas_Object *table;
  int row = 0;

  boxv = elm_box_add(parent);
  evas_object_size_hint_weight_set(boxv, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(boxv, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_box_horizontal_set(boxv, EINA_FALSE);
  evas_object_show(boxv);

  table = elm_table_add(boxv);
  elm_table_padding_set(table,ELM_SCALE_SIZE(PADDING),ELM_SCALE_SIZE(PADDING));
  evas_object_size_hint_padding_set (table, ELM_SCALE_SIZE(PADDING), ELM_SCALE_SIZE(PADDING),
                                     ELM_SCALE_SIZE(PADDING), ELM_SCALE_SIZE(PADDING));
  evas_object_size_hint_align_set(table, 0, 0);
  elm_box_pack_end (boxv, table);
  evas_object_show(table);

  /* Use default font Label */
  obj = elm_label_add(table);
  elm_object_text_set(obj, _("Use default font"));
  evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(obj, 1, 0);
  elm_table_pack(table, obj, 0, row, 1, 1);
  evas_object_show(obj);

  /* Use default font Check box */
  obj = elm_check_add(table);
  if(!ent_cfg->font.name)
    elm_check_state_set(obj, EINA_TRUE);
  evas_object_size_hint_align_set(obj, 0, 1);
  elm_table_pack(table, obj, 1, row, 1, 1);
  evas_object_smart_callback_add(obj, "changed", settings_default_font_cb, doc);
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
  if(!ent_cfg->font.name)
    disable_font_widgets(EINA_TRUE);
  elm_spinner_label_format_set(fsize, _("%.0f pts"));
  elm_spinner_step_set(fsize, 1);
  elm_spinner_wrap_set(fsize, EINA_FALSE);
  elm_object_style_set (fsize, "vertical");
  elm_spinner_min_max_set(fsize, 1, 72);
  elm_spinner_value_set(fsize, ent_cfg->font.size);
  evas_object_size_hint_align_set(fsize, 0.0, 0.5);
  elm_table_pack (table, fsize, 1, row, 2, 1);
  evas_object_smart_callback_add(fsize, "changed", settings_apply_font_cb, doc);
  evas_object_show(fsize);

  /* Fonts List */
  list = elm_list_add(boxv);
  if(!ent_cfg->font.name)
    disable_font_widgets(EINA_TRUE);
  evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_box_pack_end (boxv, list);
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
            if (ent_cfg->font.name)
              {
                if(!strcmp(ent_cfg->font.name, font))
                  cur_font = tmp;
              }
            else
              elm_object_item_disabled_set(tmp, EINA_TRUE);
         }
        EINA_LIST_FREE(flist, font)
          eina_stringshare_del(font);
        if (cur_font)
         {
            elm_list_item_bring_in(cur_font);
            elm_list_item_selected_set(cur_font, EINA_TRUE);
         }
    }
  evas_object_smart_callback_add(list, "selected", settings_apply_font_cb, doc);

  return(boxv);
}

static void
_settings_dialog_promote_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   elm_naviframe_item_promote(data);
}

Evas_Object *
ui_settings_dialog_open(Evas_Object *parent, Ecrire_Doc *doc, Ent_Cfg *_ent_cfg)
{
  ent_cfg = _ent_cfg;
  Evas_Object *boxh;
  Evas_Object *boxv;
  Evas_Object *ic;
  Evas_Object *obj;
  Evas_Object *nf;
  Evas_Object *navi_disp;
  Evas_Object *navi_font;
  Evas_Object *tb;
  Evas_Object *win;
  unsigned int h;
  unsigned int w;


  win = elm_win_util_dialog_add (parent, _("ecrire"),  _("Settings"));
  elm_win_autodel_set(win, EINA_TRUE);
  evas_object_smart_callback_add(win, "delete,request", settings_delete_cb, NULL);
  evas_object_geometry_get(doc->win, NULL, NULL, &w, &h);

  boxh = elm_box_add(win);
  evas_object_size_hint_weight_set(boxh, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(boxh, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_box_horizontal_set(boxh, EINA_TRUE);
  evas_object_show(boxh);

  boxv = elm_box_add(win);
  evas_object_size_hint_weight_set(boxv, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(boxv, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_box_horizontal_set(boxv, EINA_FALSE);
  elm_win_resize_object_add(win, boxv);
  evas_object_show(boxv);

  tb = elm_toolbar_add(win);
  evas_object_size_hint_weight_set(tb, 0, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_toolbar_horizontal_set(tb, EINA_FALSE);
  elm_toolbar_select_mode_set(tb, ELM_OBJECT_SELECT_MODE_ALWAYS);
  elm_box_pack_end(boxh, tb);
  evas_object_show(tb);

  nf = elm_naviframe_add(win);
  evas_object_size_hint_weight_set(nf, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(nf, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_naviframe_prev_btn_auto_pushed_set(nf, EINA_FALSE);
  elm_box_pack_end(boxh, nf);
  evas_object_show(nf);

  obj = _settings_dialog_font(win, doc, ent_cfg);
  navi_font = elm_naviframe_item_push(nf, _("Font Settings"), NULL, NULL, obj, NULL);

  obj = _settings_dialog_display(win, doc, ent_cfg);
  navi_disp = elm_naviframe_item_push(nf, _("Display Settings"), NULL, NULL, obj, NULL);

  elm_toolbar_item_prepend(tb,
                           "preferences-desktop-display",
                           _("Display"),
                           _settings_dialog_promote_cb,
                           navi_disp);
  elm_toolbar_item_append(tb,
                          "preferences-desktop-font",
                          _("Font"),
                          _settings_dialog_promote_cb,
                          navi_font);

  elm_box_pack_end(boxv, boxh);

  /* Close Button */
  obj = elm_button_add(win);
  elm_object_text_set(obj,_("Close"));
  ic = elm_image_add (win);
  if (elm_icon_standard_set (ic,"dialog-close")) {
    elm_object_content_set (obj, ic);
    evas_object_show(ic);
  } else
    evas_object_del(ic);
  evas_object_size_hint_align_set(obj, 0.5, 0);
  evas_object_size_hint_min_set(obj, ELM_SCALE_SIZE(BUTTON_WIDTH), ELM_SCALE_SIZE(BUTTON_HEIGHT));
  evas_object_smart_callback_add (obj, "clicked", settings_delete_cb, win);
  elm_box_pack_end(boxv, obj);
  evas_object_show (obj);

  evas_object_resize(win, w * 0.8, h * 0.8);
  elm_win_raise (win);
  evas_object_show (win);

  return win;
}
