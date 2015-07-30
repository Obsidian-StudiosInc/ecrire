#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Elementary.h>

#include "../mess_header.h"

static void *done_data;
static void (*done_cb)(void *data);

static void
_discard(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecrire_Entry *ent = done_data;

   evas_object_del(data);
   done_cb(ent);
}

static void
_fs_save_done(void *data __UNUSED__, Evas_Object *obj __UNUSED__,
      void *event_info)
{
   const char *selected = event_info;

   if (selected)
     {
        save_do(selected, done_data);
        done_cb(data);
     }
}

static void
_save(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecrire_Entry *ent = done_data;

   evas_object_del(data);
   editor_save(ent, _fs_save_done);
}

static void
_cancel(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_del(data);
}

void
ui_alert_need_saving(Evas_Object *entry, void (*done)(void *data), void *data)
{
   Evas_Object *popup, *btn1, *btn2, *btn3;
   popup = elm_popup_add(elm_object_top_widget_get(entry));

   done_cb = done;
   done_data = data;

   elm_object_part_text_set(popup, "title,text", "Unsaved Changes");
   elm_object_text_set(popup,
         _("<align=center>Would you like to save changes to document?<br>"
         "Any unsaved changes will be lost."));

   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, _("Save"));
   elm_object_part_content_set(popup, "button1",  btn1);
   evas_object_smart_callback_add(btn1, "clicked", _save, popup);

   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, _("Discard"));
   elm_object_part_content_set(popup, "button2", btn2);
   evas_object_smart_callback_add(btn2, "clicked", _discard, popup);

   btn3 = elm_button_add(popup);
   elm_object_text_set(btn3, _("Cancel"));
   elm_object_part_content_set(popup, "button3", btn3);
   evas_object_smart_callback_add(btn3, "clicked", _cancel, popup);

   elm_popup_orient_set(popup, ELM_POPUP_ORIENT_CENTER);
   evas_object_show(popup);
}
