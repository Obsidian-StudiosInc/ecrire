#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Elementary.h>

#include "../ecrire.h"

static void _cancel(void *data,
                    Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED);
static void _discard(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED);
static void _fs_save_done(void *data EINA_UNUSED,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info);
static void _save(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED);

static void *done_data;
static void (*done_cb)(void *data);

static void
_discard(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = done_data;

   evas_object_del(data);
   done_cb(doc);
}

static void
_fs_save_done(void *data EINA_UNUSED,
              Evas_Object *obj EINA_UNUSED,
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
_save(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = done_data;

   evas_object_del(data);
   editor_save(doc, _fs_save_done);
}

static void
_cancel(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

void
ui_alert_need_saving(Evas_Object *parent, void (*done)(void *data), void *data)
{
   Evas_Object *obj;
   Evas_Object *popup;
   popup = elm_popup_add(elm_object_top_widget_get(parent));

   done_cb = done;
   done_data = data;

   elm_object_part_text_set(popup, "title,text", _("Unsaved Changes"));
   elm_object_text_set(popup,
         _("<align=center>Would you like to save changes to document?<br>"
         "Any unsaved changes will be lost."));

   obj = elm_button_add(popup);
   elm_object_text_set(obj, _("Save"));
   elm_object_part_content_set(popup, "button1",  obj);
   evas_object_smart_callback_add(obj, "clicked", _save, popup);

   obj = elm_button_add(popup);
   elm_object_text_set(obj, _("Discard"));
   elm_object_part_content_set(popup, "button2", obj);
   evas_object_smart_callback_add(obj, "clicked", _discard, popup);

   obj = elm_button_add(popup);
   elm_object_text_set(obj, _("Cancel"));
   elm_object_part_content_set(popup, "button3", obj);
   evas_object_smart_callback_add(obj, "clicked", _cancel, popup);

   elm_popup_orient_set(popup, ELM_POPUP_ORIENT_CENTER);
   evas_object_show(popup);
}
