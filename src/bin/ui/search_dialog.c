#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Elementary.h>

#include "../mess_header.h"

const static int PADDING = 4;
const static int BUTTON_HEIGHT = 29;
const static int BUTTON_WIDTH = 70;

static Evas_Object *find_entry, *replace_entry, *search_box;

static void
_search_box_del(void *data EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
  evas_object_del(search_box);
  search_box = NULL;
}

static int
_find_in_entry(Ecrire_Entry *ent, const char *text, Eina_Bool again)
{
  Elm_Code_Line *code_line;
  int i, found, len, col, row, lines;

  if (!text || !*text)
    return EINA_FALSE;

  lines = elm_code_file_lines_get(ent->code->file);
  elm_obj_code_widget_cursor_position_get(ent->entry,&row,&col);
  for(i=row; i<=lines; i++)
    {
      code_line = elm_code_file_line_get(ent->code->file,i);
      found = elm_code_line_text_strpos(code_line,text,col);
      if(found>0)
        {
          len = strlen(text);
          col = found+len+1;
          elm_code_widget_selection_start(ent->entry,i,found+1);
          elm_code_widget_selection_end(ent->entry,i,col-1);
          elm_obj_code_widget_cursor_position_set(ent->entry,i,col);
          break;
        }
      else if(i==row || i==lines) // reset and repeat
        {
          if(col>1)
              col = 1;
          if(i==lines)
              i = 0;
        }
    }
  return found;
}

static void
_find_clicked(void *data,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
  _find_in_entry((Ecrire_Entry *)data,
                 elm_entry_entry_get(find_entry),
                 EINA_TRUE);
}

static void
_replace_clicked(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
  const char *replace;
  int  len, col, row;
  unsigned int pos;

  if (pos = _find_in_entry(data, elm_entry_entry_get(find_entry), EINA_FALSE))
    {
      Ecrire_Entry *ent = data;
      Elm_Code_Line *code_line;

      replace = elm_entry_entry_get(replace_entry);
      elm_obj_code_widget_cursor_position_get(ent->entry,&row,&col);
      code_line = elm_code_file_line_get(ent->code->file,row);
      len = col-pos-1;
      elm_code_line_text_remove(code_line, pos, len);
      len = strlen(replace);
      elm_code_line_text_insert(code_line, pos, replace, len);
    }
}

Evas_Object *
ui_find_dialog_open(Evas_Object *parent, Ecrire_Entry *ent)
{
  Evas_Object *obj, *table;
  int row = 0;

  if (search_box)
    {
      evas_object_show(search_box);
      return search_box;
    }

  search_box = table = elm_table_add(parent);
  elm_obj_table_padding_set(table, ELM_SCALE_SIZE(PADDING), 0);
  evas_object_size_hint_padding_set(table, ELM_SCALE_SIZE(PADDING), 0, 0, 0);
  evas_object_size_hint_weight_set(table, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(table, EVAS_HINT_FILL, EVAS_HINT_FILL);
  evas_object_show(table);

  /* Search for Label */
  obj = elm_label_add (table);
  elm_object_text_set (obj,  _("Search for"));
  evas_object_size_hint_align_set (obj, 1, EVAS_HINT_FILL);
  elm_table_pack (table, obj, 0, row, 1, 1);
  evas_object_show (obj);

  find_entry = elm_entry_add(table);
  if(!elm_code_widget_selection_is_empty(ent->entry))
    elm_object_text_set(find_entry,
                        elm_code_widget_selection_text_get(ent->entry));
  elm_entry_scrollable_set(find_entry, EINA_TRUE);
  elm_entry_single_line_set(find_entry, EINA_TRUE);
  evas_object_size_hint_weight_set(find_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(find_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_table_pack (table, find_entry, 1, row, 1, 1);
  evas_object_show(find_entry);

  obj = elm_button_add(table);
  elm_object_text_set(obj, _("Next"));
  evas_object_size_hint_min_set(obj,
                                ELM_SCALE_SIZE(BUTTON_WIDTH),
                                ELM_SCALE_SIZE(BUTTON_HEIGHT));
  elm_table_pack (table, obj, 2, row, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _find_clicked, ent);
  evas_object_show(obj);
  
  obj = elm_button_add(table);
  elm_object_text_set(obj, _("Previous"));
  evas_object_size_hint_min_set(obj,
                                ELM_SCALE_SIZE(BUTTON_WIDTH),
                                ELM_SCALE_SIZE(BUTTON_HEIGHT));
  elm_table_pack (table, obj, 3, row, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _find_clicked, ent);
  evas_object_show(obj);
  row++;

  /* Replace with Label */
  obj = elm_label_add (table);
  elm_object_text_set (obj,  _("Replace with"));
  evas_object_size_hint_align_set (obj, 1, EVAS_HINT_FILL);
  elm_table_pack (table, obj, 0, row, 1, 1);
  evas_object_show (obj);

  replace_entry = elm_entry_add(table);
  elm_entry_scrollable_set(replace_entry, EINA_TRUE);
  elm_entry_single_line_set(replace_entry, EINA_TRUE);
  evas_object_size_hint_weight_set(replace_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(replace_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_table_pack (table, replace_entry, 1, row, 1, 1);
  evas_object_show(replace_entry);

  obj = elm_button_add(table);
  elm_object_text_set(obj, _("Replace"));
  evas_object_size_hint_min_set(obj,
                                ELM_SCALE_SIZE(BUTTON_WIDTH),
                                ELM_SCALE_SIZE(BUTTON_HEIGHT));
  elm_table_pack (table, obj, 2, row, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _replace_clicked, ent);
  evas_object_show(obj);

  obj = elm_button_add(table);
  elm_object_text_set(obj, _("Close"));
  evas_object_size_hint_min_set(obj,
                                ELM_SCALE_SIZE(BUTTON_WIDTH),
                                ELM_SCALE_SIZE(BUTTON_HEIGHT));
  elm_table_pack (table, obj, 3, row, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _search_box_del, ent);
  evas_object_show(obj);

  elm_box_pack_end(ent->box_editor, table);
  evas_object_show (search_box);

  elm_object_focus_set(find_entry, EINA_TRUE);

  return search_box;
}
