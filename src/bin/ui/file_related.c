#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Elementary.h>
#include "../cfg.h"
#include "../ecrire.h"

static void _cleaning_cb(void *data, Evas_Object *obj, void *event_info);
static void _set_file_cb(void *data,
                         Evas_Object *obj EINA_UNUSED,
                         void *event_info);

static Ecrire_Doc *_file_doc;
static Evas_Object *_inwin;

static void
_cleaning_cb(void *data, Evas_Object *obj, void *event_info)
{
   void (*func) (void *, Evas_Object *, void *) = data;
   func(_file_doc, obj, event_info);
   evas_object_del(_inwin);  /* delete the test window */
   _file_doc = NULL;
}

static void
_set_file_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
  const char *file = elm_object_item_text_get(event_info);
  evas_object_smart_callback_call((Evas_Object *)data,"done",(void *)file);
}

void
ui_file_open_save_dialog_open(Ecrire_Doc *doc,
                              Evas_Smart_Cb func,
                              Eina_Bool save)
{
   const char *file;
   Eina_List *itr = NULL;
   Evas_Object *fs;
   Evas_Object *icon;
   Evas_Object *sel;
   Evas_Object *box;

   _file_doc = doc;
   _inwin = elm_win_inwin_add(ecrire_win_get());

   box = elm_box_add(_inwin);
   elm_win_inwin_content_set(_inwin, box);
   evas_object_show(box);

   sel = elm_hoversel_add(_inwin);
   elm_hoversel_auto_update_set(sel,EINA_TRUE);
   icon = elm_icon_add (_inwin);
   if (elm_icon_standard_set (icon, "document-open-recent") ||
       elm_icon_standard_set (icon, "document-multiple"))
     {
       elm_object_part_content_set(sel, "icon", icon);
       evas_object_show (icon);
     }
   else
     evas_object_del(icon);
   elm_object_text_set(sel, _("Recent files"));
   EINA_LIST_FOREACH(_ent_cfg->recent, itr, file)
     {
        elm_hoversel_item_add(sel, (char *)file, NULL, ELM_ICON_NONE, NULL, NULL);
     }
               
   evas_object_size_hint_weight_set(sel, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(sel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_start(box,sel);
   evas_object_show(sel);

   fs = elm_fileselector_add(_inwin);
   elm_fileselector_is_save_set(fs, save);
   elm_fileselector_expandable_set(fs, EINA_FALSE);
   if(doc->path)
     elm_fileselector_path_set(fs, doc->path);
   else
     elm_fileselector_path_set(fs, getenv("HOME"));
   evas_object_size_hint_weight_set(fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box,fs);
   evas_object_show(fs);

   evas_object_smart_callback_add(sel, "selected", _set_file_cb, fs);
   evas_object_smart_callback_add(fs, "done", _cleaning_cb, func);

   evas_object_resize(_inwin, _ent_cfg->width , _ent_cfg->height);
   evas_object_show(_inwin);
}
