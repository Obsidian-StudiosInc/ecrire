#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Elementary.h>

#include "../ecrire.h"

const static int PADDING = 5;
const static int BUTTON_HEIGHT = 27;
const static int BUTTON_WIDTH = 60;

static Evas_Object *goto_win, *goto_entry;

static void
_goto_win_del(void *data EINA_UNUSED,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
  evas_object_del(goto_win);
  goto_win = NULL;
}

static void
_goto_clicked(void *data,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
  Ecrire_Doc *doc;
  int line, lines;

  doc = data;
  line = atoi(elm_entry_entry_get(goto_entry));
  lines = elm_code_file_lines_get(doc->code->file);
  if (line>0 && lines > 0 && line <= lines)
    {
      elm_obj_code_widget_cursor_position_set(doc->widget,line,1);
      elm_object_focus_set(doc->widget, EINA_TRUE);
      _goto_win_del(NULL,NULL,NULL);
    }
}

Evas_Object *
ui_goto_dialog_open(Evas_Object *parent, Ecrire_Doc *doc)
{
  Evas_Object *obj, *table;

  if (goto_win)
    {
      evas_object_show(goto_win);
      return goto_win;
    }

  goto_win = elm_win_util_dialog_add(parent, "jump-to",  _("Jump to"));
  elm_win_autodel_set(goto_win, EINA_TRUE);
  evas_object_smart_callback_add(goto_win, "delete,request", _goto_win_del, NULL);

  table = elm_table_add(goto_win);
  elm_table_padding_set(table,ELM_SCALE_SIZE(PADDING),ELM_SCALE_SIZE(PADDING));
  evas_object_size_hint_padding_set (table,
                                     ELM_SCALE_SIZE(PADDING),
                                     ELM_SCALE_SIZE(PADDING),
                                     ELM_SCALE_SIZE(PADDING),
                                     ELM_SCALE_SIZE(PADDING));
  evas_object_show(table);

  obj = elm_label_add (table);
  elm_object_text_set (obj,  _("Jump to line"));
  evas_object_size_hint_weight_set (obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set (obj, 1, 0);
  elm_table_pack (table, obj, 0, 0, 1, 1);
  evas_object_show (obj);

  goto_entry = elm_entry_add(table);
  elm_entry_scrollable_set(goto_entry, EINA_TRUE);
  elm_entry_single_line_set(goto_entry, EINA_TRUE);
  evas_object_size_hint_weight_set(goto_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(goto_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_table_pack (table, goto_entry, 1, 0, 2, 1);
  evas_object_show(goto_entry);

  obj = elm_button_add(table);
  elm_object_text_set(obj, _("Go"));
  evas_object_size_hint_align_set(obj, 0, 0);
  evas_object_size_hint_min_set(obj,
                                ELM_SCALE_SIZE(BUTTON_WIDTH),
                                ELM_SCALE_SIZE(BUTTON_HEIGHT));
  elm_table_pack (table, obj, 2, 1, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _goto_clicked, doc);
  evas_object_show(obj);

  /* Box for padding */
  obj = elm_box_add (goto_win);
  elm_box_pack_end (obj, table);
  evas_object_show (obj);

  elm_win_resize_object_add (goto_win, obj);
  elm_win_raise (goto_win);
  evas_object_show (goto_win);

  elm_object_focus_set(goto_entry, EINA_TRUE);

  return goto_win;
}
