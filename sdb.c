#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdb.h"

char * read_entry(DB * db, int tableid, int entryid) {
	if (db == NULL) return NULL;
	// find table
	Table * resTable = NULL;
	for (Table * t = db->table_head; t != NULL; t = t->next) {
		if (t->id == tableid) {
			resTable = t;
			break;
		}
	}
	
	if (resTable == NULL) return NULL;
	
	// find entry
	Entry * resEntry = NULL;
	for (Entry * e = resTable->entry_head; e != NULL; e = e->next) {
		if (e->id == entryid)  {
			resEntry = e;
			break;
		}
	}
	if (resEntry == NULL) return NULL;
	return resEntry->data;
		
}

int write_entry(DB * db, int tableid, int entryid, const char * data) {
	if (db == NULL) return -1;
	// find table
	Table * resTable = NULL;

	for (Table * t = db->table_head; t != NULL; t = t->next) {
		if (t->id == tableid) {
			resTable = t;
			break;
		}
	}
	
	if (resTable == NULL) return -1;
	
	// we must create a new entry
	if (entryid == -1) {
		Entry * newEntry = create_entry(resTable, data);
		if (newEntry == NULL) 
			return -1;
		return newEntry->id;
	} else {
		// find entry
		Entry * resEntry = NULL;
		for (Entry * e = resTable->entry_head; e != NULL; e = e->next) {
			if (e->id == entryid)  {
				resEntry = e;
				break;
			}
		}
		if (resEntry == NULL) return -1;

		free(resEntry->data);
		char * newData = malloc(strlen(data) + 1);
		strcpy(newData, data);
		resEntry->data = newData;

		return resEntry->id;
	}		
}

DB * create_db(const char * dbname) {
	DB * db = malloc(sizeof(DB));
	
	if (db == NULL) return NULL;
	char * name = malloc(strlen(dbname)+1);

	if (name == NULL) {
		free(db);
		return NULL;
	}

	strcpy(name, dbname);

	db->name = name;
	db->num_tables = 0;
	db->last_id = 0;
	return db;
}

Table * create_table(DB * db, const char * tablename) {
	if (db == NULL || tablename == NULL) return NULL;

	Table * table = malloc(sizeof(Table));
	
	if (table == NULL) return NULL;

	char * name = malloc(strlen(tablename)+1);

	if (name == NULL) {
		free(table);
		return NULL;
	}

	strcpy(name, tablename);

	table->name = name;
	table->id = (++(db->last_id));
	db->num_tables++;
	table->next = db->table_head;
	db->table_head = table;

	return table;
}

Entry * create_entry(Table * table, const char * data) {
	if (table == NULL) return NULL;

	Entry * entry = malloc(sizeof(Entry));
	
	if (entry == NULL) return NULL;

	// data can be null
	char * _data = NULL;

	if (data != NULL) {
		_data = malloc(strlen(data)+1);

		if (_data == NULL) {
			free(entry);
			return NULL;
		}
		strcpy(_data, data);
	}

	entry->data = _data;
	entry->id = (++(table->last_id));
	table->num_entries++;
	entry->next = table->entry_head;
	table->entry_head = entry;

	return entry;
}

void remove_table(DB * db, int tableid) {
	if (db == NULL) return;
	
	Table * prev = NULL;
	for (Table * t = db->table_head; t != NULL;) {
		if (t->id == tableid) {
			if (prev == NULL) {
				db->table_head = t;
			} else {
				prev->next = t->next;
			}
			destroy_table(t);
			return;
		}
		prev = t;
	}

}

void remove_entry(Table * table, int entryid) {
	if (table == NULL) return;

	Entry * prev = NULL;
	for (Entry * e = table->entry_head; e != NULL;) {
		if (e->id == entryid) {
			if (prev == NULL) {
				table->entry_head = e->next;
			} else {
				prev->next = e->next;
			}
			destroy_entry(e);
			return;
		}
		prev = e;
	}
}

void destroy_db(DB * db) {
	if (db == NULL) return;
	Table * next = NULL;
	for (Table * t = db->table_head; t != NULL;) {
		next = t->next;
		destroy_table(t);
		t = next;
	}

	if (db->name != NULL) free(db->name);
	free(db);

}
	
void destroy_table(Table * table) {
	if (table == NULL) return;
	Entry * next = NULL;
	for (Entry * e = table->entry_head; e != NULL;) {
		next = e->next;
		destroy_entry(e);
		e = next;
	}

	if (table->name != NULL) free(table->name);
	free(table);

}
	
void destroy_entry(Entry * entry) {
	if (entry == NULL) return;
	if (entry->data != NULL) free(entry->data);
	free(entry);
}
