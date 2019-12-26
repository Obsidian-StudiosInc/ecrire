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

static Eina_Bool _activate_paste_cb(void *data,
                                    Evas_Object *obj EINA_UNUSED,
                                    Elm_Selection_Data *event);
static Eina_Bool _drop_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          Elm_Selection_Data *event);
static Eina_Bool _get_clipboard_cb(void *data,
                                   Evas_Object *obj EINA_UNUSED,
                                   void *ev EINA_UNUSED);
static Eina_Bool _key_down_cb(void *data,
                              EINA_UNUSED Evas_Object *obj,
                              void *ev);
static Eina_Bool _key_up_cb(void *data,
                            EINA_UNUSED Evas_Object *obj,
                            void *ev);
static Eina_Bool _win_move_cb(void *data EINA_UNUSED,
                              Evas_Object *obj EINA_UNUSED,
                              void *ev EINA_UNUSED);

static void _add_to_recent_files(const char *file);
static void _alert_if_need_saving(void (*done)(void *data), Ecrire_Doc *doc);
static void _changed(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED);
static void _check_set_redo(Ecrire_Doc *doc);
static void _check_set_undo(Ecrire_Doc *doc);
static void _close_cb(void *data,
                      Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED);
static void _close_doc (void *data);
static void _copy(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED);
static void _cur_changed(void *data,
                         Evas_Object *obj EINA_UNUSED,
                         void *event_info EINA_UNUSED);
static void _cut(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED);
static void _drop_do(void *data);
static void _find(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED);
static void _fs_open_done(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info);
static void _fs_save_done(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info);
static void _goto_column_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED);
static void _goto_line_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info EINA_UNUSED);
static void _goto_line_focus_cb(void *data,
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info EINA_UNUSED);
static void _init_font(Ecrire_Doc *doc);
static void _mouse_down_cb(void *data,
                           EINA_UNUSED Evas *evas,
                           EINA_UNUSED Evas_Object *obj,
                           void *event);
static void _new(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED);
static void _new_do(void *data);
static void _new_doc(Ecrire_Doc *doc);
static void _open_cb(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED);
static void _open_do(void *data);
static void _open_file(Ecrire_Doc *doc, const char *file);
static void _paste(void *data,
                   Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED);
static void _redo(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED);
static void _save(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED);
static void _save_as(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED);
static void _sel_clear(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED);
static void _sel_cut_copy(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info EINA_UNUSED);
static void _sel_start(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED);
static void _select_all_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED);
static void _set_cut_copy_disabled(Ecrire_Doc *doc, Eina_Bool disabled);
static void _set_doc_changed(Ecrire_Doc *doc, Eina_Bool changed);
static void _set_path(Ecrire_Doc *doc, const char *file);
static void _set_save_disabled(Ecrire_Doc *doc, Eina_Bool disabled);
static void _set_undo_redo_disabled(Ecrire_Doc *doc, Eina_Bool disabled);
static void _settings(void *data,
                      Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED);
static void _signal_cb(int sig EINA_UNUSED);
static void _undo(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED);
static void _update_cur_file(Ecrire_Doc *doc);
static void _win_del_do(void *data);

static const Ecore_Getopt options =
{
   "ecrire",
   "%prog [OPTION]... [FILE]",
   VERSION,
   "(C) 2019 Obsidian-Studios, Inc. see AUTHORS.",
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

static Evas_Object *_bg;
static Evas_Object *_box_editor;
static Evas_Object *_box_main;
static Evas_Object *_menu;
static Evas_Object *_toolbar;
static Evas_Object *_win;
Eina_Bool ctrl_pressed = EINA_FALSE;
Eina_Bool shft_pressed = EINA_FALSE;
Eina_Bool delay_anim = EINA_FALSE;
/* specific log domain to help debug only ecrire */
int _ecrire_log_dom = -1;
static int _untitled = 1;
char *drop_file = NULL;

static void
_add_to_recent_files(const char *file)
{
  if(strlen(file)>2)
    {
      Eina_List *find_list = NULL;
      Eina_List *list = NULL;
      Eina_List *l;
      Eina_List *l_next;
      char absolute[PATH_MAX];
      char *data;
      struct stat buffer;

      realpath(file,absolute);
      EINA_LIST_FOREACH_SAFE(_ent_cfg->recent, l, l_next, data)
        {
          if(stat (data, &buffer) == -1)
            _ent_cfg->recent = eina_list_remove_list(_ent_cfg->recent, l);
        }

      find_list = eina_list_data_find_list(_ent_cfg->recent,
                                           eina_stringshare_add(absolute));
      if(find_list)
        list = eina_list_promote_list(_ent_cfg->recent,find_list);
      else
        list = eina_list_prepend(_ent_cfg->recent,strdup(absolute));
      if(list)
        _ent_cfg->recent = list;
      if(eina_list_count(_ent_cfg->recent) >= ECRIRE_RECENT_COUNT)
        {
          const char *item;

          item = eina_list_last_data_get(_ent_cfg->recent);
          list = eina_list_remove(_ent_cfg->recent, item);
          if(list)
              _ent_cfg->recent = list;
        }
    }
}

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
_set_doc_changed(Ecrire_Doc *doc, Eina_Bool changed)
{
  doc->changed = changed;
  _set_save_disabled(doc, !changed);
  _update_cur_file(doc);
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
#ifdef EFL_VERSION_1_22
  if(name)
    elm_code_widget_font_set(doc->widget, name, size);
  else
    elm_code_widget_font_set(doc->widget, NULL, size);
#else
  if(name)
    elm_obj_code_widget_font_set(doc->widget, name, size);
  else
    elm_obj_code_widget_font_set(doc->widget, NULL, size);
#endif
}

static void
_init_font(Ecrire_Doc *doc)
{
  editor_font_set(doc, _ent_cfg->font.name, _ent_cfg->font.size);
}

static void
_alert_if_need_saving(void (*done)(void *data), Ecrire_Doc *doc)
{
   if (doc->changed)
     ui_alert_need_saving(doc->widget, done, doc);
   else
     done(doc);
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
  char buf[1024];

  saving = (doc->changed>0) ? "*" : "";

  if(doc->code->file->file)
    filename = eina_file_filename_get(doc->code->file->file);
  if (filename)
     snprintf(buf, sizeof(buf), _("%s%s - %s"), saving,
              filename, PACKAGE_NAME);
  else
     snprintf(buf, sizeof(buf), _("%sUntitled %d - %s"), saving,
              _untitled, PACKAGE_NAME);

  elm_win_title_set(_win, buf);
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
#ifdef EFL_VERSION_1_22
   elm_code_widget_cursor_position_get(doc->widget,&line,&col);
#else
   elm_obj_code_widget_cursor_position_get(doc->widget,&line,&col);
#endif
   snprintf(buf, sizeof(buf),"%u", line);
   elm_object_text_set(doc->entry_line, buf);
   snprintf(buf, sizeof(buf),"%u", col);
   elm_object_text_set(doc->entry_column, buf);
#ifdef EFL_VERSION_1_22
   if(elm_code_widget_can_undo_get(doc->widget))
#else
   if(elm_obj_code_widget_can_undo_get(doc->widget))
#endif
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
#ifdef EFL_VERSION_1_22
  if(!_ent_cfg->menu)
    elm_object_item_disabled_set(doc->mm_redo,
                                 !elm_code_widget_can_redo_get(doc->widget));
  if(!_ent_cfg->toolbar)
    elm_object_item_disabled_set(doc->redo_item,
                                 !elm_code_widget_can_redo_get(doc->widget));
#else
  if(!_ent_cfg->menu)
    elm_object_item_disabled_set(doc->mm_redo,
                                 !elm_obj_code_widget_can_redo_get(doc->widget));
  if(!_ent_cfg->toolbar)
    elm_object_item_disabled_set(doc->redo_item,
                                 !elm_obj_code_widget_can_redo_get(doc->widget));
#endif
}

static void
_check_set_undo(Ecrire_Doc *doc)
{
  Eina_Bool can_undo;

#ifdef EFL_VERSION_1_22
  can_undo = elm_code_widget_can_undo_get(doc->widget);
#else
  can_undo = elm_code_widget_can_undo_get(doc->widget);
#endif
  if(!_ent_cfg->menu)
    elm_object_item_disabled_set(doc->undo_item, !can_undo);
  if(!_ent_cfg->toolbar)
    elm_object_item_disabled_set(doc->undo_item, !can_undo);
  if(!can_undo && doc->changed)
    _set_doc_changed(doc, EINA_FALSE);
  else if(can_undo && !doc->changed)
    _set_doc_changed(doc, EINA_TRUE);
}

static void
_signal_cb(int sig EINA_UNUSED)
{
  evas_object_smart_callback_call(_win, "delete,request", NULL);
}

static void
_undo(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   /* In undo we care about the current item */
   Ecrire_Doc *doc = data;
#ifdef EFL_VERSION_1_22
   elm_code_widget_undo(doc->widget);
#else
   elm_obj_code_widget_undo(doc->widget);
#endif
   _check_set_redo(doc);
   _check_set_undo(doc);
}

static void
_redo(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Doc *doc = data;
#ifdef EFL_VERSION_1_22
   elm_code_widget_redo(doc->widget);
#else
   elm_obj_code_widget_redo(doc->widget);
#endif
   _check_set_redo(doc);
   _check_set_undo(doc);
}

static void
_changed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
  Ecrire_Doc *doc = data;

  _check_set_redo(doc);
  _check_set_undo(doc);
}

static void
_close_doc (void *data)
{
  Ecrire_Doc *doc = data;
  elm_code_file_close(doc->code->file);
}

static void
_new_doc(Ecrire_Doc *doc)
{
  doc->changed = EINA_FALSE;
  elm_code_file_new(doc->code);
  elm_code_file_line_append(doc->code->file, "", 0, NULL);
  _init_font(doc);
  _set_save_disabled(doc, EINA_TRUE);
  _set_undo_redo_disabled(doc, EINA_TRUE);
  _untitled++;
  _update_cur_file(doc);
  elm_object_text_set(doc->label_mime,"");
}

static void
_open_file(Ecrire_Doc *doc, const char *file)
{
  if (file)
    {
      const char *mime;

      evas_object_hide(doc->widget);
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
#ifdef EFL_VERSION_1_22
              if(strstr(mime, "text/"))
                elm_code_widget_syntax_enabled_set(doc->widget, EINA_TRUE);
              else
                elm_code_widget_syntax_enabled_set(doc->widget, EINA_FALSE);
#else
              if(strstr(mime, "text/"))
                elm_obj_code_widget_syntax_enabled_set(doc->widget, EINA_TRUE);
              else
                elm_obj_code_widget_syntax_enabled_set(doc->widget, EINA_FALSE);
#endif
            }
          elm_object_text_set(doc->label_mime,mime);
        }
      elm_code_file_open(doc->code,file);
      _init_font(doc);
      _add_to_recent_files(file);
      _set_path(doc,file);
      _set_save_disabled(doc, EINA_TRUE);
      _set_cut_copy_disabled(doc, EINA_TRUE);
      _set_undo_redo_disabled(doc, EINA_TRUE);

      if(!_ent_cfg->anim_open)
        {
          int h;

          Elm_Transit *transit = elm_transit_add();
          elm_transit_objects_final_state_keep_set(transit, EINA_TRUE);
          elm_transit_object_add(transit, doc->widget);
          evas_object_geometry_get(_win, NULL, NULL, NULL, &h);
          elm_transit_effect_translation_add(transit, 0, h, 0, 0);
          elm_transit_duration_set(transit, 0.75);
          if(delay_anim)
            {
              delay_anim = EINA_FALSE;
              elm_transit_go_in(transit, 0.25);
            }
          else
            elm_transit_go(transit);
        }
      evas_object_show(doc->widget);
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
      _add_to_recent_files(eina_file_filename_get(doc->code->file->file));
      elm_code_file_save (doc->code->file);
      _set_save_disabled(doc, EINA_TRUE);
      doc->changed = EINA_FALSE;
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
   ui_file_open_save_dialog_open(doc, _fs_open_done, EINA_FALSE);
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
#ifdef EFL_VERSION_1_22
  elm_code_widget_cursor_position_get(doc->widget,&row,&cur_col);
#else
  elm_obj_code_widget_cursor_position_get(doc->widget,&row,&cur_col);
#endif
  col = atoi(elm_entry_entry_get(doc->entry_column));
  line = elm_code_file_line_get(doc->code->file,row);
#ifdef EFL_VERSION_1_22
  cols = elm_code_widget_line_text_column_width_get(doc->widget, line);
#else
  cols = elm_obj_code_widget_line_text_column_width_get(doc->widget, line);
#endif
  if (col>cols)
      col = cur_col;
#ifdef EFL_VERSION_1_22
  elm_code_widget_cursor_position_set(doc->widget,row,col);
#else
  elm_obj_code_widget_cursor_position_set(doc->widget,row,col);
#endif
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
#ifdef EFL_VERSION_1_22
      elm_code_widget_cursor_position_set(doc->widget,line,1);
#else
      elm_obj_code_widget_cursor_position_set(doc->widget,line,1);
#endif
  else
    {
      char buf[sizeof(long)];
      unsigned int col;
      unsigned int row;

#ifdef EFL_VERSION_1_22
      elm_code_widget_cursor_position_get(doc->widget,&row,&col);
#else
      elm_obj_code_widget_cursor_position_get(doc->widget,&row,&col);
#endif
      snprintf(buf, sizeof(buf),"%u",row);
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
    {
      save_do(filename, doc);
      void (*func) (void *, Evas_Object *, void *) = callback_func;
      func(doc,NULL,(char *)filename);
    }
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
   ui_find_dialog_open(_win, doc);
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
   ui_settings_dialog_open(elm_object_top_widget_get(_win), doc, _ent_cfg);
}

static void
_win_del_do(void *data)
{
  Ecrire_Doc *doc = data;
  ecrire_cfg_save();
  if(_menu)
    evas_object_del(_menu);
  if(_toolbar)
    evas_object_del(_toolbar);
  evas_object_del(doc->widget);
  elm_code_free(doc->code);
  if(_win)
    evas_object_del(_win);
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

  elm_cnp_selection_get(_win,
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
_win_move_cb(void *data EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *ev EINA_UNUSED)
{
  evas_object_geometry_get(_win,
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
        if(!strcmp("End", event->key))
          {
            Ecrire_Doc *doc;
            Elm_Code_Line *line;
            unsigned int cols;
            unsigned int lines;

            doc = data;
            lines = elm_code_file_lines_get(doc->code->file);
            line = elm_code_file_line_get(doc->code->file, lines);
#ifdef EFL_VERSION_1_22
            cols = elm_code_widget_line_text_column_width_get(doc->widget, line);
            elm_code_widget_cursor_position_set(doc->widget, lines, cols+1);
#else
            cols = elm_obj_code_widget_line_text_column_width_get(doc->widget, line);
            elm_obj_code_widget_cursor_position_set(doc->widget, lines, cols+1);
#endif
          }
        else if(!strcmp("F", event->key) ||
                !strcmp("f", event->key) ||
                !strcmp("H", event->key) ||
                !strcmp("h", event->key) ||
                !strcmp("R", event->key) ||
                !strcmp("r", event->key))
            _find(data,NULL,NULL);
        else if(!strcmp("Home", event->key))
#ifdef EFL_VERSION_1_22
            elm_code_widget_cursor_position_set(((Ecrire_Doc *)data)->widget,1,1);
#else
            elm_obj_code_widget_cursor_position_set(((Ecrire_Doc *)data)->widget,1,1);
#endif
        else if(!strcmp("G", event->key) ||
                !strcmp("g", event->key))
            _goto_line_focus_cb(data,NULL,NULL);
        else if(!strcmp("Insert", event->key))
            elm_code_widget_selection_copy(((Ecrire_Doc *)data)->widget);
        else if(!strcmp("O", event->key) ||
                !strcmp("o", event->key))
            _open_cb(data,NULL,NULL);
        else if(!strcmp("Y", event->key) ||
                !strcmp("y", event->key))
            _check_set_redo((Ecrire_Doc *)data);
        else if(!strcmp("S", event->key) ||
                !strcmp("s", event->key))
            _save(data,NULL,NULL);
        else if(!strcmp("Z", event->key) ||
                !strcmp("z", event->key))
          {
            Ecrire_Doc *doc;

            doc = (Ecrire_Doc *)data;
            _check_set_redo(doc);
            _check_set_undo(doc);
          }
        else if(!strcmp("W", event->key) ||
                !strcmp("w", event->key))
            _close_cb(data,NULL,NULL);
      }
    else if(shft_pressed &&
            !strcmp("Insert", event->key))
        elm_code_widget_selection_paste(((Ecrire_Doc *)data)->widget);
    else if (!strcmp("Control_L", event->key) ||
             !strcmp("Control_R", event->key))
        ctrl_pressed = EINA_TRUE;
    else if (!strcmp("Shift_L", event->key) ||
             !strcmp("Shift_R", event->key))
        shft_pressed = EINA_TRUE;
  return EINA_TRUE;
}

static Eina_Bool
_key_up_cb(void *data, EINA_UNUSED Evas_Object *obj, void *ev)
{
  Ecore_Event_Key *event = ev;
    if (!strcmp("Control_L", event->key) ||
        !strcmp("Control_R", event->key))
      ctrl_pressed = EINA_FALSE;
    else if (!strcmp("Shift_L", event->key) ||
             !strcmp("Shift_R", event->key))
      shft_pressed = EINA_FALSE;
  return EINA_TRUE;
}

static void
_mouse_down_cb(void *data,
               EINA_UNUSED Evas *evas,
               EINA_UNUSED Evas_Object *obj,
               void *event)
{
  Evas_Event_Mouse_Down *e = (Evas_Event_Mouse_Down *)event;
  switch (e->button)
    {
      case 2:
        elm_code_widget_selection_paste(((Ecrire_Doc *)data)->widget);
        break;
      default:
        break;
    }
}

void
add_toolbar(Ecrire_Doc *doc)
{
  Evas_Object  *tbar;

  _toolbar = tbar = elm_toolbar_add(_win);
  elm_toolbar_homogeneous_set(tbar, 0);
  elm_toolbar_shrink_mode_set(tbar, ELM_TOOLBAR_SHRINK_SCROLL);
  elm_toolbar_select_mode_set(tbar, ELM_OBJECT_SELECT_MODE_NONE);
  elm_toolbar_align_set(tbar, 0);
  evas_object_size_hint_align_set(tbar, EVAS_HINT_FILL, 0);
  elm_box_pack_start(_box_main, tbar);
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
   Evas_Object  *obj;
   Evas_Object  *table;
   Evas_Coord h = 600;
   Evas_Coord w = 600;

   doc = calloc(1, sizeof(*doc));

   _win = elm_win_add(NULL, "editor", ELM_WIN_BASIC);
   elm_win_alpha_set (_win, EINA_TRUE);
   elm_win_autodel_set(_win, EINA_FALSE);

   _bg = elm_bg_add (_win);
   elm_win_resize_object_add (_win, _bg);
   evas_object_size_hint_weight_set (_bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show (_bg);
   evas_object_data_set(_win, "background", _bg);

   _box_main = obj = elm_box_add (_win);
   evas_object_size_hint_weight_set (obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add (_win, obj);
   evas_object_show (obj);

   if(!_ent_cfg->menu)
     {
       Evas_Object  *edit_menu;
       Evas_Object  *file_menu;

       _menu = elm_win_main_menu_get(_win);
       evas_object_show(_menu);

       file_menu = elm_menu_item_add(_menu, NULL, NULL, _("File"), NULL, NULL);
       elm_menu_item_add(_menu, file_menu, "document-new", _("New"), _new, doc);
       elm_menu_item_add(_menu, file_menu, "document-open", _("Open"), _open_cb, doc);
       doc->mm_save =
         elm_menu_item_add(_menu, file_menu, "document-save", _("Save"), _save, doc);
       doc->mm_save_as =
         elm_menu_item_add(_menu, file_menu, "document-save-as", _("Save As"), _save_as, doc);
       elm_menu_item_separator_add(_menu, file_menu);
       elm_menu_item_add(_menu, file_menu, "preferences-system", _("Settings"), _settings, doc);
       elm_menu_item_separator_add(_menu, file_menu);
       elm_menu_item_add(_menu, file_menu, "application-exit", _("Exit"), _close_cb, doc);

       edit_menu = elm_menu_item_add(_menu, NULL, NULL, _("Edit"), NULL, NULL);
       doc->mm_undo =
         elm_menu_item_add(_menu, edit_menu, "edit-undo", _("Undo"), _undo, doc);
       doc->mm_redo =
         elm_menu_item_add(_menu, edit_menu, "edit-redo", _("Redo"), _redo, doc);
       elm_menu_item_separator_add(_menu, edit_menu);
       doc->mm_cut =
         elm_menu_item_add(_menu, edit_menu, "edit-cut", _("Cut"), _cut, doc);
       doc->mm_copy =
         elm_menu_item_add(_menu, edit_menu, "edit-copy", _("Copy"), _copy, doc);
       doc->mm_paste =
         elm_menu_item_add(_menu, edit_menu, "edit-paste", _("Paste"), _paste, doc);
       doc->mm_select_all =
         elm_menu_item_add(_menu, edit_menu, "edit-select-all", _("Select All"), _select_all_cb, doc);
       elm_menu_item_separator_add(_menu, edit_menu);
       elm_menu_item_add(_menu, edit_menu, "edit-find-replace", _("Search"), _find, doc);
       elm_menu_item_add(_menu, edit_menu, "go-jump", _("Jump to"), _goto_line_focus_cb, doc);

       elm_object_item_disabled_set(doc->mm_paste, EINA_TRUE);
     }

   if(!_ent_cfg->toolbar)
     {
       add_toolbar(doc);
       /* We don't have a selection when we start, make the items disabled */
       elm_object_item_disabled_set(doc->paste_item, EINA_TRUE);
     }
   _set_cut_copy_disabled(doc, EINA_TRUE);
   _set_save_disabled(doc, EINA_TRUE);
   _set_undo_redo_disabled(doc, EINA_TRUE);

   _box_editor = obj = elm_box_add (_win);
   evas_object_size_hint_align_set (obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(obj);

   doc->code = elm_code_create();
   doc->widget = elm_code_widget_add(_win, doc->code);

   _init_font(doc);
   elm_code_file_line_append(doc->code->file, "", 0, NULL);
#ifdef EFL_VERSION_1_22
   elm_code_widget_editable_set(doc->widget, EINA_TRUE);
   elm_code_widget_syntax_enabled_set(doc->widget, EINA_TRUE);
   elm_code_widget_show_whitespace_set(doc->widget, EINA_TRUE);
   elm_code_widget_line_numbers_set(doc->widget, !_ent_cfg->line_numbers);
   if(_ent_cfg->line_width_marker>0)
     elm_code_widget_line_width_marker_set(doc->widget, _ent_cfg->line_width_marker);
   elm_code_widget_tab_inserts_spaces_set(doc->widget, !_ent_cfg->insert_spaces);
#else
   elm_obj_code_widget_editable_set(doc->widget, EINA_TRUE);
   elm_obj_code_widget_syntax_enabled_set(doc->widget, EINA_TRUE);
   elm_obj_code_widget_show_whitespace_set(doc->widget, EINA_TRUE);
   elm_obj_code_widget_line_numbers_set(doc->widget, !_ent_cfg->line_numbers);
   if(_ent_cfg->line_width_marker>0)
     elm_code_widget_line_width_marker_set(doc->widget, _ent_cfg->line_width_marker);
   elm_obj_code_widget_tab_inserts_spaces_set(doc->widget, !_ent_cfg->insert_spaces);
#endif
   evas_object_size_hint_align_set(doc->widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(doc->widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(_box_editor, doc->widget);
   elm_box_pack_end(_box_main, _box_editor);
   evas_object_show(doc->widget);

   box = elm_box_add (_win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set (box, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_padding_set(box, 5, 5, 0, 0);
   elm_box_pack_end(_box_main,box);
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
   evas_object_smart_callback_add(doc->widget, "selection,paste", _changed, doc);

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
   ecore_event_handler_add(ECORE_EVENT_KEY_UP,
                           (Ecore_Event_Handler_Cb)_key_up_cb,
                           doc);
   evas_object_event_callback_add(_box_editor,
                                  EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down_cb,
                                  doc);
   evas_object_smart_callback_add(_win,
                                  "delete,request",
                                  _close_cb,
                                  doc);
   evas_object_smart_callback_add(_win,
                                  "focused",
                                  (Evas_Smart_Cb)_get_clipboard_cb,
                                  doc);
   evas_object_event_callback_add(_win,
                                  EVAS_CALLBACK_RESIZE,
                                  (Evas_Object_Event_Cb)_win_move_cb,
                                  _win);

   if(_ent_cfg->alpha)
     ecrire_alpha_set(_ent_cfg->alpha);

   if(_ent_cfg->height && _ent_cfg->width)
     evas_object_resize(_win, _ent_cfg->width, _ent_cfg->height);
   else
     evas_object_resize(_win, w, h);
   evas_object_show(_win);

   if (optind < argc)
     {
       delay_anim = EINA_TRUE;
       _open_file(doc, argv[optind]);
       DBG("Opening filename: '%s'", argv[optind]);
     }
   else
     _update_cur_file(doc);

   elm_object_focus_set(doc->widget, EINA_TRUE);
}

void
ecrire_alpha_set(int alpha)
{
  int r;
  int g;
  int b;
  int a;

  efl_gfx_color_get(efl_part(_win, "background"), &r, &g, &b, &a);
  efl_gfx_color_set(_bg, r*1.5, g*1.5, b*1.5, alpha);
  ALPHA (_box_main, alpha);
  ALPHA (_box_editor, alpha);
}

Eina_Bool ecrire_inwin_move_cb(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               void *ev EINA_UNUSED)
{
  Evas_Coord h;
  Evas_Coord w;
  Evas_Object *inwin;

  inwin = data;
  evas_object_geometry_get(_win, NULL, NULL, &w, &h);
  evas_object_resize(inwin, w, h);

  return EINA_TRUE;
}

void
ecrire_pack_end(Evas_Object *table)
{
  elm_box_pack_end(_box_editor, table);
}

void
ecrire_toolbar_del(void)
{
  evas_object_del(_toolbar);
}

Evas_Object *
ecrire_win_get(void)
{
  return(_win);
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
   
   if (args < 0)
     {
        fprintf(stderr, "ERROR: Could not parse options.");
        return -1;
     }

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
#ifdef ENABLE_NLS
   bindtextdomain(PACKAGE, LOCALE_DIR);
   textdomain(PACKAGE);
#endif

   elm_init(argc, argv);

   ecrire_cfg_init(PACKAGE_NAME);
   ecrire_cfg_load();

   create_window(argc, argv);

   signal(SIGQUIT, _signal_cb);
   signal(SIGTERM, _signal_cb);
   signal(SIGINT, _signal_cb);
   signal(SIGHUP, _signal_cb);
   signal(SIGPIPE, _signal_cb);
   signal(SIGALRM, _signal_cb);

   elm_run();

   ecrire_cfg_shutdown();
   elm_shutdown();
   eina_log_domain_unregister(_ecrire_log_dom);
   _ecrire_log_dom = -1;
   eina_shutdown();

   return 0;
}
ELM_MAIN()
