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

	//char * data = malloc(db_size(db));
	//int res = serialize_db(&data, db);
	//printf("Success! %d\n", res);
	destroy_db(db);
	return 0;
}
