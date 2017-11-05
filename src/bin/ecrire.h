#ifndef _ECRIRE_H
#define _ECRIRE_H

#include <Elementary.h>

extern int _ecrire_log_dom;
#define ECRIRE_DEFAULT_LOG_COLOR EINA_COLOR_CYAN

#define CRI(...)      EINA_LOG_DOM_CRIT(_ecrire_log_dom, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_ecrire_log_dom, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_ecrire_log_dom, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_ecrire_log_dom, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_ecrire_log_dom, __VA_ARGS__)

#define ALPHA(O,A) evas_object_color_set (O, 255, 255, 255, A)

struct _Ecrire_Doc {
     Elm_Code *code;
     Elm_Code_Widget *widget;
     Evas_Object *win;
     Evas_Object *bg;
     Evas_Object *box_editor;
     Evas_Object *box_main;
     Evas_Object *entry_column;
     Evas_Object *entry_line;
     Evas_Object *cursor_label;
     Evas_Object *label_mime;
     Evas_Object *toolbar;
     /* Main Menu Items */
     Elm_Object_Item *mm_close;
     Elm_Object_Item *mm_copy;
     Elm_Object_Item *mm_cut;
     Elm_Object_Item *mm_paste;
     Elm_Object_Item *mm_redo;
     Elm_Object_Item *mm_save;
     Elm_Object_Item *mm_save_as;
     Elm_Object_Item *mm_select_all;
     Elm_Object_Item *mm_undo;
     /* Toolbar Items */
     Elm_Object_Item *close_item, *copy_item, *cut_item, *save_item,
                     *save_as_item, *select_all_item, *paste_item, *undo_item, *redo_item;
     int unsaved;
     char *path;
};

typedef struct _Ecrire_Doc Ecrire_Doc;

void add_toolbar(Ecrire_Doc *doc);
void editor_font_set(Ecrire_Doc *doc, const char *name, unsigned int size);
void editor_save(Ecrire_Doc *doc, void *callback_func);
void save_do(const char *file, Ecrire_Doc *doc);

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(x) gettext(x)
#else
# define _(x) (x)
#endif

#endif
