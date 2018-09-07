#ifndef _UI_H
#define _UI_H

#include "../ecrire.h"

void ui_file_open_save_dialog_open(Ecrire_Doc *doc,
                                   Evas_Smart_Cb func,
                                   Eina_Bool save);
Evas_Object *ui_find_dialog_open(Evas_Object *parent, Ecrire_Doc *doc);
Evas_Object *ui_settings_dialog_open(Evas_Object *parent,
                                     Ecrire_Doc *doc,
                                     Ent_Cfg *ent_cfg);
void ui_alert_need_saving(Evas_Object *entry,
                          void (*done)(void *data),
                          void *data);

#endif
