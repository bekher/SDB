/* not working, temporarily abandoning for a library-based solution */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ext.h"

// get the size of ONE entry
unsigned int entry_size(Entry * entry) {
	unsigned int size = 0;
	if (entry == NULL) 
		return 0;

	//wtf: size += sizeof(entry->id, &(entry->id), sizeof(entry->id));
	size += sizeof(entry->id);
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
int serialize_table(char * destbuf, Table * table) {

	if (destbuf == NULL) 
		return -1;

	int tsize = table_size(table) + 1;
	char * origbuf = destbuf;

	// serialize static table info
	memcpy(destbuf, &(table->id), sizeof(table->id));
	destbuf += sizeof(table->id);
	memcpy(destbuf, &(table->num_entries), sizeof(table->num_entries));
	destbuf += sizeof(table->num_entries);
	memcpy(destbuf, &(table->last_id), sizeof(table->last_id));
	destbuf += sizeof(table->last_id);
	memcpy(destbuf, table->name, strlen(table->name) + 1);
	destbuf += strlen(table->name) + 1;
	// note we do not serialize the * to the next entry, since mem addrs will
	// on new execution; we maintain order by serializing in given order

	// serialize entry chain
	Entry * entry = table->entry_head;
	while (entry) {
		memcpy(destbuf, &(entry->id), sizeof(entry->id));
		destbuf += sizeof(entry->id);
		memcpy(destbuf, entry->data, strlen(entry->data) + 1);
		destbuf += strlen(entry->data) + 1;	
		
		entry = entry->next;
	}

	printf("offset: %ld\n", destbuf-origbuf);
	printf("actual: %d\n", tsize);

	return (int)(destbuf-origbuf);
}

int serialize_db(char* destbuf, DB * db) {
	if (destbuf == NULL) 
		return -1;

	char * origbuf = destbuf;

	memcpy(destbuf, &db->num_tables, sizeof(db->num_tables));
	//do this:
	//sz = sprintf(destbuf,"num_tables %d\n", &db->num_tables);
	//destbuf += sz;
	destbuf += sizeof(db->num_tables);
	memcpy(destbuf, &db->last_id, sizeof(db->last_id));
	destbuf += sizeof(db->last_id);
	memcpy(destbuf, db->name, strlen(db->name) + 1);
	destbuf += strlen(db->name) + 1;
	Table * table = db->table_head;
	int res;

	while (table) {
		res = serialize_table(destbuf, table);
		if (res < 0) 
			return -1;
		destbuf += res;
		table = table->next;
	}

	// should return size
	return (destbuf-origbuf);
}

int unserialize_db(char * srcbuf, DB ** destdb) {
	if (destdb == NULL)
		return -1;

	char * curpos = srcbuf, *name;
	int num_tables, last_id;
	memcpy(&num_tables, curpos, sizeof(int));
	curpos += sizeof(int);
	memcpy(&last_id, curpos, sizeof(int));
	curpos += sizeof(int);

	name = malloc(strlen(curpos) + 1);
	strcpy(name, curpos);
	curpos += strlen(curpos) + 1;
	
	DB * db = malloc(sizeof(DB));
	if (db == NULL) 
		return -1;
	db->name = name;
	db->num_tables = num_tables;
	db->last_id = last_id;
	printf("dname: %s\n", name);

	Table * table = NULL, * tprev = NULL, * thead = NULL;
	
	int tid, eid, num_entries, tlast_id;
	char * tname, * edata;
	while (num_tables > 0) {
		memcpy(&tid, curpos, sizeof(int));
		curpos += sizeof(int);
		memcpy(&num_entries, curpos, sizeof(int));
		curpos += sizeof(int);
		memcpy(&tlast_id, curpos, sizeof(int));
		curpos += sizeof(int);
		tname = malloc(strlen(curpos) + 1);
		if (tname == NULL)
			return -1;
		strcpy(tname, curpos);
		curpos += strlen(curpos) + 1;

		table = malloc(sizeof(Table));
		if (table == NULL)
			return -1;
		table->id = tid;
		table->num_entries = num_entries;
		table->last_id = tlast_id;
		table->name = tname;
		table->next = NULL;

		Entry * entry = NULL, * eprev = NULL, * ehead = NULL;

		while (num_entries > 0) {
			memcpy(&eid, curpos, sizeof(int));
			curpos += sizeof(int);
			edata = malloc(strlen(curpos) + 1);	
			if (edata == NULL)
				return -1;
			strcpy(edata, curpos);
			curpos += strlen(curpos) + 1;
			
			entry = malloc(sizeof(Entry));
			if (entry == NULL)
				return -1;
			entry->id = eid;
			entry->data = edata;
			entry->next = NULL;

			if (eprev)
				eprev->next = entry;
			if (ehead == NULL)
				ehead = entry;

			eprev = entry;
			--num_entries;
		}
		
		table->entry_head = ehead;

		if (tprev)
			tprev->next = table;
		if (thead == NULL)
			thead = table;

		tprev = table;
		--num_tables;
	}
	db->table_head = thead;

	*destdb = db;

	return curpos-srcbuf;
}
