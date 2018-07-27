#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <regex.h>

#include <Elementary.h>

#include "../ecrire.h"

#define PATTERN_LEN 9
#define PATTERN_REGEX "^%s"
#define PATTERN_WHOLE_WORD "^(%s)(\\W)"

const static int PADDING = 2;
const static int BUTTON_ICON_SIZE = 12;

static Evas_Object *find_entry;
static Evas_Object *replace_entry;
static Evas_Object *search_box;
static Evas_Object *case_button;
static Evas_Object *regex_button;
static Evas_Object *whole_button;
static regmatch_t *_regmatch;

Eina_Bool
match(const char *string, char *pattern, Eina_Bool match_case)
{
    int flags;
    int status;
    regex_t re;

    flags = REG_EXTENDED;
    if(match_case)
        flags = flags | REG_ICASE;
    if (regcomp(&re, pattern, flags) != 0)
      return(EINA_FALSE);

    if(elm_object_disabled_get(regex_button))
      {
        _regmatch = malloc(3 * sizeof(regmatch_t));
        status = regexec(&re, string, (size_t) 3, _regmatch, 0);
      }
    else
      status = regexec(&re, string, (size_t) 0, NULL, 0);
    regfree(&re);
    if (status != 0)
      return(EINA_FALSE);
    return(EINA_TRUE);
}

EAPI int
elm_code_text_strnsearchpos(const char *content,
                            unsigned int length,
                            const char *search,
                            int offset,
                            Eina_Bool match_case,
                            Eina_Bool whole_word,
                            Eina_Bool regex)
{
  unsigned int c;
  unsigned int searchlen;
  char *ptr;

  searchlen = strlen(search);
  ptr = (char *) content;

  if (searchlen > length)
    return ELM_CODE_TEXT_NOT_FOUND;

  ptr += offset;
  for (c = offset; c <= length - searchlen; c++)
    {
      if(regex || whole_word)
        {
          char *whole_search;
          int regex_len;
          Eina_Bool matched;

          regex_len = searchlen + PATTERN_LEN;
          whole_search = malloc(regex_len + PATTERN_LEN * sizeof(char));
          if(whole_word)
            snprintf(whole_search, regex_len, PATTERN_WHOLE_WORD, search);
          else
            snprintf(whole_search, regex_len, PATTERN_REGEX, search);
          matched = match(ptr, whole_search, match_case);
          free(whole_search);
          if (matched)
            return c;
        }
      else if(match_case)
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
_elm_obj_toggle_cb(void *data EINA_UNUSED,
                   Evas_Object *obj,
                   void *event_info EINA_UNUSED)
{
  elm_object_disabled_set(obj,!elm_object_disabled_get(obj));
}

static void
_search_select_text(Elm_Code_Widget *entry,
                    Elm_Code_Line *code_line,
                    const int found,
                    const int row,
                    int col,
                    const int len)
{
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
  int i;
  int found = -1;
  unsigned int col;
  unsigned int row;
  unsigned int lines;
  unsigned int len;

  if (!text || !*text)
    return EINA_FALSE;

  lines = elm_code_file_lines_get(doc->code->file);
  elm_obj_code_widget_cursor_position_get(doc->widget,&row,&col);
  i=row;
  while(i>=0 && i<=lines)
    {
      code_line = elm_code_file_line_get(doc->code->file,i);
      line = elm_code_line_text_get(code_line, &len);
      found = elm_code_text_strnsearchpos(line,len,text,col,
                                        elm_object_disabled_get(case_button),
                                        elm_object_disabled_get(whole_button),
                                        elm_object_disabled_get(regex_button));
      if(found>=0)
        {
          if(_regmatch)
            {
              len = _regmatch[1].rm_eo;
              free(_regmatch);
            }
          else
            len = strlen(text);
          _search_select_text(doc->widget, code_line, found, i, col, len);
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
  const char *find;
  int pos;

  find = elm_entry_entry_get(find_entry);
  if(strlen(find)<=0)
    return(replaced);
  pos = _find_in_entry(doc, find, EINA_TRUE);
  if(pos>=0)
    {
      const char *replace;

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
  Evas_Object *icon;
  Evas_Object *obj;
  Evas_Object *table;
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
  evas_object_smart_callback_add(case_button, "clicked", _elm_obj_toggle_cb, doc);
  evas_object_show(case_button);

  whole_button = elm_button_add(table);
  icon = elm_icon_add (table);
  evas_object_size_hint_aspect_set (icon, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
  if (elm_icon_standard_set (icon, "zoom-out"))
    {
      evas_object_size_hint_min_set(icon,
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE),
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE));
      elm_object_part_content_set(whole_button, "icon", icon);
      elm_object_tooltip_text_set(whole_button, _("Whole word"));
      evas_object_show (icon);
    }
  else
    {
      evas_object_del(icon);
      elm_object_text_set(whole_button, _("Whole word"));
    }
  elm_table_pack (table, whole_button, 6, row, 1, 1);
  evas_object_smart_callback_add(whole_button, "clicked", _elm_obj_toggle_cb, doc);
  evas_object_show(whole_button);

  regex_button = elm_button_add(table);
  icon = elm_icon_add (table);
  evas_object_size_hint_aspect_set (icon, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
  if (elm_icon_standard_set (icon, "system-run"))
    {
      evas_object_size_hint_min_set(icon,
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE),
                                    ELM_SCALE_SIZE(BUTTON_ICON_SIZE));
      elm_object_part_content_set(regex_button, "icon", icon);
      elm_object_tooltip_text_set(regex_button, _("Regular Expression"));
      evas_object_show (icon);
    }
  else
    {
      evas_object_del(icon);
      elm_object_text_set(regex_button, _("RegEx"));
    }
  elm_table_pack (table, regex_button, 7, row, 1, 1);
  evas_object_smart_callback_add(regex_button, "clicked", _elm_obj_toggle_cb, doc);
  evas_object_show(regex_button);
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
  elm_table_pack (table, obj, 7, row, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _search_box_del, doc);
  evas_object_show(obj);

  ecrire_pack_end(table);
  evas_object_show (search_box);

  elm_object_focus_set(find_entry, EINA_TRUE);

  return search_box;
}
