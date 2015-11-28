/* define externalize/internalize (serialization) capabilities for the
   small DB to facilitate future dynamic reconfig */
#ifndef EXT_H
#define EXT_H
#include "sdb.h"

// get size of entry/table/db (excludes struct padding)
unsigned int entry_size(Entry * entry);
unsigned int table_size(Table * table);
unsigned int db_size(DB * db);

// serialize DB from DB struct to buffer
int serialize_db(char * destbuf, DB * srcdb);
// serialize DB from buffer to DB
int unserialize_db(char * srcbuf, DB ** destdb);

// write marshalled db to file
int externalize(char * srcfilename, char ** destbuf);
// read marshalled db from file
int internalize(char * dstfilename, char * srcbuf);
#endif

