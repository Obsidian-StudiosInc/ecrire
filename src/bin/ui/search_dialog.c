#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Elementary.h>

#include "../ecrire.h"

const static int PADDING = 2;
const static int BUTTON_HEIGHT = 29;
const static int BUTTON_WIDTH = 70;
const static int BUTTON_ICON_SIZE = 12;

static Evas_Object *find_entry, *replace_entry, *search_box;
static Evas_Object *case_button;

EAPI int
elm_code_text_strncasepos(const char *content,
                          unsigned int length,
                          const char *search,
                          int offset,
                          Eina_Bool match_case)
{
  unsigned int searchlen, c;
  char *ptr;

  searchlen = strlen(search);
  ptr = (char *) content;

  if (searchlen > length)
    return ELM_CODE_TEXT_NOT_FOUND;

  ptr += offset;
  for (c = offset; c <= length - searchlen; c++)
    {
      if(match_case)
        {
          if (!strncmp(ptr, search, searchlen))
            return c;
        }
      else if (!strncasecmp(ptr, search, searchlen))
        return c;
      ptr++;
    }

   return ELM_CODE_TEXT_NOT_FOUND;
}

static void
_search_box_del(void *data EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
  evas_object_del(search_box);
  search_box = NULL;
}

static void
_search_case_cb(void *data EINA_UNUSED,
                Evas_Object *obj,
                void *event_info EINA_UNUSED)
{
  elm_object_disabled_set(obj,!elm_object_disabled_get(obj));
}

static void
_search_select_text(Elm_Code_Widget *entry,
                    Elm_Code_Line *code_line,
                    const char *text,
                    const int found,
                    const int row,
                    int col)
{
  int len = strlen(text);
  col = elm_code_widget_line_text_column_width_to_position(entry,
                                                           code_line,
                                                           found);
  elm_code_widget_selection_start(entry,row,col);
  elm_obj_code_widget_cursor_position_set(entry,row,col);
  col += len-1;
  elm_code_widget_selection_end(entry,row,col);
}

static int
_find_in_entry(Ecrire_Doc *doc, const char *text, Eina_Bool forward)
{
  Eina_Bool reset = EINA_TRUE;
  Elm_Code_Line *code_line;
  const char *line;
  int i, found, col, row, lines, len;

  if (!text || !*text)
    return EINA_FALSE;

  lines = elm_code_file_lines_get(doc->code->file);
  elm_obj_code_widget_cursor_position_get(doc->widget,&row,&col);
  i=row;
  while(i>=0 && i<=lines)
    {
      code_line = elm_code_file_line_get(doc->code->file,i);
      line = elm_code_line_text_get(code_line, &len);
      found = elm_code_text_strncasepos(line,len,text,col,
                                        elm_object_disabled_get(case_button));
      if(found>=0)
        {
          _search_select_text(doc->widget, code_line, text, found, i, col);
          break;
        }
      else if(forward)
        {
          if(i==row || i==lines) // reset and repeat
            {
              if(col>0)
                col = 0;
              if(i==lines && reset)
                {
                  i = 0;
                  reset = EINA_FALSE;
                }
            }
          i++;
        }
      else
        {
         if(i==row || i==0) // reset and repeat
            {
              if(col>0)
                col = 0;
              if(i==0 && reset)
                {
                  i = lines;
                  reset = EINA_FALSE;
                }
            }
          i--;
        }
    }
  return found;
}

static Eina_Bool
_replace_in_entry(Ecrire_Doc *doc)
{
  Eina_Bool replaced = EINA_FALSE;
  const char *find, *replace;
  int pos;

  find = elm_entry_entry_get(find_entry);
  if(strlen(find)<=0)
    return(replaced);
  pos = _find_in_entry(doc, find, EINA_TRUE);
  if(pos>=0)
    {
      replace = elm_entry_entry_get(replace_entry);
      elm_code_widget_selection_delete(doc->widget);
      elm_code_widget_text_at_cursor_insert(doc->widget, replace);
      efl_event_callback_legacy_call(doc->widget,
                                     ELM_OBJ_CODE_WIDGET_EVENT_CHANGED_USER,
                                     NULL);
      replaced = EINA_TRUE;
    }
  return(replaced);
}

static void
_search_next_cb(void *data,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
  _find_in_entry((Ecrire_Doc *)data,
                 elm_entry_entry_get(find_entry),
                 EINA_TRUE);
}

static void
_search_prev_cb(void *data,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
  _find_in_entry((Ecrire_Doc *)data,
                 elm_entry_entry_get(find_entry),
                 EINA_FALSE);
}

static void
_replace_all_clicked(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
  while(_replace_in_entry((Ecrire_Doc *)data)==EINA_TRUE) ;
}

static void
_replace_clicked(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
  _replace_in_entry((Ecrire_Doc *)data);
}

Evas_Object *
ui_find_dialog_open(Evas_Object *parent, Ecrire_Doc *doc)
{
  Evas_Object *icon, *obj, *table;
  int row = 0;

  if (search_box)
    {
      evas_object_show(search_box);
      return search_box;
    }

  search_box = table = elm_table_add(parent);
  elm_table_padding_set(table, ELM_SCALE_SIZE(PADDING), 0);
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
  if(!elm_code_widget_selection_is_empty(doc->widget))
    elm_object_text_set(find_entry,
                        elm_code_widget_selection_text_get(doc->widget));
  elm_entry_scrollable_set(find_entry, EINA_TRUE);
  elm_entry_single_line_set(find_entry, EINA_TRUE);
  evas_object_size_hint_weight_set(find_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(find_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_table_pack (table, find_entry, 1, row, 1, 1);
  evas_object_smart_callback_add(find_entry, "activated", _search_next_cb, doc);
  evas_object_show(find_entry);

  obj = elm_button_add(table);
  icon = elm_icon_add (table);
  evas_object_size_hint_aspect_set (icon, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
  if (elm_icon_standard_set (icon, "edit-find"))
    {
      evas_object_size_hint_min_set(icon,
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE),
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE));
      elm_object_part_content_set(obj, "icon", icon);
      elm_object_tooltip_text_set(obj, _("Find All"));
      evas_object_show (icon);
    }
  else
    {
      evas_object_del(icon);
      elm_object_text_set(obj, _("Find All"));
    }
  elm_table_pack (table, obj, 2, row, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _search_next_cb, doc);
  evas_object_show(obj);

  obj = elm_button_add(table);
  icon = elm_icon_add (table);
  evas_object_size_hint_aspect_set (icon, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
  if (elm_icon_standard_set (icon, "go-previous"))
    {
      evas_object_size_hint_min_set(icon,
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE),
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE));
      elm_object_part_content_set(obj, "icon", icon);
      elm_object_tooltip_text_set(obj, _("Find Previous"));
      evas_object_show (icon);
    }
  else
    {
      evas_object_del(icon);
      elm_object_text_set(obj, _("Previous"));
    }
  elm_table_pack (table, obj, 3, row, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _search_prev_cb, doc);
  evas_object_show(obj);

  obj = elm_button_add(table);
  icon = elm_icon_add (table);
  evas_object_size_hint_aspect_set (icon, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
  if (elm_icon_standard_set (icon, "go-next"))
    {
      evas_object_size_hint_min_set(icon,
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE),
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE));
      elm_object_part_content_set(obj, "icon", icon);
      elm_object_tooltip_text_set(obj, _("Find Next"));
      evas_object_show (icon);
    }
  else
    {
      evas_object_del(icon);
      elm_object_text_set(obj, _("Next"));
    }
  elm_table_pack (table, obj, 4, row, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _search_next_cb, doc);
  evas_object_show(obj);

  case_button = elm_button_add(table);
  icon = elm_icon_add (table);
  evas_object_size_hint_aspect_set (icon, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
  if (elm_icon_standard_set (icon, "format-text-bold"))
    {
      evas_object_size_hint_min_set(icon,
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE),
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE));
      elm_object_part_content_set(case_button, "icon", icon);
      elm_object_tooltip_text_set(case_button, _("Match case"));
      evas_object_show (icon);
    }
  else
    {
      evas_object_del(icon);
      elm_object_text_set(case_button, _("Match Case"));
    }
  elm_table_pack (table, case_button, 5, row, 1, 1);
  evas_object_smart_callback_add(case_button, "clicked", _search_case_cb, doc);
  evas_object_show(case_button);
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
  icon = elm_icon_add (table);
  evas_object_size_hint_aspect_set (icon, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
  if (elm_icon_standard_set (icon, "document-edit"))
    {
      evas_object_size_hint_min_set(icon,
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE),
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE));
      elm_object_part_content_set(obj, "icon", icon);
      elm_object_tooltip_text_set(obj, _("Replace with"));
      evas_object_show (icon);
    }
  else
    {
      evas_object_del(icon);
      elm_object_text_set(obj, _("Replace"));
    }
  elm_table_pack (table, obj, 2, row, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _replace_clicked, doc);
  evas_object_show(obj);

  obj = elm_button_add(table);
  icon = elm_icon_add (table);
  evas_object_size_hint_aspect_set (icon, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
  if (elm_icon_standard_set (icon, "edit-find-replace"))
    {
      evas_object_size_hint_min_set(icon,
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE),
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE));
      elm_object_part_content_set(obj, "icon", icon);
      elm_object_tooltip_text_set(obj, _("Replace All"));
      evas_object_show (icon);
    }
  else
    {
      evas_object_del(icon);
      elm_object_text_set(obj, _("Replace All"));
    }
  elm_table_pack (table, obj, 3, row, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _replace_all_clicked, doc);
  evas_object_show(obj);

  obj = elm_button_add(table);
  icon = elm_icon_add (table);
  evas_object_size_hint_aspect_set (icon, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
  if (elm_icon_standard_set (icon, "window-close"))
    {
      evas_object_size_hint_min_set(icon,
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE),
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE));
      elm_object_part_content_set(obj, "icon", icon);
      elm_object_tooltip_text_set(obj, _("Close"));
      evas_object_show (icon);
    }
  else
    {
      evas_object_del(icon);
      elm_object_text_set(obj, _("Close"));
    }
  elm_table_pack (table, obj, 4, row, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _search_box_del, doc);
  evas_object_show(obj);

  elm_box_pack_end(doc->box_editor, table);
  evas_object_show (search_box);

  elm_object_focus_set(find_entry, EINA_TRUE);

  return search_box;
}
