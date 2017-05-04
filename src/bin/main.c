#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_ECORE_X
# include <Ecore_X.h>
#endif

#include <Elementary.h>

#include "mess_header.h"
#include "cfg.h"
#include "ui/ui.h"

static Eina_Unicode plain_utf8 = EINA_TRUE;
static Ecrire_Entry *main_ec_ent;

static void print_usage(const char *bin);

/* specific log domain to help debug only ecrire */
int _ecrire_log_dom = -1;

static void
_set_save_disabled(Ecrire_Entry *ent, Eina_Bool disabled)
{
  elm_object_item_disabled_set(ent->save_item, disabled);
  elm_object_item_disabled_set(ent->save_as_item, disabled);
}

void
editor_font_set(Ecrire_Entry *ent, const char *name, unsigned int size)
{
  if(size==0)
    size = 10;
  if(name)
    elm_obj_code_widget_font_set(ent->entry, name, size);
  else
    elm_obj_code_widget_font_set(ent->entry, NULL, size);
}

static void
_init_font(Ecrire_Entry *ent)
{
  editor_font_set(ent, _ent_cfg->font.name, _ent_cfg->font.size);
}

static void
_init_entry(Ecrire_Entry *ent)
{
   _init_font(ent);
   elm_object_item_disabled_set(ent->undo_item, EINA_TRUE);
   elm_object_item_disabled_set(ent->redo_item, EINA_TRUE);
}

static void
_alert_if_need_saving(void (*done)(void *data), Ecrire_Entry *ent)
{
   if (!elm_object_item_disabled_get(ent->save_item))
     {
        ui_alert_need_saving(ent->entry, done, ent);
     }
   else
     {
        done(ent);
     }
}

static void
_sel_start(void *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Ecrire_Entry *ent = data;
   elm_object_item_disabled_set(ent->copy_item, EINA_FALSE);
   elm_object_item_disabled_set(ent->cut_item, EINA_FALSE);
}

static void
_sel_clear(void *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Ecrire_Entry *ent = data;
   elm_object_item_disabled_set(ent->copy_item, EINA_TRUE);
   elm_object_item_disabled_set(ent->cut_item, EINA_TRUE);
}

static void
_update_cur_file(const char *file, Ecrire_Entry *ent)
{
   const char *saving = (!elm_object_item_disabled_get(ent->save_item)) ?
      "*" : "";
   eina_stringshare_replace(&ent->filename, file);
     {
        char buf[1024];
        if (ent->filename)
           snprintf(buf, sizeof(buf), _("%s%s - %s"), saving, ent->filename,
                 PACKAGE_NAME);
        else
           snprintf(buf, sizeof(buf), _("%sUntitled %d - %s"), saving,
                 ent->unsaved, PACKAGE_NAME);

        elm_win_title_set(ent->win, buf);
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

   Ecrire_Entry *ent = data;
   elm_obj_code_widget_cursor_position_get(ent->entry,&line,&col);
   snprintf(buf, sizeof(buf), _(" Line %d, Column %d"), line, col);
   elm_object_text_set(ent->cursor_label, buf);
   if(elm_object_item_disabled_get(ent->undo_item) &&
      elm_obj_code_widget_can_undo_get(ent->entry)) 
     {
       elm_object_item_disabled_set(ent->undo_item, EINA_FALSE);
     }
}

static void
_undo(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   /* In undo we care about the current item */
   Ecrire_Entry *ent = data;
   elm_obj_code_widget_undo(ent->entry);
   if(!elm_object_item_disabled_get(ent->undo_item) &&
      !elm_obj_code_widget_can_undo_get(ent->entry)) 
     {
       elm_object_item_disabled_set(ent->undo_item, EINA_TRUE);
     }
   else if(elm_object_item_disabled_get(ent->redo_item) &&
           elm_obj_code_widget_can_redo_get(ent->entry)) 
     {
       elm_object_item_disabled_set(ent->redo_item, EINA_FALSE);
     }
}

static void
_redo(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Entry *ent = data;
   elm_obj_code_widget_redo(ent->entry);
   if(!elm_object_item_disabled_get(ent->redo_item) &&
      !elm_obj_code_widget_can_redo_get(ent->entry)) 
     {
       elm_object_item_disabled_set(ent->redo_item, EINA_TRUE);
     }
}

static void
_ent_changed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecrire_Entry *ent = data;
   _set_save_disabled(ent, EINA_FALSE);
   _update_cur_file(ent->filename, ent);
}

static void
_clear ()
{
  elm_code_file_clear(main_ec_ent->code->file);
  elm_code_file_line_append(main_ec_ent->code->file, "", 0, NULL);
}

static void
_load_to_entry(Ecrire_Entry *ent, const char *file)
{
   if (file)
     {
        elm_code_file_open(ent->code,file);
        _init_entry(ent);
        _set_save_disabled(ent, EINA_TRUE);
     }
   else
     {
        _init_entry(ent);
     }

   _update_cur_file(file, ent);
}

static void
_fs_open_done(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
      void *event_info)
{
   const char *selected = event_info;
   if (selected)
      _load_to_entry(main_ec_ent, selected);
}

void
save_do(const char *file, Ecrire_Entry *ent)
{
   elm_code_file_save (ent->code->file);
   _set_save_disabled(ent, EINA_TRUE);
   _update_cur_file(file, ent);
}

static void
_fs_save_done(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
      void *event_info)
{
   const char *selected = event_info;

   if (selected)
     {
        save_do(selected, main_ec_ent);
     }
}

static void
_open_do(void *data)
{
   Ecrire_Entry *ent = data;
   ui_file_open_save_dialog_open(ent->win, _fs_open_done, EINA_FALSE);
}

static void
_goto_line(void *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Ecrire_Entry *ent = data;
   ui_goto_dialog_open(ent->win, ent);
}

static void
_open_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Entry *ent = data;
   _alert_if_need_saving(_open_do, ent);
}

void
editor_save(Ecrire_Entry *ent, void *callback_func)
{
   if (ent->filename)
     {
        save_do(ent->filename, ent);
     }
   else
     {
        ui_file_open_save_dialog_open(ent->win, callback_func, EINA_TRUE);
     }
}

static void
_save(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Entry *ent = data;
   editor_save(ent, _fs_save_done);
}

static void
_save_as(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Entry *ent = data;
   ui_file_open_save_dialog_open(ent->win, _fs_save_done, EINA_TRUE);
}

static void
_new_do(void *data)
{
   Ecrire_Entry *ent = data;
   _clear();
   _init_entry(ent);
   _set_save_disabled(ent, EINA_TRUE);
   _update_cur_file(NULL, ent);
}

static void
_new(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Entry *ent = data;
   _alert_if_need_saving(_new_do, ent);
}

static void
_cut(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Entry *ent = data;
   elm_code_widget_selection_cut(ent->entry);
}

static void
_copy(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Entry *ent = data;
   elm_code_widget_selection_copy(ent->entry);
}

static void
_paste(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Entry *ent = data;
   elm_code_widget_selection_paste(ent->entry);
}

static void
_find(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecrire_Entry *ent = data;
   ui_find_dialog_open(ent->win, ent);
}

static void
_settings(void *data,
          Evas_Object *obj EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   Ecrire_Entry *ent = data;
   ui_settings_dialog_open(elm_object_top_widget_get(ent->win), ent, _ent_cfg);
}


static void
_win_del_do(void *data EINA_UNUSED)
{
   elm_exit();
}

static void
my_win_del(void *data, Evas_Object *obj, void *event_info)
{
   Ecrire_Entry *ent = data;
   (void) data;
   (void) obj;
   (void) event_info;
   _alert_if_need_saving(_win_del_do, ent);
}

#ifdef HAVE_ECORE_X

Eina_Bool ctrl_pressed = EINA_FALSE;

static Eina_Bool
_selection_notify(void *data, int type EINA_UNUSED, void *_event)
{
   Ecrire_Entry *ent = data;
   Ecore_X_Event_Fixes_Selection_Notify *event =
      (Ecore_X_Event_Fixes_Selection_Notify *) _event;

   if (!event)
      return ECORE_CALLBACK_PASS_ON;

   if (event->selection == ECORE_X_SELECTION_CLIPBOARD)
     {
        elm_object_item_disabled_set(ent->paste_item,
              (event->reason != ECORE_X_OWNER_CHANGE_REASON_NEW_OWNER));
     }

   return ECORE_CALLBACK_PASS_ON;
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
           !strcmp("f", event->key))
          {
            _find(data,NULL,NULL);
          }
        else if(!strcmp("O", event->key) ||
                !strcmp("o", event->key))
          {
            _open_cb(data,NULL,NULL);
          }
        else if(!strcmp("S", event->key) ||
                !strcmp("s", event->key))
          {
            _save(data,NULL,NULL);
          }
      }
    else if (!strcmp("Control_L", event->key) ||
             !strcmp("Control_R", event->key))
      {
        ctrl_pressed = EINA_TRUE;
      }
    return ECORE_CALLBACK_PASS_ON;
}
#endif

int
main(int argc, char *argv[])
{
   Evas_Object  *box, *obj, *tbar;
   Evas_Coord w = 600, h = 600;
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

   main_ec_ent = calloc(1, sizeof(*main_ec_ent));
   main_ec_ent->unsaved = 1;
   main_ec_ent->filename = NULL;

   if (optind < argc)
     {
        main_ec_ent->filename = eina_stringshare_add(argv[optind]);
     }

   DBG("Opening filename: '%s'", main_ec_ent->filename);

   main_ec_ent->win = elm_win_add(NULL, "editor", ELM_WIN_BASIC);
   elm_win_alpha_set (main_ec_ent->win, EINA_TRUE);
   elm_win_autodel_set(main_ec_ent->win, EINA_FALSE);

   main_ec_ent->bg = elm_bg_add (main_ec_ent->win);
   if(_ent_cfg->alpha)
     ALPHA (main_ec_ent->bg, _ent_cfg->alpha);
   elm_win_resize_object_add (main_ec_ent->win, main_ec_ent->bg);
   evas_object_size_hint_weight_set (main_ec_ent->bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show (main_ec_ent->bg);

   box = obj = elm_box_add (main_ec_ent->win);
   evas_object_size_hint_weight_set (obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add (main_ec_ent->win, box);
   evas_object_show (obj);

   tbar = elm_toolbar_add(main_ec_ent->win);
   elm_toolbar_homogeneous_set(tbar, 0);
   elm_toolbar_shrink_mode_set(tbar, ELM_TOOLBAR_SHRINK_SCROLL);
   elm_toolbar_select_mode_set(tbar, ELM_OBJECT_SELECT_MODE_NONE);
   elm_toolbar_align_set(tbar, 0.0);
   evas_object_size_hint_weight_set(tbar, 0.0, 0.0);
   evas_object_size_hint_align_set(tbar, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(box, tbar);
   evas_object_show(tbar);

   main_ec_ent->bx = obj = elm_box_add (main_ec_ent->win);
   if(_ent_cfg->alpha)
     ALPHA (obj, _ent_cfg->alpha);
   evas_object_size_hint_align_set (obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(obj);

   main_ec_ent->code = elm_code_create();
   main_ec_ent->entry = efl_add(elm_code_widget_class_get(),
                                main_ec_ent->win,
                                elm_obj_code_widget_code_set(efl_added,
                                                             main_ec_ent->code));
   _init_font(main_ec_ent);
   elm_code_file_line_append(main_ec_ent->code->file, "", 0, NULL);
   elm_obj_code_widget_editable_set(main_ec_ent->entry, EINA_TRUE);
   elm_obj_code_widget_syntax_enabled_set(main_ec_ent->entry, EINA_TRUE);
   elm_obj_code_widget_show_whitespace_set(main_ec_ent->entry, EINA_TRUE);
   elm_obj_code_widget_line_numbers_set(main_ec_ent->entry, _ent_cfg->line_numbers);
   evas_object_size_hint_align_set(main_ec_ent->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(main_ec_ent->entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(main_ec_ent->bx, main_ec_ent->entry);
   elm_box_pack_end(box, main_ec_ent->bx);
   evas_object_show(main_ec_ent->entry);

   main_ec_ent->cursor_label = obj = elm_label_add(main_ec_ent->win);
   _cur_changed(main_ec_ent, NULL, NULL);
   evas_object_size_hint_align_set(obj, 0, 0.5);
   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(box, obj);
   evas_object_show(obj);

   evas_object_smart_callback_add(main_ec_ent->entry, "cursor,changed",
         _cur_changed, main_ec_ent);
   evas_object_smart_callback_add(main_ec_ent->entry, "changed,user", _ent_changed, main_ec_ent);
   evas_object_smart_callback_add(main_ec_ent->entry, "undo,request", _undo, main_ec_ent);
   evas_object_smart_callback_add(main_ec_ent->entry, "redo,request", _redo, main_ec_ent);
   evas_object_smart_callback_add(main_ec_ent->entry, "selection,start", _sel_start, main_ec_ent);
   evas_object_smart_callback_add(main_ec_ent->entry, "selection,cleared", _sel_clear, main_ec_ent);

   elm_toolbar_item_append(tbar, "document-new", _("New"), _new, main_ec_ent);
   elm_toolbar_item_append(tbar, "document-open", _("Open"), _open_cb, main_ec_ent);
   main_ec_ent->save_item =
     elm_toolbar_item_append(tbar, "document-save", _("Save"), _save, main_ec_ent);
   main_ec_ent->save_as_item =
     elm_toolbar_item_append(tbar, "document-save-as", _("Save As"), _save_as, main_ec_ent);
   elm_toolbar_item_separator_set(
         elm_toolbar_item_append(tbar, "", "", NULL, NULL), EINA_TRUE);
   main_ec_ent->undo_item =
      elm_toolbar_item_append(tbar, "edit-undo", _("Undo"), _undo, main_ec_ent);
   main_ec_ent->redo_item =
      elm_toolbar_item_append(tbar, "edit-redo", _("Redo"), _redo, main_ec_ent);
   elm_toolbar_item_separator_set(
         elm_toolbar_item_append(tbar, "", "", NULL, NULL), EINA_TRUE);
   main_ec_ent->cut_item = elm_toolbar_item_append(tbar, "edit-cut", _("Cut"), _cut, main_ec_ent);
   main_ec_ent->copy_item =
      elm_toolbar_item_append(tbar, "edit-copy", _("Copy"), _copy, main_ec_ent);
   main_ec_ent->paste_item =
      elm_toolbar_item_append(tbar, "edit-paste", _("Paste"), _paste, main_ec_ent);
   elm_toolbar_item_separator_set(
         elm_toolbar_item_append(tbar, "", "", NULL, NULL), EINA_TRUE);
   elm_toolbar_item_append(tbar, "edit-find-replace", _("Search"),
         _find, main_ec_ent);
   elm_toolbar_item_append(tbar, "go-jump", _("Jump to"), _goto_line, main_ec_ent);
   elm_toolbar_item_separator_set(
         elm_toolbar_item_append(tbar, "", "", NULL, NULL), EINA_TRUE);
   elm_toolbar_item_append(tbar, "preferences-system", _("Settings"),
         _settings, main_ec_ent);

#ifdef HAVE_ECORE_X
   if (!ecore_x_selection_owner_get(ECORE_X_ATOM_SELECTION_CLIPBOARD))
     {
        elm_object_item_disabled_set(main_ec_ent->paste_item, EINA_TRUE);
     }

   ecore_x_fixes_selection_notification_request(ECORE_X_ATOM_SELECTION_CLIPBOARD);
   ecore_event_handler_add(ECORE_X_EVENT_FIXES_SELECTION_NOTIFY,
         _selection_notify, main_ec_ent);
   
   ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, _key_down_cb, main_ec_ent);
#endif

   /* We don't have a selection when we start, make the items disabled */
   elm_object_item_disabled_set(main_ec_ent->copy_item, EINA_TRUE);
   elm_object_item_disabled_set(main_ec_ent->cut_item, EINA_TRUE);
   _set_save_disabled(main_ec_ent, EINA_TRUE);

   evas_object_resize(main_ec_ent->win, w, h);

   evas_object_smart_callback_add(main_ec_ent->win, "delete,request", my_win_del, main_ec_ent);
   evas_object_show(main_ec_ent->win);

   _load_to_entry(main_ec_ent, main_ec_ent->filename);

   elm_object_focus_set(main_ec_ent->entry, EINA_TRUE);

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
