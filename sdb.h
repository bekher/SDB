#ifndef SDB_H
#define SDB_H

typedef struct EntryStruct {
	int id;
	char * data;
	struct EntryStruct * next;
} Entry;

typedef struct TableStruct {
	int id;
	int num_entries;
	int last_id;
	char * name;
	Entry * entry_head;
	struct TableStruct * next;
} Table;

typedef struct DBStruct {
	int num_tables;
	int last_id;
	char * name;
	Table * table_head;
} DB;

// db mutators
// take *db, table id, entry id, return data
char * read_entry(DB * db, int tableid, int entryid);
// take *db, table id, data, return entry id or -1 for error
int write_entry(DB * db, int tableid, int entryid, const char * data);

// create funcs
DB * create_db(const char * dbname);
Table * create_table(DB * db, const char * tablename);
Entry * create_entry(Table * table, const char * data);

void remove_table(DB * db, int tableid);
void remove_entry(Table * table, int entryid);

// destroy funcs
void destroy_db(DB * db);
void destroy_table(Table * table);
void destroy_entry(Entry * entry);


#endif

