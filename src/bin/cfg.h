#ifndef _CFG_H
#define _CFG_H

#include <Elementary.h>

#define _ENTITLED_CFG_VERSION 1
#define ECRIRE_RECENT_COUNT 15

void ecrire_cfg_init(const char *file);
void ecrire_cfg_shutdown(void);

Eina_Bool ecrire_cfg_save(void);
Eina_Bool ecrire_cfg_load(void);

typedef struct
{
   unsigned int alpha;
   unsigned int height;
   unsigned int width;
   unsigned int version;
   struct {
        const char *name; /* NULL means theme's font */
        int size; /* 0 means theme's size */
   } font;
   Elm_Wrap_Type wrap_type;
   Eina_Bool line_numbers;
   Eina_Bool menu;
   Eina_Bool toolbar;
   Eina_List *recent;
} Ent_Cfg;

extern Ent_Cfg *_ent_cfg;

#endif
