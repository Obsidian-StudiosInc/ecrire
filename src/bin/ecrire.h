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

#define ALPHA(O,A) evas_object_color_set (O, A, A, A, A)

struct _Ecrire_Doc {
     Elm_Code *code;
     Elm_Code_Widget *widget;
     Evas_Object *entry_column;
     Evas_Object *entry_line;
     Evas_Object *cursor_label;
     Evas_Object *label_mime;
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
     Elm_Object_Item *close_item;
     Elm_Object_Item *copy_item;
     Elm_Object_Item *cut_item;
     Elm_Object_Item *save_item;
     Elm_Object_Item *save_as_item;
     Elm_Object_Item *select_all_item;
     Elm_Object_Item *paste_item;
     Elm_Object_Item *undo_item;
     Elm_Object_Item *redo_item;
     Eina_Bool changed;
     char *path;
};

typedef struct _Ecrire_Doc Ecrire_Doc;

// Needed due to side effects from efl_part requiring re-sizing for centering
Eina_Bool ecrire_inwin_move_cb(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               void *ev EINA_UNUSED);
Evas_Object * ecrire_win_get(void);
void add_toolbar(Ecrire_Doc *doc);
void ecrire_alpha_set(int alpha);
void ecrire_pack_end(Evas_Object *search);
void ecrire_toolbar_del(void);
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
