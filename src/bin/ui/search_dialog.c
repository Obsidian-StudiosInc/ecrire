#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Elementary.h>

#include "../mess_header.h"

const static int PADDING = 5;
const static int BUTTON_HEIGHT = 27;
const static int BUTTON_WIDTH = 60;

static Evas_Object *win, *search, *replace;
static Eina_Bool forward = EINA_TRUE;
static Evas_Textblock_Cursor *cur_find;

static Eina_Bool
_find_in_entry(Evas_Object *entry, const char *text, Eina_Bool jump_next)
{
   Eina_Bool try_next = EINA_FALSE;
   const char *found;
   char *utf8;
   const Evas_Object *tb = elm_entry_textblock_get(entry);
   Evas_Textblock_Cursor *end, *start, *mcur;
   size_t initial_pos;

   if (!text || !*text)
      return EINA_FALSE;

   mcur = (Evas_Textblock_Cursor *) evas_object_textblock_cursor_get(tb);
   if (!cur_find)
     {
        cur_find = evas_object_textblock_cursor_new(tb);
     }
   else if (!evas_textblock_cursor_compare(cur_find, mcur))
     {
        try_next = EINA_TRUE;
     }

   if (forward)
     {
        evas_textblock_cursor_paragraph_last(cur_find);
        start = mcur;
        end = cur_find;
     }
   else
     {
        /* Not correct, more adjustments needed. */
        evas_textblock_cursor_paragraph_first(cur_find);
        start = cur_find;
        end = mcur;
     }

   initial_pos = evas_textblock_cursor_pos_get(start);

   utf8 = evas_textblock_cursor_range_text_get(start, end,
         EVAS_TEXTBLOCK_TEXT_PLAIN);

   if (!utf8)
      return EINA_FALSE;

   if (try_next && jump_next)
     {
        found = strstr(utf8 + 1, text);
        if (!found)
          {
             found = utf8;
          }
     }
   else
     {
        found = strstr(utf8, text);
     }

   if (found)
     {
        size_t pos = 0;
        int idx = 0;
        while ((utf8 + idx) < found)
          {
             pos++;
#if (EINA_VERSION_MAJOR > 1) || (EINA_VERSION_MINOR >= 8)
             eina_unicode_utf8_next_get(utf8, &idx);
#else
             eina_unicode_utf8_get_next(utf8, &idx);
#endif
          }

        elm_entry_select_none(entry);
        evas_textblock_cursor_pos_set(mcur, pos + initial_pos + strlen(text));
        elm_entry_cursor_selection_begin(entry);
        elm_entry_cursor_pos_set(entry, pos + initial_pos);
        elm_entry_cursor_selection_end(entry);
        evas_textblock_cursor_copy(mcur, cur_find);
     }

   free(utf8);

   return !!found;
}

static void
_find_clicked(void *data,
      Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _find_in_entry(data, elm_object_text_get(search), EINA_TRUE);
}

static void
_replace_clicked(void *data,
      Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   if (_find_in_entry(data, elm_object_text_get(search), EINA_FALSE))
     {
        elm_entry_entry_insert(data, elm_object_text_get(replace));
        if (cur_find)
          {
             evas_textblock_cursor_free(cur_find);
             cur_find = NULL;
          }
     }
}

static void
my_win_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   (void) obj;
   (void) event_info;
   /* Reset the stuff that need reseting */
   if (cur_find)
     {
        evas_textblock_cursor_free(cur_find);
        cur_find = NULL;
     }
   win = NULL;
}

Evas_Object *
ui_find_dialog_open(Evas_Object *pareplace, Ecrire_Entry *ent)
{
  Evas_Object *obj, *table;
  int row = 0;

  if (win)
    {
      evas_object_show(win);
      return win;
    }

  win = elm_win_util_dialog_add(pareplace, _("ecrire"),  _("Search"));
  elm_win_autodel_set(win, EINA_TRUE);
  evas_object_smart_callback_add(win, "delete,request", my_win_del, ent->entry);

  table = elm_table_add(win);
  elm_obj_table_padding_set(table,
                            ELM_SCALE_SIZE(PADDING),
                            ELM_SCALE_SIZE(PADDING));
  evas_object_size_hint_padding_set (table,
                                     ELM_SCALE_SIZE(PADDING),
                                     ELM_SCALE_SIZE(PADDING),
                                     ELM_SCALE_SIZE(PADDING),
                                     ELM_SCALE_SIZE(PADDING));
  evas_object_show(table);

  /* Search for Label */
  obj = elm_label_add (table);
  elm_object_text_set (obj,  _("Search for"));
  evas_object_size_hint_weight_set (obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set (obj, 1, 0);
  elm_table_pack (table, obj, 0, row, 1, 1);
  evas_object_show (obj);

  search = elm_entry_add(table);
  elm_entry_scrollable_set(search, EINA_TRUE);
  elm_entry_single_line_set(search, EINA_TRUE);
  evas_object_size_hint_weight_set(search, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(search, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_table_pack (table, search, 1, row, 3, 1);
  evas_object_show(search);
  row++;

  /* Replace with Label */
  obj = elm_label_add (table);
  elm_object_text_set (obj,  _("Replace with"));
  evas_object_size_hint_weight_set (obj, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set (obj, 1, 0);
  elm_table_pack (table, obj, 0, row, 1, 1);
  evas_object_show (obj);

  replace = elm_entry_add(table);
  elm_entry_scrollable_set(replace, EINA_TRUE);
  elm_entry_single_line_set(replace, EINA_TRUE);
  evas_object_size_hint_weight_set(replace, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(replace, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_table_pack (table, replace, 1, row, 3, 1);
  evas_object_show(replace);
  row++;

  obj = elm_button_add(table);
  elm_object_text_set(obj, _("Find"));
  evas_object_size_hint_align_set(obj, 0, 0);
  evas_object_size_hint_min_set(obj,
                                ELM_SCALE_SIZE(BUTTON_WIDTH),
                                ELM_SCALE_SIZE(BUTTON_HEIGHT));
  elm_table_pack (table, obj, 2, row, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _find_clicked, ent->entry);
  evas_object_show(obj);

  obj = elm_button_add(table);
  elm_object_text_set(obj, _("Replace"));
  evas_object_size_hint_align_set(obj, 0, 0);
  evas_object_size_hint_min_set(obj,
                                ELM_SCALE_SIZE(BUTTON_WIDTH),
                                ELM_SCALE_SIZE(BUTTON_HEIGHT));
  elm_table_pack (table, obj, 3, row, 1, 1);
  evas_object_smart_callback_add(obj, "clicked", _replace_clicked, ent->entry);
  evas_object_show(obj);

  /* Box for padding */
  obj = elm_box_add (win);
  elm_box_pack_end (obj, table);
  evas_object_show (obj);

  elm_win_resize_object_add (win, obj);
  elm_win_raise (win);
  evas_object_show (win);

  elm_object_focus_set(search, EINA_TRUE);

  cur_find = NULL;
  return win;
}
