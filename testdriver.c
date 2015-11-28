#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sdb.h"
#include "ext.h"

int main() {
	DB * db = create_db("testdb");

	if (db == NULL) {
		perror("db_create() fail");
		return 1;
	}

	Table * table = create_table(db, "users");	

	if (table == NULL) {
		perror("table_create() fail");
		return 1;
	}

	printf("%s\n", db->table_head->name);

	const char * entry1str = "User1";

	int entry_id = write_entry(db, table->id, -1, entry1str);

	if (entry_id == -1) {
		perror("write_entry() fail");
	}

	char * read_res = read_entry(db, table->id, entry_id);

	if (strcmp(read_res, entry1str) != 0) {
		perror("read_res() fail");
	}

	write_entry(db, table->id, -1, "bobby");

	//remove_entry(table, entry_id);	
	//remove_table(db, table->id);

	char * data = malloc(db_size(db));
	int size = serialize_db(data, db);
	/*
	printf("Data: %s\n", data);
	printf("Strlen: %d\n", (int)(strlen(data)));
	printf("DBSize: %d\n", db_size(db));
	printf("Res: %d\n", size);
	int test;
	char * last = data;
	last += sizeof(int);
	memcpy(&test,last, sizeof(int));
	printf("Last: %d\n", test);
	last += sizeof(int);
	char *name = malloc(strlen(last)+1);
	strcpy(name, last);
	printf("Name: %s\n", name);
	last += strlen(last)+1;
	last += sizeof(int);
	memcpy(&test, last, sizeof(int));
	printf("table_numentries: %d\n", test);
*/
	DB * copy = NULL;
	unserialize_db(data, &copy);
	printf("from orig: %s", all_data_from_table(db->table_head));
	printf("from serialized copy: %s", all_data_from_table(copy->table_head));
	free(data);
	destroy_db(copy);
	destroy_db(db);
	return 0;
}
