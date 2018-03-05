#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <Elementary.h>
#include <Ecore_Getopt.h>

#include "ecrire.h"
#include "cfg.h"
#include "ui/ui.h"

static const Ecore_Getopt options =
{
   "ecrire",
   "%prog [OPTION]... [FILE]",
   VERSION,
   "(C) 2017 Obsidian-Studios, Inc. see AUTHORS.",
   "GPL-3.0, see COPYING",
   "Open source text editor using EFL",
   EINA_TRUE,
   {
      ECORE_GETOPT_HELP ('h', "help"),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_COPYRIGHT('R', "copyright"),
      ECORE_GETOPT_LICENSE('L', "license"),
      ECORE_GETOPT_SENTINEL
   }
};
static Eina_Unicode plain_utf8 = EINA_TRUE;

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
  if(!_ent_cfg->menu)
    {
      elm_object_item_disabled_set(doc->mm_cut, disabled);
      elm_object_item_disabled_set(doc->mm_copy, disabled);
      elm_object_item_disabled_set(doc->mm_select_all, disabled);
    }
  if(!_ent_cfg->toolbar)
    {
      elm_object_item_disabled_set(doc->cut_item, disabled);
      elm_object_item_disabled_set(doc->copy_item, disabled);
      elm_object_item_disabled_set(doc->select_all_item, disabled);
    }
}

static void
_set_save_disabled(Ecrire_Doc *doc, Eina_Bool disabled)
{
  if(!_ent_cfg->menu)
    {
      elm_object_item_disabled_set(doc->mm_save, disabled);
      elm_object_item_disabled_set(doc->mm_save_as, disabled);
    }
  if(!_ent_cfg->toolbar)
    {
      elm_object_item_disabled_set(doc->save_item, disabled);
      elm_object_item_disabled_set(doc->save_as_item, disabled);
    }
}

static void
_set_undo_redo_disabled(Ecrire_Doc *doc, Eina_Bool disabled)
{
  if(!_ent_cfg->menu)
    {
      elm_object_item_disabled_set(doc->mm_undo, disabled);
      elm_object_item_disabled_set(doc->mm_redo, disabled);
    }
  if(!_ent_cfg->toolbar)
    {
       elm_object_item_disabled_set(doc->undo_item, disabled);
       elm_object_item_disabled_set(doc->redo_item, disabled);
    }
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
   if (!_ent_cfg->toolbar &&
       !elm_object_item_disabled_get(doc->save_item))
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
  if(!_ent_cfg->menu)
    elm_object_item_disabled_set(doc->mm_paste, EINA_FALSE);
  if(!_ent_cfg->toolbar)
    elm_object_item_disabled_set(doc->paste_item, EINA_FALSE);
}

static void
_update_cur_file(Ecrire_Doc *doc)
{
  const char *filename = NULL;
  const char *saving = NULL;

  saving = ((!_ent_cfg->menu &&
            !elm_object_item_disabled_get(doc->mm_save)) ||
            (!_ent_cfg->toolbar &&
            !elm_object_item_disabled_get(doc->save_item))) ? "*" : "";
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
   char buf[sizeof(long)];
   unsigned int line;
   unsigned int col;

   Ecrire_Doc *doc = data;
   elm_obj_code_widget_cursor_position_get(doc->widget,&line,&col);
   snprintf(buf, sizeof(buf),"%d", line);
   elm_object_text_set(doc->entry_line, buf);
   snprintf(buf, sizeof(buf),"%d", col);
   elm_object_text_set(doc->entry_column, buf);
   if(elm_obj_code_widget_can_undo_get(doc->widget))
     {
       if(!_ent_cfg->menu &&
          elm_object_item_disabled_get(doc->mm_undo))
         elm_object_item_disabled_set(doc->mm_undo, EINA_FALSE);
       if(!_ent_cfg->toolbar &&
          elm_object_item_disabled_get(doc->undo_item))
         elm_object_item_disabled_set(doc->undo_item, EINA_FALSE);
     }
}

static void
_check_set_redo(Ecrire_Doc *doc)
{
  if(!_ent_cfg->menu)
    elm_object_item_disabled_set(doc->mm_redo,
                                 !elm_obj_code_widget_can_redo_get(doc->widget));
  if(!_ent_cfg->toolbar)
    elm_object_item_disabled_set(doc->redo_item,
                                 !elm_obj_code_widget_can_redo_get(doc->widget));
}

static void
_check_set_undo(Ecrire_Doc *doc)
{
  if(!_ent_cfg->menu)
    elm_object_item_disabled_set(doc->undo_item,
                                 !elm_obj_code_widget_can_undo_get(doc->widget));
  if(!_ent_cfg->toolbar)
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
_changed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
  Ecrire_Doc *doc = data;
  _set_save_disabled(doc, EINA_FALSE);
  if(!_ent_cfg->toolbar)
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
  _init_font(doc);
  if(!_ent_cfg->toolbar)
    elm_object_item_disabled_set(doc->close_item, EINA_TRUE);
  _set_save_disabled(doc, EINA_TRUE);
  _set_undo_redo_disabled(doc, EINA_TRUE);
  _update_cur_file(doc);
  elm_object_text_set(doc->label_mime,"");
}

static void
_open_file(Ecrire_Doc *doc, const char *file)
{
  const char *item;
  const char *mime;
  int h;
  Eina_List *find_list;
  Eina_List *list;

  if (file)
    {
      mime = efreet_mime_type_get(file);
      if(mime)
        {
          if(!strcasecmp(mime, "text/x-diff") ||
             !strcasecmp(mime, "text/x-patch"))
            elm_code_parser_standard_add(doc->code,
                                         ELM_CODE_PARSER_STANDARD_DIFF);
          else
            {
              doc->code->parsers = eina_list_remove(doc->code->parsers,
                                                    ELM_CODE_PARSER_STANDARD_DIFF);
              if(strstr(mime, "text/"))
                elm_obj_code_widget_syntax_enabled_set(doc->widget, EINA_TRUE);
              else
                elm_obj_code_widget_syntax_enabled_set(doc->widget, EINA_FALSE);
            }
          elm_object_text_set(doc->label_mime,mime);
        }
      elm_code_file_open(doc->code,file);
      _init_font(doc);

      if(strlen(file)>2)
        {
          if(eina_list_count(_ent_cfg->recent) >= ECRIRE_RECENT_COUNT)
            {
              item = eina_list_last_data_get(_ent_cfg->recent);
              list = eina_list_remove(_ent_cfg->recent, item);
              if(list)
                  _ent_cfg->recent = list;
            }
          find_list = eina_list_data_find_list(_ent_cfg->recent,file);
          if(find_list==NULL)
            {
              list = eina_list_prepend(_ent_cfg->recent,strdup(file));
              if(list)
                _ent_cfg->recent = list;
            }
          else
            {
              list = eina_list_promote_list(_ent_cfg->recent,find_list);
              if(list)
                _ent_cfg->recent = list;
            }
        }

      _set_path(doc,file);
      _set_save_disabled(doc, EINA_TRUE);
      _set_cut_copy_disabled(doc, EINA_TRUE);
      _set_undo_redo_disabled(doc, EINA_TRUE);
      if(!_ent_cfg->toolbar)
        elm_object_item_disabled_set(doc->close_item, EINA_FALSE);

      if(!_ent_cfg->anim_open)
        {
          Elm_Transit *transit = elm_transit_add();
          elm_transit_object_add(transit, doc->box_editor);
          evas_object_geometry_get(doc->win, NULL, NULL, NULL, &h);
          elm_transit_effect_translation_add(transit, 0, h, 0, 0);
          elm_transit_duration_set(transit, 0.75);
          elm_transit_go(transit);
        }
    }

  _update_cur_file(doc);
}

static void
_fs_open_done(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
  const char *selected = event_info;
  if (selected)
    _open_file((Ecrire_Doc *)data, selected);
}

void
save_do(const char *file, Ecrire_Doc *doc)
{
  const char *filename = NULL;
  const char *mime = NULL;

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

  mime = efreet_mime_type_get(file);
  if(mime)
    elm_object_text_set(doc->label_mime,mime);
}

static void
_fs_save_done(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
  const char *selected = event_info;

  if (selected)
    save_do(selected, (Ecrire_Doc *)data);
}

static void
_open_do(void *data)
{
   Ecrire_Doc *doc = data;
   ui_file_open_save_dialog_open(doc, _fs_open_done, EINA_TRUE);
}

static void
_goto_column_cb(void *data,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
  Ecrire_Doc *doc;
  Elm_Code_Line *line;
  unsigned int col;
  int cols;
  unsigned int cur_col;
  unsigned int row;

  doc = data;
  elm_obj_code_widget_cursor_position_get(doc->widget,&row,&cur_col);
  col = atoi(elm_entry_entry_get(doc->entry_column));
  line = elm_code_file_line_get(doc->code->file,row);
  cols = elm_obj_code_widget_line_text_column_width_get(doc->widget, line);
  if (col>cols)
      col = cur_col;
  elm_obj_code_widget_cursor_position_set(doc->widget,row,col);
  elm_object_focus_set(doc->widget, EINA_TRUE);
}

static void
_goto_line_cb(void *data,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
  Ecrire_Doc *doc;
  int line;
  int lines;

  doc = data;
  line = atoi(elm_entry_entry_get(doc->entry_line));
  lines = elm_code_file_lines_get(doc->code->file);
  if (line>0 && lines > 0 && line <= lines)
      elm_obj_code_widget_cursor_position_set(doc->widget,line,1);
  else
    {
      char buf[sizeof(long)];
      unsigned int col;
      unsigned int row;

      elm_obj_code_widget_cursor_position_get(doc->widget,&row,&col);
      snprintf(buf, sizeof(buf),"%d",row);
      elm_entry_entry_set(doc->entry_line,buf);
    }
  elm_object_focus_set(doc->widget, EINA_TRUE);
}

static void
_goto_line_focus_cb(void *data,
                    Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   elm_object_focus_set(doc->entry_line, EINA_TRUE);
   elm_entry_select_all(doc->entry_line);
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
_select_all_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
   elm_code_widget_selection_select_all(doc->widget);
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
  if(!_ent_cfg->toolbar)
    elm_object_item_disabled_set(doc->mm_paste,
                                 (event->data ? EINA_FALSE : EINA_TRUE));
  if(!_ent_cfg->toolbar)
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
      int len = strlen(file)+1;
      if(drop_file)
        free(drop_file); // just in case
      drop_file = (char *)malloc(len);
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
_key_down_cb(void *data, EINA_UNUSED Evas_Object *obj, void *ev)
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
        else if(!strcmp("G", event->key) ||
                !strcmp("g", event->key))
            _goto_line_focus_cb(data,NULL,NULL);
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

void
add_toolbar(Ecrire_Doc *doc)
{
  Evas_Object  *tbar;

  doc->toolbar = tbar = elm_toolbar_add(doc->win);
  elm_toolbar_homogeneous_set(tbar, 0);
  elm_toolbar_shrink_mode_set(tbar, ELM_TOOLBAR_SHRINK_SCROLL);
  elm_toolbar_select_mode_set(tbar, ELM_OBJECT_SELECT_MODE_NONE);
  elm_toolbar_align_set(tbar, 0);
  evas_object_size_hint_align_set(tbar, EVAS_HINT_FILL, 0);
  elm_box_pack_start(doc->box_main, tbar);
  evas_object_show(tbar);

  elm_toolbar_item_append(tbar, "document-new", _("New"), _new, doc);
  elm_toolbar_item_append(tbar, "document-open", _("Open"), _open_cb, doc);
  doc->close_item =
    elm_toolbar_item_append(tbar, "document-close", _("Close"), _close_cb, doc);
  doc->save_item =
    elm_toolbar_item_append(tbar, "document-save", _("Save"), _save, doc);
  doc->save_as_item =
    elm_toolbar_item_append(tbar, "document-save-as", _("Save As"), _save_as, doc);
  elm_toolbar_item_separator_set(
        elm_toolbar_item_append(tbar, "", "", NULL, NULL), EINA_TRUE);
  doc->undo_item =
     elm_toolbar_item_append(tbar, "edit-undo", _("Undo"), _undo, doc);
  doc->redo_item =
     elm_toolbar_item_append(tbar, "edit-redo", _("Redo"), _redo, doc);
  elm_toolbar_item_separator_set(
        elm_toolbar_item_append(tbar, "", "", NULL, NULL), EINA_TRUE);
  doc->cut_item = elm_toolbar_item_append(tbar, "edit-cut", _("Cut"), _cut, doc);
  doc->copy_item =
     elm_toolbar_item_append(tbar, "edit-copy", _("Copy"), _copy, doc);
  doc->paste_item =
     elm_toolbar_item_append(tbar, "edit-paste", _("Paste"), _paste, doc);
  doc->select_all_item =
     elm_toolbar_item_append(tbar, "edit-select-all", _("Select All"), _select_all_cb, doc);
  elm_toolbar_item_separator_set(
        elm_toolbar_item_append(tbar, "", "", NULL, NULL), EINA_TRUE);
  elm_toolbar_item_append(tbar, "edit-find-replace", _("Search"), _find, doc);
  elm_toolbar_item_append(tbar, "go-jump", _("Jump to"), _goto_line_focus_cb, doc);
  elm_toolbar_item_separator_set(
        elm_toolbar_item_append(tbar, "", "", NULL, NULL), EINA_TRUE);
  elm_toolbar_item_append(tbar, "preferences-system", _("Settings"),
        _settings, doc);
}

static void
create_window(int argc, char *argv[])
{
   Ecrire_Doc *doc;
   Evas_Object  *box;
   Evas_Object  *edit_menu;
   Evas_Object  *file_menu;
   Evas_Object  *menu;
   Evas_Object  *obj;
   Evas_Object  *table;
   Evas_Coord h = 600;
   Evas_Coord w = 600;

   doc = calloc(1, sizeof(*doc));
   doc->unsaved = 1;

   doc->win = elm_win_add(NULL, "editor", ELM_WIN_BASIC);
   elm_win_alpha_set (doc->win, EINA_TRUE);
   elm_win_autodel_set(doc->win, EINA_FALSE);

   doc->bg = elm_bg_add (doc->win);
   if(_ent_cfg->alpha)
     ALPHA (doc->bg, _ent_cfg->alpha);
   elm_win_resize_object_add (doc->win, doc->bg);
   evas_object_size_hint_weight_set (doc->bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show (doc->bg);

   doc->box_main = obj = elm_box_add (doc->win);
   if(_ent_cfg->alpha)
     ALPHA (doc->box_main, _ent_cfg->alpha);
   evas_object_size_hint_weight_set (obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add (doc->win, obj);
   evas_object_show (obj);

   if(!_ent_cfg->menu)
     {
       menu = elm_win_main_menu_get(doc->win);

       file_menu = elm_menu_item_add(menu, NULL, NULL, _("File"), NULL, NULL);
       elm_menu_item_add(menu, file_menu, "document-new", _("New"), _new, doc);
       elm_menu_item_add(menu, file_menu, "document-open", _("Open"), _open_cb, doc);
       doc->mm_save =
         elm_menu_item_add(menu, file_menu, "document-save", _("Save"), _save, doc);
       doc->mm_save_as =
         elm_menu_item_add(menu, file_menu, "document-save-as", _("Save As"), _save_as, doc);
       elm_menu_item_separator_add(menu, file_menu);
       elm_menu_item_add(menu, file_menu, "preferences-system", _("Settings"), _settings, doc);
       elm_menu_item_separator_add(menu, file_menu);
       elm_menu_item_add(menu, file_menu, "application-exit", _("Exit"), _close_cb, doc);

       edit_menu = elm_menu_item_add(menu, NULL, NULL, _("Edit"), NULL, NULL);
       doc->mm_undo =
         elm_menu_item_add(menu, edit_menu, "edit-undo", _("Undo"), _undo, doc);
       doc->mm_redo =
         elm_menu_item_add(menu, edit_menu, "edit-redo", _("Redo"), _redo, doc);
       elm_menu_item_separator_add(menu, edit_menu);
       doc->mm_cut =
         elm_menu_item_add(menu, edit_menu, "edit-cut", _("Cut"), _cut, doc);
       doc->mm_copy =
         elm_menu_item_add(menu, edit_menu, "edit-copy", _("Copy"), _copy, doc);
       doc->mm_paste =
         elm_menu_item_add(menu, edit_menu, "edit-paste", _("Paste"), _paste, doc);
       doc->mm_select_all =
         elm_menu_item_add(menu, edit_menu, "edit-select-all", _("Select All"), _select_all_cb, doc);
       elm_menu_item_separator_add(menu, edit_menu);
       elm_menu_item_add(menu, edit_menu, "edit-find-replace", _("Search"), _find, doc);
       elm_menu_item_add(menu, edit_menu, "go-jump", _("Jump to"), _goto_line_focus_cb, doc);

       elm_object_item_disabled_set(doc->mm_paste, EINA_TRUE);
     }

   if(!_ent_cfg->toolbar)
     {
       add_toolbar(doc);
       /* We don't have a selection when we start, make the items disabled */
       elm_object_item_disabled_set(doc->close_item, EINA_TRUE);
       elm_object_item_disabled_set(doc->paste_item, EINA_TRUE);
     }
   _set_cut_copy_disabled(doc, EINA_TRUE);
   _set_save_disabled(doc, EINA_TRUE);
   _set_undo_redo_disabled(doc, EINA_TRUE);

   doc->box_editor = obj = elm_box_add (doc->win);
   if(_ent_cfg->alpha)
     ALPHA (obj, _ent_cfg->alpha);
   evas_object_size_hint_align_set (obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(obj);

   doc->code = elm_code_create();
   doc->widget = elm_code_widget_add(doc->win, doc->code);

   _init_font(doc);
   elm_code_file_line_append(doc->code->file, "", 0, NULL);
   elm_obj_code_widget_editable_set(doc->widget, EINA_TRUE);
   elm_obj_code_widget_syntax_enabled_set(doc->widget, EINA_TRUE);
   elm_obj_code_widget_show_whitespace_set(doc->widget, EINA_TRUE);
   elm_obj_code_widget_line_numbers_set(doc->widget, !_ent_cfg->line_numbers);
   if(_ent_cfg->line_width_marker>0)
     elm_code_widget_line_width_marker_set(doc->widget, _ent_cfg->line_width_marker);
   elm_obj_code_widget_tab_inserts_spaces_set(doc->widget, !_ent_cfg->insert_spaces);
   evas_object_size_hint_align_set(doc->widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(doc->widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(doc->box_editor, doc->widget);
   elm_box_pack_end(doc->box_main, doc->box_editor);
   evas_object_show(doc->widget);

   box = elm_box_add (doc->win);
   elm_box_horizontal_set(box, EINA_TRUE);
   if(_ent_cfg->alpha)
     ALPHA (box, _ent_cfg->alpha);
   evas_object_size_hint_weight_set (box, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_padding_set(box, 5, 5, 0, 0);
   elm_box_pack_end(doc->box_main,box);
   evas_object_show (box);


   table = elm_table_add(box);
   elm_table_homogeneous_set(table, EINA_TRUE);
   elm_table_padding_set(table, 2, 0);
   evas_object_size_hint_align_set(table, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_start(box, table);
   evas_object_show(table);

   obj = elm_label_add(table);
   elm_object_text_set(obj, _("Line"));
   evas_object_size_hint_align_set(obj, 0, EVAS_HINT_FILL);
   elm_table_pack (table, obj, 0, 0, 1, 1);
   evas_object_show(obj);

   doc->entry_line = obj = elm_entry_add(table);
   elm_entry_scrollable_set(obj, EINA_TRUE);
   elm_entry_single_line_set(obj, EINA_TRUE);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack (table, obj, 1, 0, 2, 1);
   evas_object_show(obj);
   evas_object_smart_callback_add(doc->entry_line,
                                  "activated",
                                  _goto_line_cb,
                                  doc);
   evas_object_smart_callback_add(doc->entry_line,
                                  "clicked,triple",
                                  _goto_line_cb,
                                  doc);

   obj = elm_label_add(table);
   elm_object_text_set(obj, _("Column"));
   evas_object_size_hint_align_set(obj, 0, EVAS_HINT_FILL);
   elm_table_pack (table, obj, 3, 0, 2, 1);
   evas_object_show(obj);

   doc->entry_column = obj = elm_entry_add(table);
   elm_entry_scrollable_set(obj, EINA_TRUE);
   elm_entry_single_line_set(obj, EINA_TRUE);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack (table, obj, 5, 0, 2, 1);
   evas_object_show(obj);
   evas_object_smart_callback_add(doc->entry_column,
                                  "activated",
                                  _goto_column_cb,
                                  doc);
   evas_object_smart_callback_add(doc->entry_column,
                                  "clicked,triple",
                                  _goto_column_cb,
                                  doc);

   _cur_changed(doc, NULL, NULL);
   
   doc->label_mime = elm_label_add(box);
   evas_object_size_hint_weight_set(doc->label_mime, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(doc->label_mime, 1, EVAS_HINT_FILL);
   elm_box_pack_end(box, doc->label_mime);
   evas_object_show(doc->label_mime);

   evas_object_smart_callback_add(doc->widget, "cursor,changed",
         _cur_changed, doc);
   evas_object_smart_callback_add(doc->widget, "changed,user", _changed, doc);
   evas_object_smart_callback_add(doc->widget, "undo,request", _undo, doc);
   evas_object_smart_callback_add(doc->widget, "redo,request", _redo, doc);
   evas_object_smart_callback_add(doc->widget, "selection,start", _sel_start, doc);
   evas_object_smart_callback_add(doc->widget, "selection,cleared", _sel_clear, doc);
   evas_object_smart_callback_add(doc->widget, "selection,copy", _sel_cut_copy, doc);
   evas_object_smart_callback_add(doc->widget, "selection,cut", _sel_cut_copy, doc);

   elm_drop_target_add(doc->widget,
                       ELM_SEL_FORMAT_IMAGE,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       _drop_cb,
                       doc);
   ecore_event_handler_add(ECORE_EVENT_KEY_DOWN,
                           (Ecore_Event_Handler_Cb)_key_down_cb,
                           doc);
   evas_object_smart_callback_add(doc->win,
                                  "delete,request",
                                  my_win_del,
                                  doc);
   evas_object_smart_callback_add(doc->win,
                                  "focused",
                                  (Evas_Smart_Cb)_get_clipboard_cb,
                                  doc);
   evas_object_smart_callback_add(doc->win,
                                  "move",
                                  (Evas_Smart_Cb)_win_move_cb,
                                  doc->win);

   if(_ent_cfg->height && _ent_cfg->width)
     evas_object_resize(doc->win, _ent_cfg->width, _ent_cfg->height);
   else
     evas_object_resize(doc->win, w, h);
   evas_object_show(doc->win);

   if (optind < argc)
     {
       _open_file(doc, argv[optind]);
       DBG("Opening filename: '%s'", argv[optind]);
     }
   else
     _update_cur_file(doc);

   elm_object_focus_set(doc->widget, EINA_TRUE);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   int args;
   unsigned char quit_option = 0;
   Ecore_Getopt_Value values[] =
     {
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option)
     };

   args = ecore_getopt_parse(&options, values, argc, argv);
   if (quit_option)
     return EXIT_SUCCESS;

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
ELM_MAIN()
