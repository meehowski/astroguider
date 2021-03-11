/*
* IncFile1.h
*
* Created: 30/09/2012 9:16:41 AM
*  Author: michal
*/


#ifndef OBJECTDB_H_
#define OBJECTDB_H_

#include "project.h"

//#define __FLASH__ __attribute__((section (".USER_FLASH")))
#define __FLASH__

enum
{
  db_list_stars280 = 0,
  db_list_abell_gc,
  db_list_abell_pn,
  db_list_barnard_dn,
  db_list_messier,
  db_list_ic,
  db_list_ngc,
  db_list_end
};

typedef enum
{
  db_style_undefined = 0,
  db_style_named,
  db_style_indexed
} db_style_e;

typedef struct db_list_s
{
  const void *db;
  uint16_t size;
  db_style_e style;
  char *name;
} db_list_t;

typedef struct db_named_object_s
{
  char name[8];
  uint8_t ra1;
  uint8_t ra0;
  uint8_t dec1;
  uint8_t dec0;
} db_named_object_t;

typedef struct db_indexed_object_s
{
  uint8_t ra1;
  uint8_t ra0;
  uint8_t dec1;
  uint8_t dec0;
} db_indexed_object_t;

typedef struct db_entry_s
{
  char name[9];
  int32_t ra;
  int32_t dec;
  uint8_t valid;
} db_entry_t;

int8_t db_get_num();
char *db_get_name(uint8_t db);
int16_t db_get_size(uint8_t db);
db_list_t *db_get(uint8_t db);
db_entry_t *db_get_object(uint8_t db, uint16_t index);
void db_init();

#endif /* OBJECTDB_H_ */
