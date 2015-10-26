#include <stdio.h>
#include <string.h>
#include "sdb.h"

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

	remove_entry(table, entry_id);	
	remove_table(db, table->id);
	destroy_db(db);

	printf("Success!\n");
	return 0;
}
