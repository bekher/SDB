#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ext.h"

// get the size of ONE entry
unsigned int entry_size(Entry * entry) {
	unsigned int size = 0;
	if (entry == NULL) 
		return 0;

	size += sizeof(entry->id, &(entry->id), sizeof(entry->id));
	size += strlen(entry->data) + 1;
	return size;
}

// get the size of ONE table
unsigned int table_size(Table * table) {
	unsigned int size = 0;
	if (table == NULL) 
		return 0;

	Entry * entry = table->entry_head;

	while (entry) {
		size += entry_size(entry);
		entry = entry->next;
	}

	size += sizeof(table->id);
	size += sizeof(table->num_entries);
	size += sizeof(table->last_id);
	size += strlen((table->name)) + 1;

	return size;
}

unsigned int db_size(DB * db) {
	unsigned int size = 0;
	if (db == NULL) 
		return 0;

	Table * table = db->table_head;

	while (table) {
		size += table_size(table);
		table = table->next;
	}

	size += sizeof(db->num_tables);
	size += sizeof(db->last_id);
	size += strlen((db->name)) + 1;

	return size;
}

// serialize ONE table, assumes preallocated buffer
int serialize_table(char ** destbuf, Table * table) {

	if (destbuf == NULL) 
		return -1;

	int tsize = table_size(table) + 1, pos = 0;

	// serialize static table info
	memcpy(destbuf[pos], &(table->id), sizeof(table->id));
	pos += sizeof(table->id);
	memcpy(destbuf[pos], &(table->num_entries), sizeof(table->num_entries));
	pos += sizeof(table->num_entries);
	memcpy(destbuf[pos], &(table->last_id), sizeof(table->last_id));
	pos += sizeof(table->last_id);
	memcpy(destbuf[pos], &(table->name), strlen(table->name) + 1);
	pos += strlen(table->name) + 1;
	// note we do not serialize the * to the next entry, since mem addrs will
	// on new execution; we maintain order by serializing in given order

	// serialize entry chain
	Entry * entry = table->entry_head;

	while (entry) {
		memcpy(destbuf[pos], &(entry->id), sizeof(entry->id));
		pos += sizeof(entry->id);
		memcpy(destbuf[pos], &(entry->data), strlen(entry->data) + 1);
		pos += strlen(entry->data) + 1;	
		
		entry = entry->next;
	}

	return tsize;
}

int serialize_db(char** destbuf, DB * db) {
	if (destbuf == NULL) 
		return -1;

	int dbsize = db_size(db), pos = 0;

	memcpy(destbuf[pos], &db->num_tables, sizeof(db->num_tables));
	pos += sizeof(db->num_tables);
	memcpy(destbuf[pos], &db->last_id, sizeof(db->last_id));
	pos += sizeof(db->last_id);
	memcpy(destbuf[pos], &db->name, strlen(db->name) + 1);
	pos += strlen(db->name) + 1;

	Table * table = db->table_head;
	char ** nextbuf;
	int res;

	while (table) {
		nextbuf = &(destbuf[pos]);
		res = serialize_table(nextbuf, table);
		if (res < 0) 
			return -1;
		pos += res;
	}
	return dbsize;
}

