#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Elementary.h>

#include "../mess_header.h"

static Evas_Object *goto_win, *sent;

static void
_goto_do(Evas_Object *entry, const char *text)
{
   int line;
   Evas_Object *tb = elm_entry_textblock_get(entry);
   Evas_Textblock_Cursor *mcur = evas_object_textblock_cursor_get(tb);

   line = atoi(text);

   if (line > 0)
     {
        evas_object_hide(goto_win);
	evas_textblock_cursor_line_set(mcur, line-1);
	elm_entry_calc_force(entry);

	elm_object_focus_set(entry, EINA_TRUE);
     }
}

static void
_goto_clicked(void *data,
     Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _goto_do(data, elm_object_text_get(sent));
}

static void
_my_win_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   (void) obj;
   (void) event_info;
   /* Reset the stuff that need reseting */
   goto_win = NULL;
}

Evas_Object *
ui_goto_dialog_open(Evas_Object *parent, Ecrire_Entry *ent)
{
   Evas_Object *entry = ent->entry;
   Evas_Object *win, *bg, *bx, *lbl, *hbx, *btn;

   if (goto_win)
     {
        evas_object_show(goto_win);
        return goto_win;
     }

   goto_win = win = elm_win_add(parent, "jump-to", ELM_WIN_TOOLBAR);
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_title_set(win, _("Jump to"));
   evas_object_smart_callback_add(win, "delete,request", _my_win_del, entry);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   hbx = elm_box_add(win);
   elm_box_padding_set(hbx, 15, 0);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, 0.0);
   evas_object_show(hbx);
   elm_box_pack_end(bx, hbx);

   lbl = elm_label_add(win);
   elm_object_text_set(lbl, _("Jump to line:"));
   evas_object_size_hint_align_set(lbl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(lbl, 0.0, 0.0);
   elm_box_pack_end(hbx, lbl);
   evas_object_show(lbl);

   sent = elm_entry_add(win);
   elm_entry_scrollable_set(sent, EINA_TRUE);
   elm_entry_single_line_set(sent, EINA_TRUE);
   evas_object_size_hint_align_set(sent, EVAS_HINT_FILL, 0.0);
   evas_object_size_hint_weight_set(sent, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(hbx, sent);
   evas_object_show(sent);

   hbx = elm_box_add(win);
   elm_box_homogeneous_set(hbx, EINA_FALSE);
   elm_box_padding_set(hbx, 15, 0);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, 0.0);
   evas_object_show(hbx);
   elm_box_pack_end(bx, hbx);

   btn = elm_button_add(win);
   elm_object_text_set(btn, _("Go"));
   evas_object_size_hint_align_set(btn, 1.0, 0.0);
   evas_object_size_hint_weight_set(btn, 0.0, 0.0);
   evas_object_show(btn);
   elm_box_pack_end(hbx, btn);
   evas_object_smart_callback_add(btn, "clicked", _goto_clicked, entry);

   /* Forcing it to be the min height. */
   evas_object_resize(win, 300, 1);
   evas_object_show(win);

   return win;
}

