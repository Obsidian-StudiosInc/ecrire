#ifndef _MESS_HEADER_H
#define _MESS_HEADER_H

#include <Elementary.h>

extern int _ecrire_log_dom;
#define ECRIRE_DEFAULT_LOG_COLOR EINA_COLOR_CYAN

#define CRI(...)      EINA_LOG_DOM_CRIT(_ecrire_log_dom, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_ecrire_log_dom, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_ecrire_log_dom, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_ecrire_log_dom, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_ecrire_log_dom, __VA_ARGS__)

#define ALPHA(O,A) evas_object_color_set (O, 255, 255, 255, A)

struct _Ecrire_Entry {
     Elm_Code *code;
     Elm_Code_Widget *entry;
     Evas_Object *win;
     Evas_Object *bg;
     Evas_Object *bx;
     Evas_Object *cursor_label;
     const char *filename;
     int unsaved;
     Elm_Object_Item *copy_item, *cut_item, *save_item, *save_as_item,
                     *paste_item, *undo_item, *redo_item;
};

typedef struct _Ecrire_Entry Ecrire_Entry;

char *_load_plain(const char *file);
Eina_Bool _load_file(const char *file, const Elm_Code_File *code_file);
Eina_Bool _save_markup_utf8(const char *file, const char *text);
Eina_Bool _save_plain_utf8(const char *file, const char *text);

void editor_font_set(Ecrire_Entry *ent, const char *name, unsigned int size);
void editor_save(Ecrire_Entry *ent, void *callback_func);
void save_do(const char *file, Ecrire_Entry *ent);

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(x) gettext(x)
#else
# define _(x) (x)
#endif

#endif
