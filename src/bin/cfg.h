#ifndef _CFG_H
#define _CFG_H

#include <Elementary.h>

#define _ENTITLED_CFG_VERSION 1

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
} Ent_Cfg;

extern Ent_Cfg *_ent_cfg;

#endif
