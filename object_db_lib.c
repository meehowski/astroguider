/*
* CFile1.c
*
* Created: 30/09/2012 7:22:23 AM
*  Author: michal
*/

#include "project.h"
#include "db_stars.h"
#include "db_abell_gc.h"
#include "db_abell_pn.h"
#include "db_barnard_dn.h"
#include "db_messier.h"
#include "db_ic.h"
#include "db_ngc.h"

db_list_t db_list[db_list_end] =
{
  { (const void *) db_stars, DB_STARS_SIZE, db_style_named, "STARS"},
  { (const void *) db_messier, DB_MESSIER_SIZE, db_style_indexed, "MESSIER"},
  { (const void *) db_abell_gc, DB_ABELL_GC_SIZE, db_style_named, "ABELL GC"},
  { (const void *) db_abell_pn, DB_ABELL_PN_SIZE, db_style_named, "ABELL PN"},
  { (const void *) db_barnard_dn, DB_BARNARD_DN_SIZE, db_style_named, "BARNARD DN"},
  { (const void *) db_ic, DB_IC_SIZE, db_style_indexed, "IC"},
  { (const void *) db_ngc, DB_NGC_SIZE, db_style_indexed, "NGC"}
};

void db_init()
{
  DEBUG_PRINT("OBJ DB lib init\n");
}

int8_t db_get_num()
{
  return db_list_end; // stores total number of databases
}

db_list_t *db_get(uint8_t db)
{
  // database index out of bounds
  if (db >= db_list_end)
  {
    return NULL;
  }

  return &db_list[db];
}

char *db_get_name(uint8_t db)
{
  // database index out of bounds
  if (db >= db_list_end)
  {
    return NULL;
  }

  return db_list[db].name;
}

int16_t db_get_size(uint8_t db)
{
  // database index out of bounds
  if (db >= db_list_end)
  {
    return -1;
  }

  return db_list[db].size;
}

db_entry_t *db_get_object(uint8_t db, uint16_t index)
{
  static db_entry_t retval;
  db_list_t *dbp;
  db_named_object_t *dbno;
  db_indexed_object_t *dbio;
  union
  {
    uint16_t val;
    uint8_t i8[2];
  } db2val;
  // init
  retval.valid = 0;

  // database index out of bounds
  if (db >= db_list_end)
  {
    return &retval;
  }

  dbp = &db_list[db];

  switch (dbp->style)
  {
    case db_style_named:
      dbno = & ((db_named_object_t *) dbp->db) [index];
      db2val.i8[1] = dbno->ra1;
      db2val.i8[0] = dbno->ra0;
      retval.ra = db2val.val;
      db2val.i8[1] = dbno->dec1;
      db2val.i8[0] = dbno->dec0;
      retval.dec = db2val.val;
      memcpy(&retval.name, (void *) &dbno->name, sizeof(dbno->name));
      retval.name[sizeof(retval.name) - 1] = 0;
      retval.valid = 1;
      break;

    case db_style_indexed:
      dbio = & ((db_indexed_object_t *) dbp->db) [index];
      db2val.i8[1] = dbio->ra1;
      db2val.i8[0] = dbio->ra0;
      retval.ra = db2val.val;
      db2val.i8[1] = dbio->dec1;
      db2val.i8[0] = dbio->dec0;
      retval.dec = db2val.val;
      snprintf(retval.name, sizeof(retval.name) - 1, "%c%i", tolower((int) dbp->name[0]), index + 1);
      retval.name[sizeof(retval.name) - 1] = 0;
      retval.valid = 1;
      break;

    default:
      // internal error
      break;
  }

  // db encoding: (x * 24 * 60 * 60 ) / 64800
  retval.ra = (retval.ra * 4) / 3;

  // db encoding: (x * 180 * 60 * 60) / 64800
  if (retval.dec < 64800)   // +ive
  {
    retval.dec = retval.dec * 10;
  }
  else     // -ive
  {
    retval.dec = retval.dec - 65536;
// db encoding: (x * 180 * 60 * 60) / 65536
    retval.dec = retval.dec * 10125;
    retval.dec = retval.dec / 1024;
  }

  //RUNTIME_DEBUG_VA("%lu", retval.dec);
  return (&retval);
}
