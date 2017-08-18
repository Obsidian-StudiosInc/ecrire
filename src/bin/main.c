#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <Elementary.h>

#include "ecrire.h"
#include "cfg.h"
#include "ui/ui.h"

static Eina_Unicode plain_utf8 = EINA_TRUE;
static Ecrire_Doc *main_doc;

static void print_usage(const char *bin);

Eina_Bool ctrl_pressed = EINA_FALSE;
/* specific log domain to help debug only ecrire */
int _ecrire_log_dom = -1;
char *drop_file = NULL;

static void
_set_path(Ecrire_Doc *doc, const char *file)
{
  char *f = NULL;
  char *fp = NULL;
  char *path = NULL;
  int len;
  if(doc->path)
    free(doc->path);
  len = strlen(file)+1;
  fp = f = (char *)malloc(len);
  if(f)
    {
      strncpy(f,file,len); 
      path = dirname(f);
      len = strlen(path)+1;
      doc->path = (char *)malloc(len);
      if(doc->path)
        strncpy(doc->path,path,len);
      free(fp);
    }
}

static void
_set_cut_copy_disabled(Ecrire_Doc *doc, Eina_Bool disabled)
{
  elm_object_item_disabled_set(doc->cut_item, disabled);
  elm_object_item_disabled_set(doc->copy_item, disabled);
}

static void
_set_save_disabled(Ecrire_Doc *doc, Eina_Bool disabled)
{
  elm_object_item_disabled_set(doc->save_item, disabled);
  elm_object_item_disabled_set(doc->save_as_item, disabled);
}

static void
_set_undo_redo_disabled(Ecrire_Doc *doc, Eina_Bool disabled)
{
   elm_object_item_disabled_set(doc->undo_item, disabled);
   elm_object_item_disabled_set(doc->redo_item, disabled);
}

void
editor_font_set(Ecrire_Doc *doc, const char *name, unsigned int size)
{
  if(size==0)
    size = 10;
  if(name)
    elm_obj_code_widget_font_set(doc->widget, name, size);
  else
    elm_obj_code_widget_font_set(doc->widget, NULL, size);
}

static void
_init_font(Ecrire_Doc *doc)
{
  editor_font_set(doc, _ent_cfg->font.name, _ent_cfg->font.size);
}

static void
_alert_if_need_saving(void (*done)(void *data), Ecrire_Doc *doc)
{
   if (!elm_object_item_disabled_get(doc->save_item))
     {
        ui_alert_need_saving(doc->widget, done, doc);
     }
   else
     {
        done(doc);
     }
}

static void
_sel_start(void *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
  _set_cut_copy_disabled((Ecrire_Doc *)data, EINA_FALSE);
}

static void
_sel_clear(void *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
  _set_cut_copy_disabled((Ecrire_Doc *)data, EINA_TRUE);
}

static void
_sel_cut_copy(void *data,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
  Ecrire_Doc *doc = data;
  elm_object_item_disabled_set(doc->paste_item, EINA_FALSE);
}

static void
_update_cur_file(Ecrire_Doc *doc)
{
  const char *filename = NULL, *saving;
  saving = (!elm_object_item_disabled_get(doc->save_item)) ? "*" : "";
    {
      char buf[1024];
      if(doc->code->file->file)
        filename = eina_file_filename_get(doc->code->file->file);
      if (filename)
         snprintf(buf, sizeof(buf), _("%s%s - %s"), saving,
                  filename, PACKAGE_NAME);
      else
         snprintf(buf, sizeof(buf), _("%sUntitled %d - %s"), saving,
                  doc->unsaved, PACKAGE_NAME);

      elm_win_title_set(doc->win, buf);
    }
}

static void
_cur_changed(void *data,
             Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   char buf[50];
   int line;
   int col;

   Ecrire_Doc *doc = data;
   elm_obj_code_widget_cursor_position_get(doc->widget,&line,&col);
   snprintf(buf, sizeof(buf), _(" Line %d, Column %d"), line, col);
   elm_object_text_set(doc->cursor_label, buf);
   if(elm_object_item_disabled_get(doc->undo_item) &&
      elm_obj_code_widget_can_undo_get(doc->widget))
     {
       elm_object_item_disabled_set(doc->undo_item, EINA_FALSE);
     }
}

static void
_check_set_redo(Ecrire_Doc *doc)
{
  elm_object_item_disabled_set(doc->redo_item,
                               !elm_obj_code_widget_can_redo_get(doc->widget));
}

static void
_check_set_undo(Ecrire_Doc *doc)
{
  elm_object_item_disabled_set(doc->undo_item,
                               !elm_obj_code_widget_can_undo_get(doc->widget));

}

static void
_undo(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   /* In undo we care about the current item */
   Ecrire_Doc *doc = data;
   elm_obj_code_widget_undo(doc->widget);
   _check_set_redo(doc);
   _check_set_undo(doc);
}

static void
_redo(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   elm_obj_code_widget_redo(doc->widget);
   _check_set_redo(doc);
}

static void
_changed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecrire_Doc *doc = data;
   _set_save_disabled(doc, EINA_FALSE);
   elm_object_item_disabled_set(doc->close_item, EINA_FALSE);
   _check_set_redo(doc);
   _update_cur_file(doc);
}

static void
_close_doc (void *data)
{
  Ecrire_Doc *doc = data;
  elm_code_file_close(doc->code->file);
}

static void
_new_doc(Ecrire_Doc *doc) {
  elm_code_file_new(doc->code);
  elm_code_file_line_append(doc->code->file, "", 0, NULL);
  elm_object_item_disabled_set(doc->close_item, EINA_TRUE);
  _init_font(doc);
  _set_save_disabled(doc, EINA_TRUE);
  _set_undo_redo_disabled(doc, EINA_TRUE);
  _update_cur_file(doc);
}

static void
_open_file(Ecrire_Doc *doc, const char *file)
{
  Elm_Code_Syntax *syntax = NULL;
  const char *mime;
  int h;

  if (file)
    {
      mime = efreet_mime_type_get(file);
      if(mime)
        {
          if(!strcasecmp(mime, "text/x-chdr") ||
             !strcasecmp(mime, "text/x-csrc") ||
             !strcasecmp(mime, "text/x-python"))
            syntax = elm_code_syntax_for_mime_get(mime);
          else if(!strcasecmp(mime, "text/x-diff") ||
                  !strcasecmp(mime, "text/x-patch"))
           elm_code_parser_standard_add(doc->code,
                                        ELM_CODE_PARSER_STANDARD_DIFF);
          else if(strstr(mime, "text/"))
            syntax = elm_code_syntax_for_mime_get("text/plain");
          if(syntax)
            {
              elm_obj_code_widget_syntax_enabled_set(doc->widget, EINA_TRUE);
              elm_code_syntax_parse_file(syntax,doc->code->file);
            }
          else
            elm_obj_code_widget_syntax_enabled_set(doc->widget, EINA_FALSE);
        }
      elm_code_file_open(doc->code,file);
      _init_font(doc);
      _set_path(doc,file);
      _set_save_disabled(doc, EINA_TRUE);
      _set_cut_copy_disabled(doc, EINA_TRUE);
      _set_undo_redo_disabled(doc, EINA_TRUE);
      elm_object_item_disabled_set(doc->close_item, EINA_FALSE);

      Elm_Transit *transit = elm_transit_add();
      elm_transit_object_add(transit, doc->box_editor);
      evas_object_geometry_get(doc->win, NULL, NULL, NULL, &h);
      elm_transit_effect_translation_add(transit, 0, h, 0, 0);
      elm_transit_duration_set(transit, 0.75);
      elm_transit_go(transit);
    }

  _update_cur_file(doc);
}

static void
_fs_open_done(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
      void *event_info)
{
   const char *selected = event_info;
   if (selected)
      _open_file(main_doc, selected);
}

void
save_do(const char *file, Ecrire_Doc *doc)
{
  const char *filename = NULL;

  if(doc->code->file->file)
    filename = eina_file_filename_get(doc->code->file->file);

  /* New file name, another file opened, close first */
  if(file && filename && strcmp(file, filename))
    {
      eina_file_close(doc->code->file->file);
      doc->code->file->file = NULL;
    }

  /* File closed, open one, create if does not exist */
  if(file && !doc->code->file->file)
    {
      FILE *fp = NULL;
      fp = fopen(file,"w");
      if(fp)
        {
          fclose(fp);
          doc->code->file->file = eina_file_open(file,EINA_FALSE);
        }
    }

  /* File open, save */
  if(doc->code->file->file)
    {
      elm_code_file_save (doc->code->file);
      _set_save_disabled(doc, EINA_TRUE);
      _update_cur_file(doc);
    }
}

static void
_fs_save_done(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
      void *event_info)
{
   const char *selected = event_info;

   if (selected)
     {
        save_do(selected, main_doc);
     }
}

static void
_open_do(void *data)
{
   Ecrire_Doc *doc = data;
   ui_file_open_save_dialog_open(doc, _fs_open_done, EINA_TRUE);
}

static void
_goto_line(void *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   ui_goto_dialog_open(doc->win, doc);
}

static void
_open_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   _alert_if_need_saving(_open_do, doc);
}

void
editor_save(Ecrire_Doc *doc, void *callback_func)
{
  const char *filename = NULL;

  if(doc->code->file->file)
    filename = eina_file_filename_get(doc->code->file->file);
  if (filename)
    save_do(filename, doc);
  else
    ui_file_open_save_dialog_open(doc, callback_func, EINA_TRUE);
}

static void
_save(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   editor_save(doc, _fs_save_done);
}

static void
_save_as(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   ui_file_open_save_dialog_open(doc, _fs_save_done, EINA_TRUE);
}

static void
_new_do(void *data)
{
   Ecrire_Doc *doc = data;
   _close_doc(doc);
   _new_doc(doc);
}

static void
_new(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   _alert_if_need_saving(_new_do, doc);
}

static void
_cut(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   elm_code_widget_selection_cut(doc->widget);
}

static void
_copy(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   elm_code_widget_selection_copy(doc->widget);
}

static void
_paste(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   elm_code_widget_selection_paste(doc->widget);
}

static void
_find(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   ui_find_dialog_open(doc->win, doc);
}

static void
_settings(void *data,
          Evas_Object *obj EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   ui_settings_dialog_open(elm_object_top_widget_get(doc->win), doc, _ent_cfg);
}

static void
_win_del_do(void *data)
{
   Ecrire_Doc *doc = data;
  ecrire_cfg_save();
   _close_doc(doc);
   evas_object_del(doc->win);
  if(doc->path)
    free(doc->path);
   free(doc);
   elm_exit();
}

static void
_close_cb(void *data,
          Evas_Object *obj EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
  _alert_if_need_saving(_win_del_do, (Ecrire_Doc *)data);
}

static void
my_win_del(void *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   _alert_if_need_saving(_win_del_do, doc);
}

static Eina_Bool
_activate_paste_cb(void *data,
                   Evas_Object *obj EINA_UNUSED,
                   Elm_Selection_Data *event)
{
  if (!event)
    return EINA_FALSE;

  Ecrire_Doc *doc = data;

  elm_object_item_disabled_set(doc->paste_item,
                               (event->data ? EINA_FALSE : EINA_TRUE));

  return EINA_TRUE;
}

static Eina_Bool
_get_clipboard_cb(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *ev EINA_UNUSED)
{
  Ecrire_Doc *doc = data;

  elm_cnp_selection_get(doc->win,
                        ELM_SEL_TYPE_CLIPBOARD,
                        ELM_SEL_FORMAT_TARGETS,
                        _activate_paste_cb,
                        doc);

  return EINA_TRUE;
}

static void
_drop_do(void *data)
{
  if(drop_file)
    {
       Ecrire_Doc *doc = data;
       // FIXME: Need to set/pass document vs using global
       _open_file(doc, drop_file);
       free(drop_file);
       drop_file = NULL;
    }
}

static Eina_Bool
_drop_cb(void *data, Evas_Object *obj EINA_UNUSED, Elm_Selection_Data *event)
{
  Ecrire_Doc *doc = data;
  if(event && event->data)
    {
      // FIXME: Need to set/pass filename vs using global
      const char *file = event->data;
      int len = strlen(file);
      if(drop_file)
        free(drop_file); // just in case
      drop_file = (char *)malloc(len+1);
      if(drop_file)
        {
          strncpy(drop_file,file,len);
          _alert_if_need_saving(_drop_do, doc);
        }
    }
  return EINA_TRUE;
}

static Eina_Bool
_win_move_cb(void *data EINA_UNUSED, Evas_Object *obj, void *ev EINA_UNUSED)
{
  evas_object_geometry_get(obj,
                           NULL,
                           NULL,
                           &(_ent_cfg->width),
                           &(_ent_cfg->height));
  return EINA_TRUE;
}

static Eina_Bool
_key_down_cb(void *data,
             Evas_Object *obj,
             void *ev)
{
    Ecore_Event_Key *event = ev;
    if(ctrl_pressed)
      {
        ctrl_pressed = EINA_FALSE;
        if(!strcmp("F", event->key) ||
           !strcmp("f", event->key) ||
           !strcmp("H", event->key) ||
           !strcmp("h", event->key) ||
           !strcmp("R", event->key) ||
           !strcmp("r", event->key))
            _find(data,NULL,NULL);
        else if(!strcmp("O", event->key) ||
                !strcmp("o", event->key))
            _open_cb(data,NULL,NULL);
        else if(!strcmp("S", event->key) ||
                !strcmp("s", event->key))
            _save(data,NULL,NULL);
        else if(!strcmp("W", event->key) ||
                !strcmp("w", event->key))
            _close_cb(data,NULL,NULL);
      }
    else if (!strcmp("Control_L", event->key) ||
             !strcmp("Control_R", event->key))
      {
        ctrl_pressed = EINA_TRUE;
      }
  return EINA_TRUE;
}

static void
create_window(int argc, char *argv[])
{
   Evas_Object  *obj, *tbar;
   Evas_Coord w = 600, h = 600;

   main_doc = calloc(1, sizeof(*main_doc));
   main_doc->unsaved = 1;

   main_doc->win = elm_win_add(NULL, "editor", ELM_WIN_BASIC);
   elm_win_alpha_set (main_doc->win, EINA_TRUE);
   elm_win_autodel_set(main_doc->win, EINA_FALSE);

   main_doc->bg = elm_bg_add (main_doc->win);
   if(_ent_cfg->alpha)
     ALPHA (main_doc->bg, _ent_cfg->alpha);
   elm_win_resize_object_add (main_doc->win, main_doc->bg);
   evas_object_size_hint_weight_set (main_doc->bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show (main_doc->bg);

   main_doc->box_main = obj = elm_box_add (main_doc->win);
   if(_ent_cfg->alpha)
     ALPHA (main_doc->box_main, _ent_cfg->alpha);
   evas_object_size_hint_weight_set (obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add (main_doc->win, obj);
   evas_object_show (obj);

   tbar = elm_toolbar_add(main_doc->win);
   elm_toolbar_homogeneous_set(tbar, 0);
   elm_toolbar_shrink_mode_set(tbar, ELM_TOOLBAR_SHRINK_SCROLL);
   elm_toolbar_select_mode_set(tbar, ELM_OBJECT_SELECT_MODE_NONE);
   elm_toolbar_align_set(tbar, 0.0);
   evas_object_size_hint_weight_set(tbar, 0.0, 0.0);
   evas_object_size_hint_align_set(tbar, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(main_doc->box_main, tbar);
   evas_object_show(tbar);

   main_doc->box_editor = obj = elm_box_add (main_doc->win);
   if(_ent_cfg->alpha)
     ALPHA (obj, _ent_cfg->alpha);
   evas_object_size_hint_align_set (obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(obj);

   main_doc->code = elm_code_create();
   main_doc->widget = efl_add(elm_code_widget_class_get(),
                              main_doc->win,
                              elm_obj_code_widget_code_set(efl_added,
                                                           main_doc->code));
   _init_font(main_doc);
   elm_code_file_line_append(main_doc->code->file, "", 0, NULL);
   elm_obj_code_widget_editable_set(main_doc->widget, EINA_TRUE);
   elm_obj_code_widget_syntax_enabled_set(main_doc->widget, EINA_TRUE);
   elm_obj_code_widget_show_whitespace_set(main_doc->widget, EINA_TRUE);
   elm_obj_code_widget_line_numbers_set(main_doc->widget, _ent_cfg->line_numbers);
   evas_object_size_hint_align_set(main_doc->widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(main_doc->widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(main_doc->box_editor, main_doc->widget);
   elm_box_pack_end(main_doc->box_main, main_doc->box_editor);
   evas_object_show(main_doc->widget);

   main_doc->cursor_label = obj = elm_label_add(main_doc->win);
   _cur_changed(main_doc, NULL, NULL);
   evas_object_size_hint_align_set(obj, 0, 0.5);
   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(main_doc->box_main, obj);
   evas_object_show(obj);

   evas_object_smart_callback_add(main_doc->widget, "cursor,changed",
         _cur_changed, main_doc);
   evas_object_smart_callback_add(main_doc->widget, "changed,user", _changed, main_doc);
   evas_object_smart_callback_add(main_doc->widget, "undo,request", _undo, main_doc);
   evas_object_smart_callback_add(main_doc->widget, "redo,request", _redo, main_doc);
   evas_object_smart_callback_add(main_doc->widget, "selection,start", _sel_start, main_doc);
   evas_object_smart_callback_add(main_doc->widget, "selection,cleared", _sel_clear, main_doc);
   evas_object_smart_callback_add(main_doc->widget, "selection,copy", _sel_cut_copy, main_doc);
   evas_object_smart_callback_add(main_doc->widget, "selection,cut", _sel_cut_copy, main_doc);

   elm_toolbar_item_append(tbar, "document-new", _("New"), _new, main_doc);
   elm_toolbar_item_append(tbar, "document-open", _("Open"), _open_cb, main_doc);
   main_doc->close_item =
     elm_toolbar_item_append(tbar, "document-close", _("Close"), _close_cb, main_doc);
   main_doc->save_item =
     elm_toolbar_item_append(tbar, "document-save", _("Save"), _save, main_doc);
   main_doc->save_as_item =
     elm_toolbar_item_append(tbar, "document-save-as", _("Save As"), _save_as, main_doc);
   elm_toolbar_item_separator_set(
         elm_toolbar_item_append(tbar, "", "", NULL, NULL), EINA_TRUE);
   main_doc->undo_item =
      elm_toolbar_item_append(tbar, "edit-undo", _("Undo"), _undo, main_doc);
   main_doc->redo_item =
      elm_toolbar_item_append(tbar, "edit-redo", _("Redo"), _redo, main_doc);
   elm_toolbar_item_separator_set(
         elm_toolbar_item_append(tbar, "", "", NULL, NULL), EINA_TRUE);
   main_doc->cut_item = elm_toolbar_item_append(tbar, "edit-cut", _("Cut"), _cut, main_doc);
   main_doc->copy_item =
      elm_toolbar_item_append(tbar, "edit-copy", _("Copy"), _copy, main_doc);
   main_doc->paste_item =
      elm_toolbar_item_append(tbar, "edit-paste", _("Paste"), _paste, main_doc);
   elm_toolbar_item_separator_set(
         elm_toolbar_item_append(tbar, "", "", NULL, NULL), EINA_TRUE);
   elm_toolbar_item_append(tbar, "edit-find-replace", _("Search"),
         _find, main_doc);
   elm_toolbar_item_append(tbar, "go-jump", _("Jump to"), _goto_line, main_doc);
   elm_toolbar_item_separator_set(
         elm_toolbar_item_append(tbar, "", "", NULL, NULL), EINA_TRUE);
   elm_toolbar_item_append(tbar, "preferences-system", _("Settings"),
         _settings, main_doc);

   /* We don't have a selection when we start, make the items disabled */
   elm_object_item_disabled_set(main_doc->close_item, EINA_TRUE);
   _set_cut_copy_disabled(main_doc, EINA_TRUE);
   elm_object_item_disabled_set(main_doc->paste_item, EINA_TRUE);
   _set_save_disabled(main_doc, EINA_TRUE);
   _set_undo_redo_disabled(main_doc, EINA_TRUE);

   elm_drop_target_add(main_doc->widget,
                       ELM_SEL_FORMAT_IMAGE,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       _drop_cb,
                       main_doc);
   ecore_event_handler_add(ECORE_EVENT_KEY_DOWN,
                           (Ecore_Event_Handler_Cb)_key_down_cb,
                           main_doc);
   evas_object_smart_callback_add(main_doc->win,
                                  "delete,request",
                                  my_win_del,
                                  main_doc);
   evas_object_smart_callback_add(main_doc->win,
                                  "focused",
                                  (Evas_Smart_Cb)_get_clipboard_cb,
                                  main_doc);
   evas_object_smart_callback_add(main_doc->win,
                                  "move",
                                  (Evas_Smart_Cb)_win_move_cb,
                                  main_doc->win);

   if(_ent_cfg->height && _ent_cfg->width)
     evas_object_resize(main_doc->win, _ent_cfg->width, _ent_cfg->height);
   else
     evas_object_resize(main_doc->win, w, h);
   evas_object_show(main_doc->win);

   if (optind < argc)
     {
       _open_file(main_doc, argv[optind]);
       DBG("Opening filename: '%s'", argv[optind]);
     }
   else
     _update_cur_file(main_doc);

   elm_object_focus_set(main_doc->widget, EINA_TRUE);
}

int
main(int argc, char *argv[])
{
   int c;

   opterr = 0;

   if (!eina_init())
     {
        printf("Failed to initialize Eina_log module\n");
        return EXIT_FAILURE;
     }

   _ecrire_log_dom = eina_log_domain_register("ecrire", ECRIRE_DEFAULT_LOG_COLOR);
   if (_ecrire_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create a log domain.");
        exit(-1);
     }

   while ((c = getopt (argc, argv, "")) != -1)
     {
        switch (c)
          {
           case '?':
              print_usage(argv[0]);
              if (isprint (optopt))
                {
                   ERR("Unknown option or requires an argument `-%c'.",
                         optopt);
                }
              else
                {
                   ERR("Unknown option character `\\x%x'.", optopt);
                }
              return 1;
              break;
           default:
              abort();
          }
     }

   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALE_DIR);
   textdomain(PACKAGE);

   elm_init(argc, argv);

   ecrire_cfg_init(PACKAGE_NAME);
   ecrire_cfg_load();

   create_window(argc, argv);

   elm_run();

   ecrire_cfg_shutdown();
   elm_shutdown();
   eina_log_domain_unregister(_ecrire_log_dom);
   _ecrire_log_dom = -1;
   eina_shutdown();

   return 0;
}

static void
print_usage(const char *bin)
{
   fprintf(stderr,
         "Usage: %s [filename]\n", bin);
}
